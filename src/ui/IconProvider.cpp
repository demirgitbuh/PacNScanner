#include "ui/IconProvider.h"

#include <QPainter>
#include <QSvgRenderer>

namespace pacn::iconprovider {

QPixmap logoPixmap(int size) {
    QSvgRenderer renderer(QStringLiteral(":/logo.svg"));
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    renderer.render(&p);
    return pm;
}

QIcon appIcon() {
    QIcon icon;
    for (int s : {16, 24, 32, 48, 64, 128, 256}) icon.addPixmap(logoPixmap(s));
    return icon;
}

QColor statusColor(DeviceStatus status) {
    switch (status) {
        case DeviceStatus::Online:  return QColor(0x27, 0xae, 0x60);
        case DeviceStatus::New:     return QColor(0x2b, 0xbf, 0xd6);
        case DeviceStatus::Lost:    return QColor(0xe6, 0x7e, 0x22);
        case DeviceStatus::Offline: return QColor(0x95, 0xa5, 0xa6);
        case DeviceStatus::Unknown: return QColor(0xbd, 0xc3, 0xc7);
    }
    return QColor(0xbd, 0xc3, 0xc7);
}

QColor riskColor(RiskLevel level) {
    switch (level) {
        case RiskLevel::Critical: return QColor(0xc0, 0x39, 0x2b);
        case RiskLevel::High:     return QColor(0xe6, 0x7e, 0x22);
        case RiskLevel::Medium:   return QColor(0xf1, 0xc4, 0x0f);
        case RiskLevel::Low:      return QColor(0x27, 0xae, 0x60);
        case RiskLevel::Info:     return QColor(0x7f, 0x8c, 0x8d);
    }
    return QColor(0x7f, 0x8c, 0x8d);
}

QIcon dotIcon(const QColor& color) {
    QPixmap pm(16, 16);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawEllipse(3, 3, 10, 10);
    return QIcon(pm);
}

QIcon statusIcon(DeviceStatus status) { return dotIcon(statusColor(status)); }

QString deviceTypeGlyph(DeviceType type) {
    switch (type) {
        case DeviceType::Gateway:
        case DeviceType::Router:         return QStringLiteral("⟚");
        case DeviceType::Switch:         return QStringLiteral("▦");
        case DeviceType::AccessPoint:    return QStringLiteral("≋");
        case DeviceType::Server:         return QStringLiteral("▤");
        case DeviceType::Computer:       return QStringLiteral("▣");
        case DeviceType::Laptop:         return QStringLiteral("▢");
        case DeviceType::Phone:          return QStringLiteral("▯");
        case DeviceType::Tablet:         return QStringLiteral("▭");
        case DeviceType::Printer:        return QStringLiteral("⎙");
        case DeviceType::Nas:            return QStringLiteral("▥");
        case DeviceType::Camera:         return QStringLiteral("◉");
        case DeviceType::MediaPlayer:    return QStringLiteral("►");
        case DeviceType::IoT:            return QStringLiteral("◈");
        case DeviceType::VirtualMachine: return QStringLiteral("◫");
        case DeviceType::Unknown:        return QStringLiteral("•");
    }
    return QStringLiteral("•");
}

}  // namespace pacn::iconprovider
