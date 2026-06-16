#pragma once

#include "core/probe/Probe.h"

namespace pacn {

// Privilege-free liveness probe: a host is considered up if any discovery port
// either accepts the connection or actively refuses it (RST => host present).
class TcpHostProbe : public IHostProbe {
public:
    HostProbeResult probe(const QHostAddress& host, const ScanConfig& cfg) override;
};

// TCP connect port probe with optional banner grabbing.
class TcpPortProbe : public IPortProbe {
public:
    PortProbeResult probe(const QHostAddress& host, quint16 port, int timeoutMs,
                          bool grabBanner) override;
};

}  // namespace pacn
