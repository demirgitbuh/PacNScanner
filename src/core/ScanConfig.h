#pragma once

#include <QList>
#include <QString>

#include "core/IpRange.h"
#include "core/Types.h"

namespace pacn {

// Everything needed to run a scan. UI and CLI both build one of these and hand
// it to ScanEngine. Speed profile drives concurrency/timeouts unless overridden.
struct ScanConfig {
    QList<IpRange> targets;

    ScanMethods methods =
        ScanMethod::TcpConnect | ScanMethod::Icmp | ScanMethod::Arp;
    SpeedProfile speed = SpeedProfile::Normal;

    // Resolved TCP ports to scan on live hosts (empty => discovery ports only).
    QList<quint16> ports;
    QString portProfileName = QStringLiteral("Top 100");

    HostnameMethods hostnameMethods = HostnameMethod::Dns;
    bool resolveHostnames = true;
    bool detectOs = true;        // optional OS fingerprinting
    bool grabBanners = true;     // banner grabbing for service detection
    bool useOnlineVendor = false;  // off by default (privacy)
    bool adaptiveTimeout = true;

    // Overrides (0 = derive from speed profile).
    int concurrencyOverride = 0;
    int hostTimeoutOverride = 0;
    int portTimeoutOverride = 0;

    QString adapterName;  // empty => auto-select

    // --- Derived parameters --------------------------------------------------
    int concurrency() const;
    int hostTimeoutMs() const;
    int portTimeoutMs() const;

    // Small fixed port set used to decide liveness during TCP discovery.
    static QList<quint16> discoveryPorts();

    quint64 totalHostCount() const;
    IpRange::Estimate estimate() const;
    bool hasHugeTarget() const;
};

}  // namespace pacn
