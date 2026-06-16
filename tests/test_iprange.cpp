#include <gtest/gtest.h>

#include "core/IpRange.h"

using namespace pacn;

TEST(IpRange, ParsesIpv4Cidr) {
    auto r = IpRange::parse(QStringLiteral("192.168.1.0/24"));
    ASSERT_TRUE(static_cast<bool>(r));
    const IpRange range = r.value();
    EXPECT_EQ(range.hostCount(), 254u);
    EXPECT_EQ(range.first().toString().toStdString(), "192.168.1.1");
    EXPECT_EQ(range.last().toString().toStdString(), "192.168.1.254");
    EXPECT_EQ(range.expand().size(), 254);
}

TEST(IpRange, ExcludesNetworkAndBroadcast) {
    const auto range = IpRange::parse(QStringLiteral("10.0.0.0/30")).value();
    // /30 => hosts .1 and .2 (network .0 and broadcast .3 excluded).
    EXPECT_EQ(range.hostCount(), 2u);
    const auto hosts = range.expand();
    ASSERT_EQ(hosts.size(), 2);
    EXPECT_EQ(hosts.first().toString().toStdString(), "10.0.0.1");
    EXPECT_EQ(hosts.last().toString().toStdString(), "10.0.0.2");
}

TEST(IpRange, SlashThirtyOneAndThirtyTwo) {
    EXPECT_EQ(IpRange::parse(QStringLiteral("10.0.0.0/31")).value().hostCount(), 2u);
    EXPECT_EQ(IpRange::parse(QStringLiteral("10.0.0.5/32")).value().hostCount(), 1u);
}

TEST(IpRange, ParsesRangeAndShortForm) {
    EXPECT_EQ(IpRange::parse(QStringLiteral("10.0.0.1-10.0.0.10")).value().hostCount(), 10u);
    const auto shortForm = IpRange::parse(QStringLiteral("192.168.1.10-20")).value();
    EXPECT_EQ(shortForm.first().toString().toStdString(), "192.168.1.10");
    EXPECT_EQ(shortForm.last().toString().toStdString(), "192.168.1.20");
    EXPECT_EQ(shortForm.hostCount(), 11u);
}

TEST(IpRange, ParsesSingleHost) {
    const auto r = IpRange::parse(QStringLiteral("8.8.8.8")).value();
    EXPECT_EQ(r.hostCount(), 1u);
    EXPECT_EQ(r.expand().size(), 1);
}

TEST(IpRange, RejectsInvalid) {
    EXPECT_FALSE(static_cast<bool>(IpRange::parse(QStringLiteral("not-an-ip"))));
    EXPECT_FALSE(static_cast<bool>(IpRange::parse(QStringLiteral("10.0.0.10-10.0.0.1"))));
}

TEST(IpRange, ParsesIpv6Prefix) {
    const auto r = IpRange::parse(QStringLiteral("fe80::/120"));
    ASSERT_TRUE(static_cast<bool>(r));
    EXPECT_TRUE(r.value().isIpv6());
    EXPECT_EQ(r.value().hostCount(), 256u);
    EXPECT_LE(r.value().expand().size(), 256);
}

TEST(IpRange, ParsesMultipleTargets) {
    QStringList errors;
    const auto list = IpRange::parseList(
        QStringLiteral("192.168.1.0/24, 10.0.0.1-10.0.0.5 bogus"), &errors);
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(errors.size(), 1);
}

TEST(IpRange, EstimateScalesWithConcurrency) {
    const auto fast = IpRange::estimate(1000, 100, 500);
    const auto slow = IpRange::estimate(1000, 10, 500);
    EXPECT_LT(fast.seconds, slow.seconds);
    EXPECT_EQ(IpRange::estimate(100, 50, 100).load.toStdString(), "Low");
    EXPECT_EQ(IpRange::estimate(200000, 50, 100).load.toStdString(), "High");
}
