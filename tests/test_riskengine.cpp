#include <gtest/gtest.h>

#include "core/RiskEngine.h"

using namespace pacn;

namespace {
Device deviceWithPorts(std::initializer_list<quint16> ports) {
    Device d;
    d.ip = QHostAddress(QStringLiteral("192.168.1.5"));
    d.status = DeviceStatus::Online;
    for (quint16 p : ports) {
        Port port;
        port.number = p;
        port.state = PortState::Open;
        d.ports.push_back(port);
    }
    return d;
}
}  // namespace

TEST(RiskEngine, NoPortsIsInfo) {
    Device d = deviceWithPorts({});
    RiskEngine().evaluate(d);
    EXPECT_EQ(d.riskScore, 0);
    EXPECT_EQ(d.riskLevel, RiskLevel::Info);
}

TEST(RiskEngine, TelnetIsHighRisk) {
    Device d = deviceWithPorts({23});
    RiskEngine().evaluate(d);
    EXPECT_GT(d.riskScore, 0);
    EXPECT_GE(d.riskLevel, RiskLevel::Medium);
    bool hasTelnet = false;
    for (const RiskFinding& f : d.risks)
        if (f.title.contains(QStringLiteral("Telnet"))) hasTelnet = true;
    EXPECT_TRUE(hasTelnet);
}

TEST(RiskEngine, DatabaseExposureFlagged) {
    Device d = deviceWithPorts({6379});  // redis, no auth
    RiskEngine().evaluate(d);
    EXPECT_GE(d.riskLevel, RiskLevel::Medium);
}

TEST(RiskEngine, ScoreIsClampedAndMonotonic) {
    Device few = deviceWithPorts({80});
    Device many = deviceWithPorts({21, 23, 3389, 5900, 445, 161, 6379, 80, 443, 22, 25});
    RiskEngine e;
    e.evaluate(few);
    e.evaluate(many);
    EXPECT_LE(many.riskScore, 100);
    EXPECT_GT(many.riskScore, few.riskScore);
}

TEST(RiskEngine, LevelForScoreThresholds) {
    EXPECT_EQ(RiskEngine::levelForScore(0), RiskLevel::Info);
    EXPECT_EQ(RiskEngine::levelForScore(15), RiskLevel::Low);
    EXPECT_EQ(RiskEngine::levelForScore(40), RiskLevel::Medium);
    EXPECT_EQ(RiskEngine::levelForScore(60), RiskLevel::High);
    EXPECT_EQ(RiskEngine::levelForScore(90), RiskLevel::Critical);
}
