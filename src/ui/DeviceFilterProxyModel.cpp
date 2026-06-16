#include "ui/DeviceFilterProxyModel.h"

#include "ui/DeviceTableModel.h"

namespace pacn {

DeviceFilterProxyModel::DeviceFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent) {
    setSortRole(DeviceTableModel::SortRole);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);
}

void DeviceFilterProxyModel::setFilterText(const QString& text) {
    text_ = text.trimmed();
    invalidateFilter();
}

void DeviceFilterProxyModel::setOnlyRisky(bool on) {
    onlyRisky_ = on;
    invalidateFilter();
}

void DeviceFilterProxyModel::setOnlyFavorites(bool on) {
    onlyFavorites_ = on;
    invalidateFilter();
}

void DeviceFilterProxyModel::setPrioritize(bool on) {
    prioritize_ = on;
    invalidate();
}

bool DeviceFilterProxyModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex& sourceParent) const {
    const QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    const Device d =
        sourceModel()->data(idx, DeviceTableModel::DeviceRole).value<Device>();

    if (onlyRisky_ && d.riskLevel < RiskLevel::High) return false;
    if (onlyFavorites_ && !d.favorite) return false;

    if (text_.isEmpty()) return true;
    const QString hay =
        QStringList({d.ipString(), d.mac, d.hostname, d.vendor, d.osGuess,
                     toString(d.type), d.serviceNames().join(QLatin1Char(' ')),
                     d.openPortNumbers().join(QLatin1Char(' '))})
            .join(QLatin1Char(' '))
            .toLower();
    return hay.contains(text_.toLower());
}

bool DeviceFilterProxyModel::lessThan(const QModelIndex& left,
                                      const QModelIndex& right) const {
    if (prioritize_) {
        const Device l =
            sourceModel()->data(left, DeviceTableModel::DeviceRole).value<Device>();
        const Device r =
            sourceModel()->data(right, DeviceTableModel::DeviceRole).value<Device>();
        if (l.isPrioritized() != r.isPrioritized()) return l.isPrioritized();
    }
    const QString lv = sourceModel()->data(left, DeviceTableModel::SortRole).toString();
    const QString rv = sourceModel()->data(right, DeviceTableModel::SortRole).toString();
    return QString::compare(lv, rv, Qt::CaseInsensitive) < 0;
}

}  // namespace pacn
