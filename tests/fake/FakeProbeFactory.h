#pragma once

#include <QHash>
#include <QSet>

#include "core/probe/Probe.h"

namespace pacn::test {

// Scripted, offline probes for deterministic engine tests.
class FakeHostProbe : public IHostProbe {
public:
    QSet<QString> alive;
    HostProbeResult probe(const QHostAddress& host, const ScanConfig&) override {
        HostProbeResult r;
        r.method = QStringLiteral("fake");
        if (alive.contains(host.toString())) {
            r.alive = true;
            r.rttMs = 1;
            r.ttl = 64;
        }
        return r;
    }
};

class FakePortProbe : public IPortProbe {
public:
    QHash<QString, QSet<quint16>> open;
    QHash<quint16, QString> banners;
    PortProbeResult probe(const QHostAddress& host, quint16 port, int, bool grab) override {
        PortProbeResult r;
        if (open.value(host.toString()).contains(port)) {
            r.open = true;
            if (grab) r.banner = banners.value(port);
        }
        return r;
    }
};

class FakeProbeFactory : public IProbeFactory {
public:
    std::shared_ptr<FakeHostProbe> host = std::make_shared<FakeHostProbe>();
    std::shared_ptr<FakePortProbe> port = std::make_shared<FakePortProbe>();
    std::shared_ptr<IHostProbe> hostProbe() override { return host; }
    std::shared_ptr<IPortProbe> portProbe() override { return port; }
};

}  // namespace pacn::test
