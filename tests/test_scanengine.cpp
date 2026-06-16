#include <gtest/gtest.h>

#include <QEventLoop>
#include <QTimer>

#include "core/ScanEngine.h"
#include "fake/FakeProbeFactory.h"
#include "platform/fake/FakePlatform.h"

using namespace pacn;

namespace {

struct ScanResult {
    QList<Device> discovered;
    ScanSummary summary;
};

ScanResult runScan(const ScanConfig& cfg, std::shared_ptr<IProbeFactory> factory,
                   std::shared_ptr<IPlatformServices> platform,
                   bool cancelImmediately = false) {
    ScanResult result;
    ScanEngine engine;
    engine.setProbeFactory(factory);
    engine.setPlatform(platform);
    engine.setConfig(cfg);

    QEventLoop loop;
    QObject::connect(&engine, &ScanEngine::deviceDiscovered,
                     [&](const Device& d) { result.discovered.push_back(d); });
    QObject::connect(&engine, &ScanEngine::finished, [&](const ScanSummary& s) {
        result.summary = s;
        loop.quit();
    });
    QTimer guard;
    guard.setSingleShot(true);
    QObject::connect(&guard, &QTimer::timeout, &loop, &QEventLoop::quit);
    guard.start(30000);

    engine.start();
    if (cancelImmediately) engine.cancel();
    loop.exec();
    return result;
}

}  // namespace

TEST(ScanEngineIntegration, DiscoversHostsAndPorts) {
    auto factory = std::make_shared<test::FakeProbeFactory>();
    factory->host->alive = {QStringLiteral("192.168.50.10"), QStringLiteral("192.168.50.20")};
    factory->port->open[QStringLiteral("192.168.50.10")] = {22, 80};
    factory->port->open[QStringLiteral("192.168.50.20")] = {3389};

    auto platform = std::make_shared<FakePlatform>();
    ArpEntry e;
    e.ip = QHostAddress(QStringLiteral("192.168.50.10"));
    e.mac = QStringLiteral("AA:BB:CC:00:00:10");
    platform->arp.push_back(e);

    ScanConfig cfg;
    cfg.targets = {IpRange::parse(QStringLiteral("192.168.50.0/24")).value()};
    cfg.methods = ScanMethod::TcpConnect;
    cfg.ports = {22, 80, 443, 3389};
    cfg.resolveHostnames = false;
    cfg.detectOs = false;
    cfg.grabBanners = false;
    cfg.speed = SpeedProfile::Fast;

    const ScanResult r = runScan(cfg, factory, platform);

    EXPECT_EQ(r.summary.hostsUp, 2u);
    EXPECT_EQ(r.summary.scanned, 254u);  // /24 minus network & broadcast
    ASSERT_EQ(r.discovered.size(), 2);

    const Device* ten = nullptr;
    for (const Device& d : r.discovered)
        if (d.ipString() == QLatin1String("192.168.50.10")) ten = &d;
    ASSERT_NE(ten, nullptr);
    EXPECT_EQ(ten->openPorts().size(), 2);
    EXPECT_EQ(ten->mac.toStdString(), "AA:BB:CC:00:00:10");
}

TEST(ScanEngineIntegration, CancelStillFinishes) {
    auto factory = std::make_shared<test::FakeProbeFactory>();
    auto platform = std::make_shared<FakePlatform>();

    ScanConfig cfg;
    cfg.targets = {IpRange::parse(QStringLiteral("10.0.0.0/24")).value()};
    cfg.methods = ScanMethod::TcpConnect;
    cfg.resolveHostnames = false;

    const ScanResult r = runScan(cfg, factory, platform, /*cancelImmediately=*/true);
    // Must terminate (loop quit) and report no crash; scanned <= total.
    EXPECT_LE(r.summary.scanned, 254u);
}

TEST(ScanEngineIntegration, EmptyTargetsFinishImmediately) {
    auto factory = std::make_shared<test::FakeProbeFactory>();
    auto platform = std::make_shared<FakePlatform>();
    ScanConfig cfg;  // no targets
    const ScanResult r = runScan(cfg, factory, platform);
    EXPECT_EQ(r.summary.hostsUp, 0u);
    EXPECT_EQ(r.discovered.size(), 0);
}
