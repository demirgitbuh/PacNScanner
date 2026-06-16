#pragma once

#include "core/probe/Probe.h"

namespace pacn {

// Combined host probe: tries ICMP first (cheap, gives TTL) then TCP connect,
// honouring the methods enabled in ScanConfig. Falls back gracefully.
class DefaultHostProbe : public IHostProbe {
public:
    HostProbeResult probe(const QHostAddress& host, const ScanConfig& cfg) override;
};

// Real-network probe factory used by the GUI and CLI.
class DefaultProbeFactory : public IProbeFactory {
public:
    std::shared_ptr<IHostProbe> hostProbe() override;
    std::shared_ptr<IPortProbe> portProbe() override;

private:
    std::shared_ptr<IHostProbe> host_;
    std::shared_ptr<IPortProbe> port_;
};

}  // namespace pacn
