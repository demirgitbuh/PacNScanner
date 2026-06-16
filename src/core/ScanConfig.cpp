#include "core/ScanConfig.h"

namespace pacn {

int ScanConfig::concurrency() const {
    if (concurrencyOverride > 0) return concurrencyOverride;
    switch (speed) {
        case SpeedProfile::Slow:   return 32;
        case SpeedProfile::Normal: return 128;
        case SpeedProfile::Fast:   return 512;
    }
    return 128;
}

int ScanConfig::hostTimeoutMs() const {
    if (hostTimeoutOverride > 0) return hostTimeoutOverride;
    switch (speed) {
        case SpeedProfile::Slow:   return 1500;
        case SpeedProfile::Normal: return 800;
        case SpeedProfile::Fast:   return 400;
    }
    return 800;
}

int ScanConfig::portTimeoutMs() const {
    if (portTimeoutOverride > 0) return portTimeoutOverride;
    switch (speed) {
        case SpeedProfile::Slow:   return 1200;
        case SpeedProfile::Normal: return 600;
        case SpeedProfile::Fast:   return 300;
    }
    return 600;
}

QList<quint16> ScanConfig::discoveryPorts() {
    // Ports most likely to be open on *something* — used only to infer liveness
    // when ICMP is unavailable/blocked.
    return {80, 443, 22, 445, 139, 53, 135, 3389, 8080, 23, 21, 5357, 62078};
}

quint64 ScanConfig::totalHostCount() const {
    quint64 total = 0;
    for (const IpRange& r : targets) {
        const quint64 c = r.hostCount();
        if (c == IpRange::kHugeCount) return IpRange::kHugeCount;
        total += c;
    }
    return total;
}

bool ScanConfig::hasHugeTarget() const {
    for (const IpRange& r : targets)
        if (r.hostCount() == IpRange::kHugeCount) return true;
    return false;
}

IpRange::Estimate ScanConfig::estimate() const {
    const quint64 hosts = totalHostCount();
    // Per-host cost: discovery + (optional) port scan, dominated by timeouts on
    // unreachable hosts.
    const int portCount = ports.isEmpty() ? discoveryPorts().size() : ports.size();
    const int perHostMs = hostTimeoutMs() + (portCount * portTimeoutMs()) / 8;
    return IpRange::estimate(hosts, concurrency(), perHostMs);
}

}  // namespace pacn
