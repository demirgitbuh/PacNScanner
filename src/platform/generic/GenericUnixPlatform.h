#pragma once

#include "core/PlatformServices.h"

namespace pacn {

// Portable IPlatformServices using QNetworkInterface plus parsing of common
// CLI tools (ip/arp/route). Used on macOS, Linux and any Unix; also a safe
// fallback elsewhere. OS-specific subclasses may override individual calls.
class GenericUnixPlatform : public IPlatformServices {
public:
    QList<AdapterInfo> enumerateAdapters() override;
    QList<ArpEntry> arpTable() override;
    QList<GatewayInfo> defaultGateways() override;
    bool isElevated() override;
    bool rawSocketsAvailable() override;

protected:
    // Best-effort SSID for a wireless adapter (empty if unavailable).
    static QString currentSsid();
};

}  // namespace pacn
