#include "platform/generic/GenericUnixPlatform.h"

#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QtGlobal>

#include "core/NetUtils.h"
#include "platform/AdapterUtil.h"

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

namespace pacn {

namespace {

QString runCommand(const QString& program, const QStringList& args, int timeoutMs = 2000) {
    QProcess proc;
    proc.start(program, args);
    if (!proc.waitForStarted(800)) return {};
    if (!proc.waitForFinished(timeoutMs)) {
        proc.kill();
        proc.waitForFinished(200);
        return {};
    }
    return QString::fromLocal8Bit(proc.readAllStandardOutput());
}

const QRegularExpression& ipv4Re() {
    static const QRegularExpression re(QStringLiteral("(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})"));
    return re;
}

const QRegularExpression& macRe() {
    static const QRegularExpression re(
        QStringLiteral("([0-9a-fA-F]{1,2}(?::[0-9a-fA-F]{1,2}){5})"));
    return re;
}

}  // namespace

QList<AdapterInfo> GenericUnixPlatform::enumerateAdapters() {
    QList<AdapterInfo> adapters = adapterutil::enumerateViaQt();
    const QString ssid = currentSsid();
    if (!ssid.isEmpty()) {
        for (AdapterInfo& a : adapters)
            if (a.isWireless && a.isUp) a.wifiSsid = ssid;
    }
    return adapters;
}

QList<ArpEntry> GenericUnixPlatform::arpTable() {
    QList<ArpEntry> out;

    // Linux fast path: /proc/net/arp.
    QFile procArp(QStringLiteral("/proc/net/arp"));
    if (procArp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString text = QString::fromLocal8Bit(procArp.readAll());
        const QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (int i = 1; i < lines.size(); ++i) {  // skip header
            const QStringList cols = lines[i].split(QRegularExpression(QStringLiteral("\\s+")),
                                                    Qt::SkipEmptyParts);
            if (cols.size() < 6) continue;
            const QString mac = netutils::normalizeMac(cols[3]);
            if (mac.isEmpty() || mac == QLatin1String("00:00:00:00:00:00")) continue;
            ArpEntry e;
            e.ip = QHostAddress(cols[0]);
            e.mac = mac;
            e.adapter = cols[5];
            out.push_back(e);
        }
        if (!out.isEmpty()) return out;
    }

    // Portable fallback: parse `ip neigh` then `arp -a`.
    QString text = runCommand(QStringLiteral("ip"), {QStringLiteral("neigh")});
    if (text.isEmpty())
        text = runCommand(QStringLiteral("arp"), {QStringLiteral("-a")});

    const QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        const auto ipM = ipv4Re().match(line);
        const auto macM = macRe().match(line);
        if (!ipM.hasMatch() || !macM.hasMatch()) continue;
        const QString mac = netutils::normalizeMac(macM.captured(1));
        if (mac.isEmpty() || mac == QLatin1String("00:00:00:00:00:00")) continue;
        ArpEntry e;
        e.ip = QHostAddress(ipM.captured(1));
        e.mac = mac;
        out.push_back(e);
    }
    return out;
}

QList<GatewayInfo> GenericUnixPlatform::defaultGateways() {
    QList<GatewayInfo> out;

    // Linux: `ip route show default` => "default via 192.168.1.1 dev eth0".
    QString text = runCommand(QStringLiteral("ip"),
                              {QStringLiteral("route"), QStringLiteral("show"),
                               QStringLiteral("default")});
    if (text.isEmpty())  // macOS/BSD
        text = runCommand(QStringLiteral("route"),
                          {QStringLiteral("-n"), QStringLiteral("get"),
                           QStringLiteral("default")});
    if (text.isEmpty())
        text = runCommand(QStringLiteral("netstat"), {QStringLiteral("-rn")});

    const QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        const QString l = line.trimmed();
        const bool isDefault = l.startsWith(QLatin1String("default")) ||
                               l.contains(QLatin1String("via")) ||
                               l.contains(QLatin1String("gateway"));
        if (!isDefault) continue;
        const auto ipM = ipv4Re().match(l);
        if (!ipM.hasMatch()) continue;
        const QHostAddress addr(ipM.captured(1));
        if (addr.isNull()) continue;
        bool dup = false;
        for (const GatewayInfo& g : out)
            if (g.address == addr) dup = true;
        if (dup) continue;
        GatewayInfo g;
        g.address = addr;
        out.push_back(g);
    }
    return out;
}

bool GenericUnixPlatform::isElevated() {
#if defined(Q_OS_UNIX)
    return ::geteuid() == 0;
#else
    return false;
#endif
}

bool GenericUnixPlatform::rawSocketsAvailable() { return isElevated(); }

QString GenericUnixPlatform::currentSsid() {
#if defined(Q_OS_MACOS)
    const QString airport =
        QStringLiteral("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/"
                       "Current/Resources/airport");
    const QString out = runCommand(airport, {QStringLiteral("-I")});
    static const QRegularExpression re(QStringLiteral("\\bSSID:\\s*(.+)"));
    const auto m = re.match(out);
    if (m.hasMatch()) return m.captured(1).trimmed();
    return {};
#elif defined(Q_OS_LINUX)
    QString out = runCommand(QStringLiteral("iwgetid"), {QStringLiteral("-r")});
    if (!out.trimmed().isEmpty()) return out.trimmed();
    out = runCommand(QStringLiteral("nmcli"),
                     {QStringLiteral("-t"), QStringLiteral("-f"),
                      QStringLiteral("active,ssid"), QStringLiteral("dev"),
                      QStringLiteral("wifi")});
    for (const QString& line : out.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        if (line.startsWith(QLatin1String("yes:")))
            return line.section(QLatin1Char(':'), 1).trimmed();
    }
    return {};
#else
    return {};
#endif
}

}  // namespace pacn
