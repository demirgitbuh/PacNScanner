#pragma once

#include <QHostAddress>
#include <QList>
#include <QString>
#include <memory>

namespace pacn {

// Description of a local network adapter (incl. VPN / virtual / Wi-Fi info).
struct AdapterInfo {
    QString name;
    QString description;
    QString macAddress;
    QList<QHostAddress> ipv4;
    QList<QHostAddress> ipv6;
    QHostAddress primaryIpv4;
    int prefixLength = 24;
    bool isUp = false;
    bool isLoopback = false;
    bool isVirtual = false;
    bool isWireless = false;
    bool isVpn = false;
    QString wifiSsid;
    int wifiSignal = -1;  // 0..100 percentage, -1 = unknown
};

struct ArpEntry {
    QHostAddress ip;
    QString mac;
    QString adapter;
};

struct GatewayInfo {
    QHostAddress address;
    QString adapter;
    QString mac;
};

// Port (interface) for OS-specific networking facts. Implemented per-platform
// in src/platform; tests inject a FakePlatform. Keeping the interface in core
// preserves layering (core defines the port, platform provides the adapter).
class IPlatformServices {
public:
    virtual ~IPlatformServices() = default;

    virtual QList<AdapterInfo> enumerateAdapters() = 0;
    virtual QList<ArpEntry> arpTable() = 0;
    virtual QList<GatewayInfo> defaultGateways() = 0;

    virtual bool isElevated() = 0;
    virtual bool rawSocketsAvailable() = 0;

    virtual QString netbiosName(const QHostAddress&) { return {}; }
    virtual QString mdnsName(const QHostAddress&) { return {}; }
};

// Implemented in src/platform — returns the concrete OS implementation.
std::shared_ptr<IPlatformServices> createPlatformServices();

}  // namespace pacn
