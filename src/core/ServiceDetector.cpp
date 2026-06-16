#include "core/ServiceDetector.h"

#include <QFile>
#include <QMutex>
#include <QTextStream>

namespace pacn {

ServiceDetector& ServiceDetector::instance() {
    static ServiceDetector inst;
    return inst;
}

void ServiceDetector::ensureLoaded() {
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    if (loaded_) return;
    loaded_ = true;

    QFile f(QStringLiteral(":/services.txt"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Minimal built-in fallback so detection still works without resources.
        table_[22] = {QStringLiteral("ssh"), {QStringLiteral("admin")}};
        table_[80] = {QStringLiteral("http"), {QStringLiteral("web")}};
        table_[443] = {QStringLiteral("https"), {QStringLiteral("web")}};
        table_[23] = {QStringLiteral("telnet"),
                      {QStringLiteral("plaintext"), QStringLiteral("admin")}};
        return;
    }
    QTextStream in(&f);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) continue;
        const QStringList cols = line.split(QLatin1Char('\t'), Qt::SkipEmptyParts);
        if (cols.size() < 2) continue;
        bool ok = false;
        const quint16 port = static_cast<quint16>(cols[0].toUInt(&ok));
        if (!ok) continue;
        Entry e;
        e.name = cols[1].trimmed();
        if (cols.size() >= 3)
            e.flags = cols[2].split(QLatin1Char(','), Qt::SkipEmptyParts);
        table_.insert(port, e);
    }
}

QString ServiceDetector::serviceName(quint16 port) const {
    const auto it = table_.constFind(port);
    return it != table_.cend() ? it->name : QString();
}

QStringList ServiceDetector::flags(quint16 port) const {
    const auto it = table_.constFind(port);
    return it != table_.cend() ? it->flags : QStringList();
}

bool ServiceDetector::hasFlag(quint16 port, const QString& flag) const {
    return flags(port).contains(flag);
}

QString ServiceDetector::refineFromBanner(quint16 port, const QString& banner) const {
    const QString base = serviceName(port);
    if (banner.isEmpty()) return base;

    const QString b = banner.trimmed();
    const QString lower = b.toLower();

    // Common banner signatures.
    if (lower.startsWith(QLatin1String("ssh-")))
        return b.section(QLatin1Char(' '), 0, 0);  // e.g. "SSH-2.0-OpenSSH_9.6"
    if (lower.contains(QLatin1String("server:"))) {
        const int idx = lower.indexOf(QLatin1String("server:"));
        QString server = b.mid(idx + 7).section(QLatin1Char('\r'), 0, 0).trimmed();
        if (!server.isEmpty()) return server;
    }
    if (lower.startsWith(QLatin1String("220")) && (port == 21 || port == 25 || port == 587))
        return b.mid(3).trimmed().left(60);
    if (lower.contains(QLatin1String("http/1.")) && !base.isEmpty())
        return base;

    return base.isEmpty() ? b.left(40) : base;
}

}  // namespace pacn
