#include <gtest/gtest.h>

#include "core/DeviceClassifier.h"

using namespace pacn;

namespace {
Device make(const QString& vendor, std::initializer_list<quint16> ports) {
    Device d;
    d.ip = QHostAddress(QStringLiteral("192.168.1.9"));
    d.vendor = vendor;
    for (quint16 p : ports) {
        Port port;
        port.number = p;
        port.state = PortState::Open;
        port.service = QString();
        d.ports.push_back(port);
    }
    return d;
}
}  // namespace

TEST(DeviceClassifier, GatewayFlagWins) {
    Device d = make(QStringLiteral("Cisco"), {80});
    EXPECT_EQ(DeviceClassifier().classify(d, /*isGateway=*/true), DeviceType::Gateway);
}

TEST(DeviceClassifier, VirtualVendor) {
    Device d = make(QStringLiteral("VMware, Inc."), {22});
    EXPECT_EQ(DeviceClassifier().classify(d), DeviceType::VirtualMachine);
}

TEST(DeviceClassifier, PrinterByPort) {
    Device d = make(QString(), {9100});
    EXPECT_EQ(DeviceClassifier().classify(d), DeviceType::Printer);
}

TEST(DeviceClassifier, RouterVendor) {
    Device d = make(QStringLiteral("MikroTik"), {80});
    EXPECT_EQ(DeviceClassifier().classify(d), DeviceType::Router);
}

TEST(DeviceClassifier, FallsBackToComputerOrServer) {
    EXPECT_EQ(DeviceClassifier().classify(make(QString(), {445})), DeviceType::Nas);
    EXPECT_EQ(DeviceClassifier().classify(make(QString(), {3000})), DeviceType::Computer);
}
