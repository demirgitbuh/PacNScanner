#pragma once

#include <QHostAddress>
#include <QList>
#include <QString>

#include "core/Result.h"

namespace pacn {

// A scannable IPv4/IPv6 target expressed as CIDR ("192.168.1.0/24"),
// inclusive range ("10.0.0.1-10.0.0.50" or short "10.0.0.1-50"), or a single
// host ("10.0.0.5", "fe80::1"). Multiple targets can be scanned at once.
class IpRange {
public:
    // Sentinel returned by hostCount() for IPv6 ranges too large to enumerate.
    static constexpr quint64 kHugeCount = ~quint64(0);
    // Practical cap when expanding an IPv6 prefix/range to concrete hosts.
    static constexpr quint64 kIpv6ExpandCap = 65536;

    IpRange() = default;

    static Result<IpRange> parse(const QString& token);
    // Splits on comma / whitespace / newlines; collects per-token errors.
    static QList<IpRange> parseList(const QString& text, QStringList* errors = nullptr);

    bool isValid() const { return valid_; }
    bool isIpv6() const { return first_.protocol() == QAbstractSocket::IPv6Protocol; }
    QHostAddress first() const { return first_; }
    QHostAddress last() const { return last_; }
    int prefixLength() const { return prefix_; }

    // Number of *scannable* hosts (IPv4 network/broadcast already excluded).
    quint64 hostCount() const;

    // Concrete addresses to probe. `maxHosts` (0 = unlimited for IPv4) caps the
    // result; IPv6 is always capped at kIpv6ExpandCap regardless.
    QList<QHostAddress> expand(quint64 maxHosts = 0) const;

    QString toString() const { return text_; }

    struct Estimate {
        quint64 hosts = 0;
        double seconds = 0.0;
        QString load;  // "Low" | "Moderate" | "High" | "Very High"
    };
    static Estimate estimate(quint64 totalHosts, int concurrency, int perHostMs);

private:
    bool valid_ = false;
    QHostAddress first_;
    QHostAddress last_;
    int prefix_ = -1;
    QString text_;
};

}  // namespace pacn
