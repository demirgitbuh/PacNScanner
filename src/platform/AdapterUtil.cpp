#include "platform/AdapterUtil.h"

#include <QNetworkInterface>

namespace pacn::adapterutil {

bool looksVirtual(const QString& name, const QString& humanName, const QString& mac) {
    const QString n = (name + QLatin1Char(' ') + humanName).toLower();
    if (n.contains(QLatin1String("vmware")) || n.contains(QLatin1String("virtualbox")) ||
        n.contains(QLatin1String("hyper-v")) || n.contains(QLatin1String("vethernet")) ||
        n.contains(QLatin1String("veth")) || n.contains(QLatin1String("docker")) ||
        n.contains(QLatin1String("vboxnet")) || n.contains(QLatin1String("virbr")) ||
        n.contains(QLatin1String("loopback pseudo")))
        return true;
    // Locally-administered MAC bit often indicates a virtual interface.
    const QString m = mac.trimmed();
    if (m.size() >= 2) {
        bool ok = false;
        const int firstByte = m.left(2).toInt(&ok, 16);
        if (ok && (firstByte & 0x02)) return true;
    }
    return false;
}

bool looksVpn(const QString& name, const QString& humanName) {
    const QString n = (name + QLatin1Char(' ') + humanName).toLower();
    return n.contains(QLatin1String("tun")) || n.contains(QLatin1String("tap")) ||
           n.contains(QLatin1String("wireguard")) || n.contains(QLatin1String("wg")) ||
           n.contains(QLatin1String("ppp")) || n.contains(QLatin1String("vpn")) ||
           n.contains(QLatin1String("openvpn")) || n.contains(QLatin1String("zerotier")) ||
           n.contains(QLatin1String("tailscale"));
}

bool looksWireless(const QString& name, const QString& humanName) {
    const QString n = (name + QLatin1Char(' ') + humanName).toLower();
    return n.contains(QLatin1String("wi-fi")) || n.contains(QLatin1String("wifi")) ||
           n.contains(QLatin1String("wlan")) || n.startsWith(QLatin1String("wl")) ||
           n.contains(QLatin1String("wireless")) || n.contains(QLatin1String("airport")) ||
           n.contains(QLatin1String("802.11"));
}

QList<AdapterInfo> enumerateViaQt() {
    QList<AdapterInfo> out;
    const QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : ifaces) {
        AdapterInfo a;
        a.name = iface.name();
        a.description = iface.humanReadableName();
        a.macAddress = iface.hardwareAddress();
        const QNetworkInterface::InterfaceFlags flags = iface.flags();
        a.isUp = flags.testFlag(QNetworkInterface::IsUp) &&
                 flags.testFlag(QNetworkInterface::IsRunning);
        a.isLoopback = flags.testFlag(QNetworkInterface::IsLoopBack);

        for (const QNetworkAddressEntry& e : iface.addressEntries()) {
            const QHostAddress ip = e.ip();
            if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
                a.ipv4.push_back(ip);
                if (a.primaryIpv4.isNull()) {
                    a.primaryIpv4 = ip;
                    a.prefixLength = e.prefixLength() > 0 ? e.prefixLength() : 24;
                }
            } else if (ip.protocol() == QAbstractSocket::IPv6Protocol) {
                a.ipv6.push_back(ip);
            }
        }

        a.isVirtual = looksVirtual(a.name, a.description, a.macAddress);
        a.isVpn = looksVpn(a.name, a.description);
        a.isWireless = looksWireless(a.name, a.description);
        out.push_back(a);
    }
    return out;
}

}  // namespace pacn::adapterutil
