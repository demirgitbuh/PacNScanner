#include <gtest/gtest.h>

#include "core/PortProfiles.h"

using namespace pacn;

TEST(PortProfiles, Top100ContainsCommonPorts) {
    const auto ports = portprofiles::top100();
    EXPECT_GE(ports.size(), 90);
    EXPECT_TRUE(ports.contains(80));
    EXPECT_TRUE(ports.contains(443));
    EXPECT_TRUE(ports.contains(22));
}

TEST(PortProfiles, FullCoversEntireRange) {
    EXPECT_EQ(portprofiles::full().size(), 65535);
    EXPECT_EQ(portprofiles::wellKnown().size(), 1024);
}

TEST(PortProfiles, ParseSpecSortsAndDedupes) {
    const auto ports = portprofiles::parseSpec(QStringLiteral("80, 22,22, 8000-8002"));
    ASSERT_EQ(ports.size(), 5);
    EXPECT_EQ(ports.first(), 22);
    EXPECT_TRUE(ports.contains(8000));
    EXPECT_TRUE(ports.contains(8002));
    EXPECT_FALSE(ports.contains(8003));
}

TEST(PortProfiles, ParseSpecClampsAndIgnoresGarbage) {
    const auto ports = portprofiles::parseSpec(QStringLiteral("0, 70000, abc, 443"));
    EXPECT_EQ(ports.size(), 1);
    EXPECT_EQ(ports.first(), 443);
}

TEST(PortProfiles, ForNameResolvesNamedProfiles) {
    EXPECT_FALSE(portprofiles::forName(QStringLiteral("Top 100")).isEmpty());
    EXPECT_TRUE(portprofiles::forName(QStringLiteral("Custom")).isEmpty());
}
