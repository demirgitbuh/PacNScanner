#include "core/PortProfiles.h"

#include <QSet>
#include <algorithm>

namespace pacn::portprofiles {

QStringList names() {
    return {QStringLiteral("Top 100"), QStringLiteral("Top 1000"),
            QStringLiteral("Well-known (1-1024)"), QStringLiteral("Full (1-65535)"),
            QStringLiteral("Custom")};
}

QList<quint16> top100() {
    return {7,    20,   21,   22,   23,   25,   26,   37,   53,   79,   80,   81,
            88,   106,  110,  111,  113,  119,  135,  139,  143,  144,  179,  199,
            389,  427,  443,  444,  445,  465,  513,  514,  515,  543,  544,  548,
            554,  587,  631,  646,  873,  990,  993,  995,  1025, 1026, 1027, 1028,
            1029, 1110, 1433, 1720, 1723, 1755, 1900, 2000, 2001, 2049, 2121, 2717,
            3000, 3128, 3306, 3389, 3986, 4899, 5000, 5009, 5051, 5060, 5101, 5190,
            5357, 5432, 5631, 5666, 5800, 5900, 6000, 6001, 6646, 7070, 8000, 8008,
            8009, 8080, 8081, 8443, 8888, 9100, 9999, 10000, 32768, 49152, 49153,
            49154, 49155, 49156, 49157};
}

QList<quint16> wellKnown() {
    QList<quint16> out;
    out.reserve(1024);
    for (quint16 p = 1; p <= 1024; ++p) out.push_back(p);
    return out;
}

QList<quint16> top1000() {
    // Pragmatic "top 1000": the well-known range plus a curated set of common
    // high ports. (Documented approximation of nmap's frequency table.)
    QSet<quint16> set;
    for (quint16 p : wellKnown()) set.insert(p);
    static const quint16 extra[] = {
        1080, 1099, 1194, 1234, 1337, 1352, 1433, 1434, 1521, 1604, 1701, 1723,
        1755, 1812, 1813, 1883, 1900, 2000, 2049, 2082, 2083, 2086, 2087, 2095,
        2096, 2121, 2181, 2222, 2375, 2376, 2483, 2484, 3000, 3128, 3260, 3268,
        3306, 3333, 3389, 3478, 3690, 3724, 4000, 4040, 4444, 4500, 4567, 4711,
        4848, 5000, 5060, 5061, 5222, 5269, 5353, 5357, 5432, 5555, 5601, 5672,
        5683, 5800, 5900, 5938, 5984, 6000, 6379, 6443, 6660, 6667, 6881, 7000,
        7001, 7070, 7077, 7443, 7474, 7547, 8000, 8008, 8009, 8080, 8081, 8086,
        8088, 8123, 8181, 8333, 8443, 8500, 8554, 8649, 8765, 8888, 9000, 9001,
        9042, 9090, 9091, 9092, 9100, 9200, 9300, 9418, 9999, 10000, 11211,
        15672, 16992, 16993, 25565, 27017, 27018, 28017, 32400, 49152, 49153,
        49154, 50000, 51820, 54321, 55443, 62078};
    for (quint16 p : extra) set.insert(p);
    QList<quint16> out(set.cbegin(), set.cend());
    std::sort(out.begin(), out.end());
    return out;
}

QList<quint16> full() {
    QList<quint16> out;
    out.reserve(65535);
    for (int p = 1; p <= 65535; ++p) out.push_back(static_cast<quint16>(p));
    return out;
}

QList<quint16> forName(const QString& name) {
    if (name == QLatin1String("Top 100")) return top100();
    if (name == QLatin1String("Top 1000")) return top1000();
    if (name == QLatin1String("Well-known (1-1024)")) return wellKnown();
    if (name == QLatin1String("Full (1-65535)")) return full();
    return {};  // Custom: caller supplies parseSpec()
}

QList<quint16> parseSpec(const QString& spec) {
    QSet<quint16> set;
    const QStringList parts = spec.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (QString part : parts) {
        part = part.trimmed();
        if (part.isEmpty()) continue;
        if (part.contains(QLatin1Char('-'))) {
            const QStringList se = part.split(QLatin1Char('-'));
            if (se.size() != 2) continue;
            bool ok1 = false, ok2 = false;
            int a = se[0].trimmed().toInt(&ok1);
            int b = se[1].trimmed().toInt(&ok2);
            if (!ok1 || !ok2) continue;
            if (a > b) std::swap(a, b);
            a = qMax(1, a);
            b = qMin(65535, b);
            for (int p = a; p <= b; ++p) set.insert(static_cast<quint16>(p));
        } else {
            bool ok = false;
            const int p = part.toInt(&ok);
            if (ok && p >= 1 && p <= 65535) set.insert(static_cast<quint16>(p));
        }
    }
    QList<quint16> out(set.cbegin(), set.cend());
    std::sort(out.begin(), out.end());
    return out;
}

}  // namespace pacn::portprofiles
