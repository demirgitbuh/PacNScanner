#pragma once

#include "core/PlatformServices.h"

namespace pacn {

// Windows IPlatformServices using the IP Helper API (iphlpapi) for the ARP
// table and default gateways, QNetworkInterface for adapters, and `netsh` for
// Wi-Fi SSID. Raw/ARP-active scanning is enabled separately via optional Npcap.
class WindowsPlatform : public IPlatformServices {
public:
    QList<AdapterInfo> enumerateAdapters() override;
    QList<ArpEntry> arpTable() override;
    QList<GatewayInfo> defaultGateways() override;
    bool isElevated() override;
    bool rawSocketsAvailable() override;
};

}  // namespace pacn
