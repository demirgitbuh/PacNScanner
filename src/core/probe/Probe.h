#pragma once

#include <QHostAddress>
#include <QString>
#include <memory>

#include "core/ScanConfig.h"

namespace pacn {

// Result of a host liveness probe.
struct HostProbeResult {
    bool alive = false;
    int rttMs = -1;
    int ttl = -1;
    QString method;  // "icmp" | "tcp" | "arp"
};

// Result of a single TCP port probe.
struct PortProbeResult {
    bool open = false;
    QString banner;
};

// Determines whether a host is up. Implementations MUST be reentrant (callable
// from many worker threads at once) — they create their own sockets/processes.
class IHostProbe {
public:
    virtual ~IHostProbe() = default;
    virtual HostProbeResult probe(const QHostAddress& host, const ScanConfig& cfg) = 0;
};

// Probes a single TCP port, optionally grabbing a service banner.
class IPortProbe {
public:
    virtual ~IPortProbe() = default;
    virtual PortProbeResult probe(const QHostAddress& host, quint16 port, int timeoutMs,
                                  bool grabBanner) = 0;
};

// Supplies the probes used by the engine. The default factory uses real network
// I/O; tests inject a fake one for deterministic, offline runs.
class IProbeFactory {
public:
    virtual ~IProbeFactory() = default;
    virtual std::shared_ptr<IHostProbe> hostProbe() = 0;
    virtual std::shared_ptr<IPortProbe> portProbe() = 0;
};

}  // namespace pacn
