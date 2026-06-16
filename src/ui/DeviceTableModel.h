#pragma once

#include <QAbstractTableModel>
#include <QColor>
#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QTimer>

#include "core/Device.h"

namespace pacn {

// Table model backing the device list. Provides a dedicated SortRole so the
// proxy can sort IPs numerically and risk by score, and a fading background for
// freshly discovered rows (the "animated results" effect).
class DeviceTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column {
        ColFavorite = 0,
        ColStatus,
        ColIp,
        ColMac,
        ColHostname,
        ColVendor,
        ColOs,
        ColType,
        ColPorts,
        ColServices,
        ColRisk,
        ColumnCount
    };
    static constexpr int SortRole = Qt::UserRole + 1;
    static constexpr int DeviceRole = Qt::UserRole + 2;

    explicit DeviceTableModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation o, int role) const override;

    void upsert(const Device& device);   // add or update (keyed by IP)
    void setDevices(const QList<Device>& devices);
    void clear();

    Device deviceAt(int row) const;
    int rowForIp(const QString& ip) const;
    QList<Device> devices() const { return devices_; }

    void setAccent(const QColor& accent);
    void toggleFavorite(int row);

private:
    QString sortKey(int column, const Device& d) const;

    QList<Device> devices_;
    QHash<QString, int> rowByIp_;
    QColor accent_{0x2b, 0xbf, 0xd6};

    QTimer highlightTimer_;
    QElapsedTimer clock_;
    QHash<QString, qint64> highlightAt_;  // ip -> ms when added
};

}  // namespace pacn
