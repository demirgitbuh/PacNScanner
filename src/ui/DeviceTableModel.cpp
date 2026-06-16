#include "ui/DeviceTableModel.h"

#include "ui/IconProvider.h"

namespace pacn {

namespace {
constexpr qint64 kHighlightMs = 1600;
}

DeviceTableModel::DeviceTableModel(QObject* parent) : QAbstractTableModel(parent) {
    clock_.start();
    highlightTimer_.setInterval(50);
    connect(&highlightTimer_, &QTimer::timeout, this, [this] {
        bool any = false;
        const qint64 now = clock_.elapsed();
        for (auto it = highlightAt_.begin(); it != highlightAt_.end();) {
            if (now - it.value() > kHighlightMs)
                it = highlightAt_.erase(it);
            else {
                any = true;
                ++it;
            }
        }
        if (!devices_.isEmpty())
            emit dataChanged(index(0, 0), index(rowCount() - 1, ColumnCount - 1),
                             {Qt::BackgroundRole});
        if (!any) highlightTimer_.stop();
    });
}

int DeviceTableModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : devices_.size();
}

int DeviceTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : ColumnCount;
}

QString DeviceTableModel::sortKey(int column, const Device& d) const {
    switch (column) {
        case ColFavorite:
            return d.favorite ? QStringLiteral("0") : QStringLiteral("1");
        case ColStatus:
            return QString::number(static_cast<int>(d.status));
        case ColIp: {
            if (d.ip.protocol() == QAbstractSocket::IPv4Protocol) {
                const quint32 v = d.ip.toIPv4Address();
                return QStringLiteral("4%1")
                    .arg(v, 10, 10, QLatin1Char('0'));
            }
            const Q_IPV6ADDR a = d.ip.toIPv6Address();
            QString s = QStringLiteral("6");
            for (int i = 0; i < 16; ++i)
                s += QStringLiteral("%1").arg(a[i], 2, 16, QLatin1Char('0'));
            return s;
        }
        case ColPorts:
            return QStringLiteral("%1").arg(d.openPorts().size(), 6, 10, QLatin1Char('0'));
        case ColRisk:
            return QStringLiteral("%1").arg(d.riskScore, 4, 10, QLatin1Char('0'));
        default:
            return data(index(rowByIp_.value(d.ipString()), column), Qt::DisplayRole).toString();
    }
}

QVariant DeviceTableModel::data(const QModelIndex& idx, int role) const {
    if (!idx.isValid() || idx.row() >= devices_.size()) return {};
    const Device& d = devices_[idx.row()];
    const int col = idx.column();

    if (role == DeviceRole) return QVariant::fromValue(d);

    if (role == Qt::DisplayRole) {
        switch (col) {
            case ColFavorite: return d.favorite ? QStringLiteral("★") : QString();
            case ColStatus:   return toString(d.status);
            case ColIp:       return d.ipString();
            case ColMac:      return d.mac;
            case ColHostname: return d.hostname;
            case ColVendor:   return d.vendor;
            case ColOs:       return d.osGuess;
            case ColType:     return toString(d.type);
            case ColPorts:    return d.openPortNumbers().join(QStringLiteral(", "));
            case ColServices: return d.serviceNames().join(QStringLiteral(", "));
            case ColRisk:
                return d.riskScore > 0 ? QStringLiteral("%1 · %2")
                                             .arg(d.riskScore)
                                             .arg(toString(d.riskLevel))
                                       : toString(d.riskLevel);
        }
    }

    if (role == SortRole) return sortKey(col, d);

    if (role == Qt::DecorationRole) {
        if (col == ColStatus) return iconprovider::statusIcon(d.status);
        if (col == ColRisk) return iconprovider::dotIcon(iconprovider::riskColor(d.riskLevel));
    }

    if (role == Qt::ForegroundRole && col == ColFavorite && d.favorite)
        return QColor(0xf1, 0xc4, 0x0f);

    if (role == Qt::TextAlignmentRole) {
        if (col == ColFavorite || col == ColStatus)
            return static_cast<int>(Qt::AlignCenter);
    }

    if (role == Qt::ToolTipRole && col == ColRisk && !d.risks.isEmpty()) {
        QStringList lines;
        for (const RiskFinding& f : d.risks)
            lines << QStringLiteral("• [%1] %2").arg(toString(f.level), f.title);
        return lines.join(QLatin1Char('\n'));
    }

    if (role == Qt::BackgroundRole) {
        const auto it = highlightAt_.constFind(d.ipString());
        if (it != highlightAt_.cend()) {
            const qint64 age = clock_.elapsed() - it.value();
            if (age < kHighlightMs) {
                QColor c = accent_;
                c.setAlphaF(0.35 * (1.0 - static_cast<double>(age) / kHighlightMs));
                return c;
            }
        }
    }
    return {};
}

QVariant DeviceTableModel::headerData(int section, Qt::Orientation o, int role) const {
    if (role != Qt::DisplayRole || o != Qt::Horizontal) return {};
    switch (section) {
        case ColFavorite: return QStringLiteral("★");
        case ColStatus:   return tr("Status");
        case ColIp:       return tr("IP");
        case ColMac:      return tr("MAC");
        case ColHostname: return tr("Hostname");
        case ColVendor:   return tr("Vendor");
        case ColOs:       return tr("OS");
        case ColType:     return tr("Type");
        case ColPorts:    return tr("Open Ports");
        case ColServices: return tr("Services");
        case ColRisk:     return tr("Risk");
    }
    return {};
}

void DeviceTableModel::upsert(const Device& device) {
    const QString ip = device.ipString();
    const auto it = rowByIp_.constFind(ip);
    if (it != rowByIp_.cend()) {
        const int row = it.value();
        // Preserve favourite/labels set locally if the incoming record lacks them.
        Device merged = device;
        if (merged.labels.isEmpty()) merged.labels = devices_[row].labels;
        merged.favorite = merged.favorite || devices_[row].favorite;
        devices_[row] = merged;
        emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
        return;
    }
    const int row = devices_.size();
    beginInsertRows({}, row, row);
    devices_.push_back(device);
    rowByIp_.insert(ip, row);
    endInsertRows();

    highlightAt_.insert(ip, clock_.elapsed());
    if (!highlightTimer_.isActive()) highlightTimer_.start();
}

void DeviceTableModel::setDevices(const QList<Device>& devices) {
    beginResetModel();
    devices_ = devices;
    rowByIp_.clear();
    for (int i = 0; i < devices_.size(); ++i) rowByIp_.insert(devices_[i].ipString(), i);
    highlightAt_.clear();
    endResetModel();
}

void DeviceTableModel::clear() {
    beginResetModel();
    devices_.clear();
    rowByIp_.clear();
    highlightAt_.clear();
    endResetModel();
}

Device DeviceTableModel::deviceAt(int row) const {
    return (row >= 0 && row < devices_.size()) ? devices_[row] : Device{};
}

int DeviceTableModel::rowForIp(const QString& ip) const { return rowByIp_.value(ip, -1); }

void DeviceTableModel::setAccent(const QColor& accent) { accent_ = accent; }

void DeviceTableModel::toggleFavorite(int row) {
    if (row < 0 || row >= devices_.size()) return;
    devices_[row].favorite = !devices_[row].favorite;
    emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
}

}  // namespace pacn
