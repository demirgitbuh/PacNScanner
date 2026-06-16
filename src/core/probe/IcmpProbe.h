#pragma once

#include "core/probe/Probe.h"

namespace pacn {

// ICMP echo liveness via the system `ping` utility. This needs no elevated
// privileges and works uniformly across Windows/macOS/Linux. When raw sockets
// are available a future RawScanner can replace it; the engine falls back to
// this transparently.
class IcmpHostProbe : public IHostProbe {
public:
    HostProbeResult probe(const QHostAddress& host, const ScanConfig& cfg) override;
};

}  // namespace pacn
