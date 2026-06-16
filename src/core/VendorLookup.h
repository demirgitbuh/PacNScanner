#pragma once

#include <QHash>
#include <QString>

namespace pacn {

// Resolves a MAC address to its hardware vendor using a bundled OUI subset
// (:/oui.txt) and, when present, a full IEEE registry dropped at
// <dataDir>/oui.csv. Online lookup is intentionally opt-in (privacy).
class VendorLookup {
public:
    static VendorLookup& instance();

    void ensureLoaded();
    // Adds the full IEEE registry if found under dataDir (oui.csv / oui.txt).
    void loadExternal(const QString& dataDir);

    QString vendor(const QString& mac) const;
    bool isVirtualMac(const QString& mac) const;

private:
    VendorLookup() = default;
    QHash<QString, QString> table_;  // OUI(6 hex) -> vendor
    bool loaded_ = false;
};

}  // namespace pacn
