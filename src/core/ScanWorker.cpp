#include "core/ScanWorker.h"

#include <QElapsedTimer>
#include <QRunnable>
#include <QSemaphore>

#include "core/DeviceClassifier.h"
#include "core/HostnameResolver.h"
#include "core/Logger.h"
#include "core/NetUtils.h"
#include "core/OsFingerprinter.h"
#include "core/RiskEngine.h"
#include "core/ServiceDetector.h"
#include "core/VendorLookup.h"

namespace pacn {

// One host: liveness probe + (if alive) full enrichment. Runs in the pool.
class HostTask : public QRunnable {
public:
    HostTask(ScanWorker* worker, QHostAddress ip, QSemaphore* permits)
        : worker_(worker), ip_(std::move(ip)), slots_(permits) {
        setAutoDelete(true);
    }

    void run() override {
        if (worker_->cancel_.load()) {
            Device dead;
            dead.ip = ip_;
            slots_->release();
            worker_->reportHostFinished(dead, false);
            return;
        }
        auto hostProbe = worker_->factory_->hostProbe();
        const HostProbeResult r = hostProbe->probe(ip_, worker_->config_);
        Device device;
        device.ip = ip_;
        if (r.alive) device = worker_->enrichHost(ip_, r);
        slots_->release();
        worker_->reportHostFinished(device, r.alive);
    }

private:
    ScanWorker* worker_;
    QHostAddress ip_;
    QSemaphore* slots_;
};

ScanWorker::ScanWorker(ScanConfig config, std::shared_ptr<IProbeFactory> factory,
                       std::shared_ptr<IPlatformServices> platform)
    : config_(std::move(config)),
      factory_(std::move(factory)),
      platform_(std::move(platform)) {
    pool_.setMaxThreadCount(qMax(1, config_.concurrency()));
}

void ScanWorker::requestPause() { paused_.store(true); }

void ScanWorker::requestResume() {
    paused_.store(false);
    QMutexLocker lock(&pauseMutex_);
    pauseCond_.wakeAll();
}

void ScanWorker::requestCancel() {
    cancel_.store(true);
    paused_.store(false);
    QMutexLocker lock(&pauseMutex_);
    pauseCond_.wakeAll();
}

void ScanWorker::waitWhilePaused() {
    QMutexLocker lock(&pauseMutex_);
    while (paused_.load() && !cancel_.load()) {
        pauseCond_.wait(&pauseMutex_, 200);
    }
}

void ScanWorker::run() {
    emit started();
    QElapsedTimer timer;
    timer.start();

    VendorLookup::instance().ensureLoaded();
    ServiceDetector::instance().ensureLoaded();

    // Snapshot platform facts shared (read-only) with host tasks.
    if (platform_) {
        emit phaseChanged(tr("Reading network state"));
        const auto arp = platform_->arpTable();
        for (const ArpEntry& e : arp)
            arpByIp_.insert(e.ip.toString(), netutils::normalizeMac(e.mac));
        const auto gws = platform_->defaultGateways();
        for (const GatewayInfo& g : gws) {
            gatewayIps_.insert(g.address.toString());
            if (gatewayMac_.isEmpty()) gatewayMac_ = netutils::normalizeMac(g.mac);
        }
    }

    // Effective total (IPv6 ranges are capped for enumeration).
    total_ = 0;
    for (const IpRange& r : config_.targets) {
        const quint64 c = r.hostCount();
        if (r.isIpv6())
            total_ += qMin<quint64>(c, IpRange::kIpv6ExpandCap);
        else if (c != IpRange::kHugeCount)
            total_ += c;
    }

    emit phaseChanged(tr("Discovering hosts"));
    Logger::instance().info(QStringLiteral("scan"),
                            QStringLiteral("Starting scan of %1 host(s)").arg(total_));

    QSemaphore permits(qMax(1, config_.concurrency()));

    for (const IpRange& range : config_.targets) {
        if (cancel_.load()) break;
        QHostAddress cur = range.first();
        const QHostAddress last = range.last();
        quint64 produced = 0;
        const quint64 cap = range.isIpv6() ? IpRange::kIpv6ExpandCap : 0;

        while (true) {
            if (cancel_.load()) break;
            waitWhilePaused();
            if (cancel_.load()) break;

            permits.acquire();
            if (cancel_.load()) {
                permits.release();
                break;
            }
            pool_.start(new HostTask(this, cur, &permits));
            ++produced;

            if (cap != 0 && produced >= cap) break;
            if (netutils::compareIp(cur, last) >= 0) break;
            cur = netutils::nextAddress(cur);
        }
    }

    pool_.waitForDone();

    if (!cancel_.load()) {
        emit phaseChanged(tr("Resolving MAC addresses"));
        enrichMacsFromArp();
    }

    {
        QMutexLocker lock(&resultsMutex_);
        summary_.scanned = done_;
        summary_.elapsedMs = timer.elapsed();
        summary_.canceled = cancel_.load();
    }
    Logger::instance().info(
        QStringLiteral("scan"),
        QStringLiteral("Scan finished: %1 up / %2 scanned in %3 ms")
            .arg(summary_.hostsUp)
            .arg(summary_.scanned)
            .arg(summary_.elapsedMs));
    emit finished(summary_);
}

Device ScanWorker::enrichHost(const QHostAddress& ip, const HostProbeResult& probe) const {
    Device d;
    d.ip = ip;
    d.status = DeviceStatus::Online;
    d.rttMs = probe.rttMs;
    d.ttl = probe.ttl;
    d.discoveryMethod = probe.method;
    d.adapter = config_.adapterName;
    d.lastSeen = QDateTime::currentDateTime();
    d.firstSeen = d.lastSeen;

    const QString ipStr = ip.toString();
    d.mac = arpByIp_.value(ipStr);
    if (!d.mac.isEmpty()) d.vendor = VendorLookup::instance().vendor(d.mac);

    if (config_.resolveHostnames) {
        HostnameResolver resolver(platform_);
        d.hostname = resolver.resolve(ip, config_.hostnameMethods,
                                      qMin(1500, config_.hostTimeoutMs() * 2));
    }

    // Port scan (only configured ports; empty list => discovery only).
    ServiceDetector& svc = ServiceDetector::instance();
    auto portProbe = factory_->portProbe();
    for (quint16 port : config_.ports) {
        if (cancel_.load()) break;
        const PortProbeResult pr =
            portProbe->probe(ip, port, config_.portTimeoutMs(), config_.grabBanners);
        if (!pr.open) continue;
        Port p;
        p.number = port;
        p.state = PortState::Open;
        p.banner = pr.banner;
        p.service = config_.grabBanners ? svc.refineFromBanner(port, pr.banner)
                                        : svc.serviceName(port);
        if (p.service.isEmpty()) p.service = svc.serviceName(port);
        d.ports.push_back(p);
    }

    if (config_.detectOs) {
        OsFingerprinter osfp;
        d.osGuess = osfp.guess(d);
    }

    DeviceClassifier classifier;
    d.type = classifier.classify(d, gatewayIps_.contains(ipStr));

    RiskEngine risk;
    risk.evaluate(d);
    return d;
}

void ScanWorker::reportHostFinished(const Device& device, bool alive) {
    quint64 localDone = 0;
    {
        QMutexLocker lock(&resultsMutex_);
        ++done_;
        localDone = done_;
        summary_.scanned = done_;
        if (alive) {
            ++summary_.hostsUp;
            if (device.riskLevel >= RiskLevel::High) ++summary_.risky;
            devices_.insert(device.ipString(), device);
        }
    }
    if (alive) emit deviceDiscovered(device);
    emit progress(localDone, total_);
}

void ScanWorker::enrichMacsFromArp() {
    if (!platform_) return;
    // Probing populated the OS ARP cache; re-read it to fill in MAC/vendor.
    const auto arp = platform_->arpTable();
    QHash<QString, QString> fresh;
    for (const ArpEntry& e : arp) fresh.insert(e.ip.toString(), netutils::normalizeMac(e.mac));

    QList<Device> updated;
    {
        QMutexLocker lock(&resultsMutex_);
        for (auto it = devices_.begin(); it != devices_.end(); ++it) {
            const QString mac = fresh.value(it.key());
            if (mac.isEmpty() || mac == it.value().mac) continue;
            it.value().mac = mac;
            it.value().vendor = VendorLookup::instance().vendor(mac);
            if (it.value().vendor.contains(QLatin1String("VMware"), Qt::CaseInsensitive) ||
                VendorLookup::instance().isVirtualMac(mac))
                it.value().type = DeviceType::VirtualMachine;
            updated.push_back(it.value());
        }
    }
    for (const Device& d : updated) emit deviceUpdated(d);
}

}  // namespace pacn
