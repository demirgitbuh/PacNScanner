#include "core/OsFingerprinter.h"

namespace pacn {

QString OsFingerprinter::fromTtl(int ttl) {
    if (ttl <= 0) return {};
    // Hosts decrement TTL per hop; map to the nearest common initial value.
    if (ttl <= 64) return QStringLiteral("Linux/Unix");
    if (ttl <= 128) return QStringLiteral("Windows");
    return QStringLiteral("Network device");
}

QString OsFingerprinter::guess(const Device& device) const {
    // Banner signals first (most reliable).
    for (const Port& p : device.ports) {
        const QString b = p.banner.toLower();
        if (b.isEmpty()) continue;
        if (b.contains(QLatin1String("windows")) || b.contains(QLatin1String("microsoft")))
            return QStringLiteral("Windows");
        if (b.contains(QLatin1String("ubuntu")) || b.contains(QLatin1String("debian")) ||
            b.contains(QLatin1String("linux")))
            return QStringLiteral("Linux");
        if (b.contains(QLatin1String("openssh")) && b.contains(QLatin1String("freebsd")))
            return QStringLiteral("FreeBSD");
        if (b.contains(QLatin1String("mikrotik")) || b.contains(QLatin1String("routeros")))
            return QStringLiteral("MikroTik RouterOS");
    }

    // Service-based hints.
    const QStringList services = device.serviceNames();
    if (services.contains(QStringLiteral("microsoft-ds")) ||
        services.contains(QStringLiteral("ms-wbt-rdp")) ||
        services.contains(QStringLiteral("netbios-ssn")))
        return QStringLiteral("Windows");

    // TTL last (least reliable across hops).
    return fromTtl(device.ttl);
}

}  // namespace pacn
