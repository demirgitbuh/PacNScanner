#include "core/probe/DefaultProbeFactory.h"

#include "core/probe/IcmpProbe.h"
#include "core/probe/TcpProbe.h"

namespace pacn {

HostProbeResult DefaultHostProbe::probe(const QHostAddress& host, const ScanConfig& cfg) {
    if (cfg.methods.testFlag(ScanMethod::Icmp)) {
        IcmpHostProbe icmp;
        HostProbeResult r = icmp.probe(host, cfg);
        if (r.alive) return r;
    }
    if (cfg.methods.testFlag(ScanMethod::TcpConnect)) {
        TcpHostProbe tcp;
        HostProbeResult r = tcp.probe(host, cfg);
        if (r.alive) return r;
    }
    return {};  // not alive via active methods (ARP table still consulted later)
}

std::shared_ptr<IHostProbe> DefaultProbeFactory::hostProbe() {
    if (!host_) host_ = std::make_shared<DefaultHostProbe>();
    return host_;
}

std::shared_ptr<IPortProbe> DefaultProbeFactory::portProbe() {
    if (!port_) port_ = std::make_shared<TcpPortProbe>();
    return port_;
}

}  // namespace pacn
