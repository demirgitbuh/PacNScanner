#include <gtest/gtest.h>

#include "core/NetUtils.h"

using namespace pacn;

TEST(NetUtils, NormalizesMacVariants) {
    EXPECT_EQ(netutils::normalizeMac(QStringLiteral("aa-bb-cc-dd-ee-ff")).toStdString(),
              "AA:BB:CC:DD:EE:FF");
    EXPECT_EQ(netutils::normalizeMac(QStringLiteral("aabb.ccdd.eeff")).toStdString(),
              "AA:BB:CC:DD:EE:FF");
    EXPECT_EQ(netutils::normalizeMac(QStringLiteral("AA BB CC DD EE FF")).toStdString(),
              "AA:BB:CC:DD:EE:FF");
    EXPECT_TRUE(netutils::normalizeMac(QStringLiteral("zz")).isEmpty());
}

TEST(NetUtils, OuiKey) {
    EXPECT_EQ(netutils::ouiKey(QStringLiteral("00:0c:29:ab:cd:ef")).toStdString(), "000C29");
    EXPECT_EQ(netutils::ouiKey(QStringLiteral("000c29")).toStdString(), "000C29");
}

TEST(NetUtils, CompareIpOrdersV4BeforeV6AndNumerically) {
    const QHostAddress a(QStringLiteral("192.168.1.2"));
    const QHostAddress b(QStringLiteral("192.168.1.10"));
    const QHostAddress v6(QStringLiteral("fe80::1"));
    EXPECT_LT(netutils::compareIp(a, b), 0);
    EXPECT_GT(netutils::compareIp(b, a), 0);
    EXPECT_LT(netutils::compareIp(a, v6), 0);   // IPv4 sorts first
    EXPECT_EQ(netutils::compareIp(a, a), 0);
}

TEST(NetUtils, NextAddress) {
    EXPECT_EQ(netutils::nextAddress(QHostAddress(QStringLiteral("10.0.0.1")))
                  .toString()
                  .toStdString(),
              "10.0.0.2");
    EXPECT_EQ(netutils::nextAddress(QHostAddress(QStringLiteral("10.0.0.255")))
                  .toString()
                  .toStdString(),
              "10.0.1.0");
}

TEST(NetUtils, IsPrivate) {
    EXPECT_TRUE(netutils::isPrivate(QHostAddress(QStringLiteral("192.168.0.1"))));
    EXPECT_TRUE(netutils::isPrivate(QHostAddress(QStringLiteral("10.1.2.3"))));
    EXPECT_TRUE(netutils::isPrivate(QHostAddress(QStringLiteral("172.16.5.4"))));
    EXPECT_FALSE(netutils::isPrivate(QHostAddress(QStringLiteral("8.8.8.8"))));
}
