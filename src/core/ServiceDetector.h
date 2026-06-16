#pragma once

#include <QHash>
#include <QString>
#include <QStringList>

namespace pacn {

// Maps TCP ports to service names + risk flags (loaded from :/services.txt) and
// refines guesses using grabbed banners. Process-wide singleton; loaded once.
class ServiceDetector {
public:
    static ServiceDetector& instance();

    // Loads the bundled table. Safe to call multiple times (idempotent).
    void ensureLoaded();

    QString serviceName(quint16 port) const;
    QStringList flags(quint16 port) const;
    bool hasFlag(quint16 port, const QString& flag) const;

    // Returns a possibly more specific "service/version" string from a banner,
    // or the port's default service name when the banner is unhelpful.
    QString refineFromBanner(quint16 port, const QString& banner) const;

private:
    ServiceDetector() = default;
    struct Entry { QString name; QStringList flags; };
    QHash<quint16, Entry> table_;
    bool loaded_ = false;
};

}  // namespace pacn
