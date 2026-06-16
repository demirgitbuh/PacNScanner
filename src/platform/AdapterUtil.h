#pragma once

#include <QList>

#include "core/PlatformServices.h"

namespace pacn::adapterutil {

// Enumerates local adapters using QNetworkInterface and applies portable
// heuristics for virtual / VPN / wireless classification. Wi-Fi SSID/signal are
// left for the OS-specific layer to fill in.
QList<AdapterInfo> enumerateViaQt();

bool looksVirtual(const QString& name, const QString& humanName, const QString& mac);
bool looksVpn(const QString& name, const QString& humanName);
bool looksWireless(const QString& name, const QString& humanName);

}  // namespace pacn::adapterutil
