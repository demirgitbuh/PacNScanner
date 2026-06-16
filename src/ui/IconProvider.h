#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>

#include "core/Types.h"

namespace pacn {

// Centralises app/logo icons and status/risk colours so the table, badges and
// tray share one source of truth.
namespace iconprovider {

QIcon appIcon();
QPixmap logoPixmap(int size);

QColor statusColor(DeviceStatus status);
QColor riskColor(RiskLevel level);

QIcon statusIcon(DeviceStatus status);
QIcon dotIcon(const QColor& color);

QString deviceTypeGlyph(DeviceType type);  // short text glyph fallback

}  // namespace iconprovider
}  // namespace pacn
