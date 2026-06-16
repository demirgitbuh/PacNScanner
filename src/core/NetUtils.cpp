#include "core/NetUtils.h"

#include <QRegularExpression>

namespace pacn::netutils {

QString normalizeMac(const QString& raw) {
    static const QRegularExpression nonHex(QStringLiteral("[^0-9a-fA-F]"));
    QString hex = raw;
    hex.remove(nonHex);
    if (hex.size() < 12) return {};
    hex = hex.left(12).toUpper();
    QString out;
    out.reserve(17);
    for (int i = 0; i < 12; i += 2) {
        if (i > 0) out.append(QLatin1Char(':'));
        out.append(hex.mid(i, 2));
    }
    return out;
}

QString ouiKey(const QString& mac) {
    static const QRegularExpression nonHex(QStringLiteral("[^0-9a-fA-F]"));
    QString hex = mac;
    hex.remove(nonHex);
    if (hex.size() < 6) return {};
    return hex.left(6).toUpper();
}

int compareIp(const QHostAddress& a, const QHostAddress& b) {
    const bool a4 = a.protocol() == QAbstractSocket::IPv4Protocol;
    const bool b4 = b.protocol() == QAbstractSocket::IPv4Protocol;
    if (a4 != b4) return a4 ? -1 : 1;  // IPv4 first
    if (a4) {
        const quint32 ia = a.toIPv4Address();
        const quint32 ib = b.toIPv4Address();
        if (ia < ib) return -1;
        if (ia > ib) return 1;
        return 0;
    }
    const Q_IPV6ADDR ia = a.toIPv6Address();
    const Q_IPV6ADDR ib = b.toIPv6Address();
    for (int i = 0; i < 16; ++i) {
        if (ia[i] < ib[i]) return -1;
        if (ia[i] > ib[i]) return 1;
    }
    return 0;
}

QHostAddress nextAddress(const QHostAddress& addr) {
    if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
        const quint32 v = addr.toIPv4Address();
        if (v == 0xFFFFFFFFu) return addr;
        return QHostAddress(v + 1);
    }
    Q_IPV6ADDR v = addr.toIPv6Address();
    for (int i = 15; i >= 0; --i) {
        if (v[i] != 0xFF) { v[i] = static_cast<quint8>(v[i] + 1); break; }
        v[i] = 0;
    }
    return QHostAddress(v);
}

bool isPrivate(const QHostAddress& addr) {
    if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
        const quint32 v = addr.toIPv4Address();
        const quint8 a = (v >> 24) & 0xFF;
        const quint8 b = (v >> 16) & 0xFF;
        if (a == 10) return true;                       // 10.0.0.0/8
        if (a == 172 && b >= 16 && b <= 31) return true; // 172.16.0.0/12
        if (a == 192 && b == 168) return true;           // 192.168.0.0/16
        if (a == 169 && b == 254) return true;           // link-local
        return false;
    }
    if (addr.isLinkLocal()) return true;
    const Q_IPV6ADDR v = addr.toIPv6Address();
    return (v[0] & 0xFE) == 0xFC;  // fc00::/7 unique local
}

bool isNetworkOrBroadcast(const QHostAddress& addr, int prefixLength,
                          const QHostAddress& network) {
    if (addr.protocol() != QAbstractSocket::IPv4Protocol) return false;
    if (prefixLength >= 31 || prefixLength < 0) return false;  // /31,/32 carry no bcast
    const quint32 mask = prefixLength == 0 ? 0u : (0xFFFFFFFFu << (32 - prefixLength));
    const quint32 net = network.toIPv4Address() & mask;
    const quint32 bcast = net | ~mask;
    const quint32 v = addr.toIPv4Address();
    return v == net || v == bcast;
}

}  // namespace pacn::netutils
