#pragma once

#include <QSortFilterProxyModel>

namespace pacn {

// Filtering + smart sorting for the device table. Sorts via the model's
// SortRole (numeric IP/risk), supports a free-text filter and quick toggles,
// and can float favourites / risky / new devices to the top.
class DeviceFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit DeviceFilterProxyModel(QObject* parent = nullptr);

    void setFilterText(const QString& text);
    void setOnlyRisky(bool on);
    void setOnlyFavorites(bool on);
    void setPrioritize(bool on);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    QString text_;
    bool onlyRisky_ = false;
    bool onlyFavorites_ = false;
    bool prioritize_ = false;
};

}  // namespace pacn
