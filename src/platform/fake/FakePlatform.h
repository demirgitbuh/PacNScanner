#pragma once

#include <QHash>

#include "core/PlatformServices.h"

namespace pacn {

// Deterministic, in-memory IPlatformServices for tests and offline runs. Tests
// populate the public fields, then inject this into the engine.
class FakePlatform : public IPlatformServices {
public:
    QList<AdapterInfo> adapters;
    QList<ArpEntry> arp;
    QList<GatewayInfo> gateways;
    bool elevated = false;
    bool raw = false;
    QHash<QString, QString> netbiosByIp;
    QHash<QString, QString> mdnsByIp;

    QList<AdapterInfo> enumerateAdapters() override { return adapters; }
    QList<ArpEntry> arpTable() override { return arp; }
    QList<GatewayInfo> defaultGateways() override { return gateways; }
    bool isElevated() override { return elevated; }
    bool rawSocketsAvailable() override { return raw; }
    QString netbiosName(const QHostAddress& ip) override {
        return netbiosByIp.value(ip.toString());
    }
    QString mdnsName(const QHostAddress& ip) override {
        return mdnsByIp.value(ip.toString());
    }
};

}  // namespace pacn
