#include "core/IpRange.h"

#include <QRegularExpression>

#include "core/NetUtils.h"

namespace pacn {

namespace {
quint32 toV4(const QHostAddress& a) { return a.toIPv4Address(); }
}  // namespace

Result<IpRange> IpRange::parse(const QString& tokenRaw) {
    const QString token = tokenRaw.trimmed();
    if (token.isEmpty()) return Result<IpRange>::err(QStringLiteral("empty target"));

    IpRange r;
    r.text_ = token;

    // --- CIDR ---------------------------------------------------------------
    if (token.contains(QLatin1Char('/'))) {
        const QPair<QHostAddress, int> subnet = QHostAddress::parseSubnet(token);
        if (subnet.first.isNull() || subnet.second < 0)
            return Result<IpRange>::err(QStringLiteral("invalid CIDR: %1").arg(token));

        r.prefix_ = subnet.second;
        if (subnet.first.protocol() == QAbstractSocket::IPv4Protocol) {
            const quint32 mask =
                subnet.second == 0 ? 0u : (0xFFFFFFFFu << (32 - subnet.second));
            const quint32 network = toV4(subnet.first) & mask;
            const quint32 broadcast = network | ~mask;
            if (subnet.second <= 30) {
                r.first_ = QHostAddress(network + 1);
                r.last_ = QHostAddress(broadcast - 1);
            } else {  // /31, /32 — every address is scannable
                r.first_ = QHostAddress(network);
                r.last_ = QHostAddress(broadcast);
            }
        } else {
            Q_IPV6ADDR base = subnet.first.toIPv6Address();
            Q_IPV6ADDR end = base;
            const int hostBits = 128 - subnet.second;
            for (int bit = 0; bit < hostBits; ++bit) {
                const int byte = 15 - (bit / 8);
                end[byte] = static_cast<quint8>(end[byte] | (1 << (bit % 8)));
            }
            r.first_ = QHostAddress(base);
            r.last_ = QHostAddress(end);
        }
        r.valid_ = true;
        return Result<IpRange>::ok(r);
    }

    // --- Range "a-b" / "a-LASTOCTET" ---------------------------------------
    if (token.contains(QLatin1Char('-'))) {
        const QStringList parts = token.split(QLatin1Char('-'));
        if (parts.size() != 2)
            return Result<IpRange>::err(QStringLiteral("invalid range: %1").arg(token));
        const QHostAddress a(parts[0].trimmed());
        if (a.isNull())
            return Result<IpRange>::err(QStringLiteral("invalid range start: %1").arg(token));

        // Short form "a.b.c.d-N" (N = final octet only) is detected from the
        // token shape, not by QHostAddress("N") being null (which is not
        // guaranteed across platforms).
        const QString endTok = parts[1].trimmed();
        const bool shortForm = a.protocol() == QAbstractSocket::IPv4Protocol &&
                               !endTok.contains(QLatin1Char('.')) &&
                               !endTok.contains(QLatin1Char(':'));
        QHostAddress b;
        if (shortForm) {
            bool ok = false;
            const uint lastOctet = endTok.toUInt(&ok);
            if (ok && lastOctet <= 255)
                b = QHostAddress((toV4(a) & 0xFFFFFF00u) | lastOctet);
        } else {
            b = QHostAddress(endTok);
        }
        if (b.isNull())
            return Result<IpRange>::err(QStringLiteral("invalid range end: %1").arg(token));
        if (a.protocol() != b.protocol())
            return Result<IpRange>::err(QStringLiteral("mixed IP families: %1").arg(token));
        if (netutils::compareIp(a, b) > 0)
            return Result<IpRange>::err(QStringLiteral("range start after end: %1").arg(token));

        r.first_ = a;
        r.last_ = b;
        r.valid_ = true;
        return Result<IpRange>::ok(r);
    }

    // --- Single host --------------------------------------------------------
    const QHostAddress single(token);
    if (single.isNull())
        return Result<IpRange>::err(QStringLiteral("invalid address: %1").arg(token));
    r.first_ = single;
    r.last_ = single;
    r.prefix_ = single.protocol() == QAbstractSocket::IPv4Protocol ? 32 : 128;
    r.valid_ = true;
    return Result<IpRange>::ok(r);
}

QList<IpRange> IpRange::parseList(const QString& text, QStringList* errors) {
    static const QRegularExpression sep(QStringLiteral("[,;\\s]+"));
    QList<IpRange> out;
    const QStringList tokens = text.split(sep, Qt::SkipEmptyParts);
    for (const QString& t : tokens) {
        auto r = parse(t);
        if (r)
            out.push_back(r.value());
        else if (errors)
            errors->push_back(r.error());
    }
    return out;
}

quint64 IpRange::hostCount() const {
    if (!valid_) return 0;
    if (!isIpv6()) {
        return static_cast<quint64>(toV4(last_) - toV4(first_)) + 1;
    }
    const Q_IPV6ADDR a = first_.toIPv6Address();
    const Q_IPV6ADDR b = last_.toIPv6Address();
    for (int i = 0; i < 8; ++i)
        if (a[i] != b[i]) return kHugeCount;
    quint64 lo_a = 0, lo_b = 0;
    for (int i = 8; i < 16; ++i) {
        lo_a = (lo_a << 8) | a[i];
        lo_b = (lo_b << 8) | b[i];
    }
    return (lo_b - lo_a) + 1;
}

QList<QHostAddress> IpRange::expand(quint64 maxHosts) const {
    QList<QHostAddress> out;
    if (!valid_) return out;

    quint64 cap = maxHosts;
    if (isIpv6())
        cap = (cap == 0) ? kIpv6ExpandCap : qMin<quint64>(cap, kIpv6ExpandCap);

    QHostAddress cur = first_;
    quint64 produced = 0;
    while (true) {
        const bool skipV4 = !isIpv6() && prefix_ >= 0 && prefix_ <= 30 &&
                            netutils::isNetworkOrBroadcast(cur, prefix_, first_);
        if (!skipV4) {
            out.push_back(cur);
            if (cap != 0 && ++produced >= cap) break;
        }
        if (netutils::compareIp(cur, last_) >= 0) break;
        cur = netutils::nextAddress(cur);
    }
    return out;
}

IpRange::Estimate IpRange::estimate(quint64 totalHosts, int concurrency, int perHostMs) {
    Estimate e;
    e.hosts = totalHosts;
    const int c = qMax(1, concurrency);
    e.seconds = (totalHosts == kHugeCount)
                    ? -1.0
                    : (static_cast<double>(totalHosts) * perHostMs) / c / 1000.0;
    if (totalHosts == kHugeCount || totalHosts > 1000000)
        e.load = QStringLiteral("Very High");
    else if (totalHosts > 65536)
        e.load = QStringLiteral("High");
    else if (totalHosts > 1024)
        e.load = QStringLiteral("Moderate");
    else
        e.load = QStringLiteral("Low");
    return e;
}

}  // namespace pacn
