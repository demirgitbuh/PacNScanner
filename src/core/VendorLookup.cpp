#include "core/VendorLookup.h"

#include <QDir>
#include <QFile>
#include <QMutex>
#include <QTextStream>

#include "core/NetUtils.h"

namespace pacn {

VendorLookup& VendorLookup::instance() {
    static VendorLookup inst;
    return inst;
}

void VendorLookup::ensureLoaded() {
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    if (loaded_) return;
    loaded_ = true;

    QFile f(QStringLiteral(":/oui.txt"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) continue;
        const int tab = line.indexOf(QLatin1Char('\t'));
        if (tab < 0) continue;
        const QString oui = netutils::ouiKey(line.left(tab));
        const QString vendor = line.mid(tab + 1).trimmed();
        if (!oui.isEmpty() && !vendor.isEmpty()) table_.insert(oui, vendor);
    }
}

void VendorLookup::loadExternal(const QString& dataDir) {
    // Accepts the IEEE "oui.csv" (Registry,Assignment,Org Name,...) or a plain
    // "OUI<TAB>Vendor" file.
    for (const QString& name : {QStringLiteral("oui.csv"), QStringLiteral("oui_full.txt")}) {
        const QString path = QDir(dataDir).filePath(name);
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        QTextStream in(&f);
        while (!in.atEnd()) {
            const QString line = in.readLine();
            if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) continue;
            if (name.endsWith(QLatin1String(".csv"))) {
                const QStringList cols = line.split(QLatin1Char(','));
                if (cols.size() < 3) continue;
                const QString oui = netutils::ouiKey(cols[1]);
                QString vendor = cols[2].trimmed();
                if (vendor.startsWith(QLatin1Char('"')) && vendor.endsWith(QLatin1Char('"')))
                    vendor = vendor.mid(1, vendor.size() - 2);
                if (!oui.isEmpty() && !vendor.isEmpty()) table_.insert(oui, vendor);
            } else {
                const int tab = line.indexOf(QLatin1Char('\t'));
                if (tab < 0) continue;
                const QString oui = netutils::ouiKey(line.left(tab));
                const QString vendor = line.mid(tab + 1).trimmed();
                if (!oui.isEmpty() && !vendor.isEmpty()) table_.insert(oui, vendor);
            }
        }
    }
}

QString VendorLookup::vendor(const QString& mac) const {
    const QString oui = netutils::ouiKey(mac);
    if (oui.isEmpty()) return {};
    return table_.value(oui);
}

bool VendorLookup::isVirtualMac(const QString& mac) const {
    const QString v = vendor(mac).toLower();
    return v.contains(QLatin1String("vmware")) || v.contains(QLatin1String("virtualbox")) ||
           v.contains(QLatin1String("qemu")) || v.contains(QLatin1String("hyper-v")) ||
           v.contains(QLatin1String("parallels")) || v.contains(QLatin1String("xen"));
}

}  // namespace pacn
