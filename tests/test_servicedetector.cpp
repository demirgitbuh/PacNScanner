#include <gtest/gtest.h>

#include "core/ServiceDetector.h"

using namespace pacn;

TEST(ServiceDetector, ResolvesCommonPorts) {
    ServiceDetector& d = ServiceDetector::instance();
    d.ensureLoaded();
    EXPECT_EQ(d.serviceName(22).toStdString(), "ssh");
    EXPECT_EQ(d.serviceName(80).toStdString(), "http");
    EXPECT_EQ(d.serviceName(443).toStdString(), "https");
}

TEST(ServiceDetector, FlagsInsecureServices) {
    ServiceDetector& d = ServiceDetector::instance();
    d.ensureLoaded();
    EXPECT_TRUE(d.hasFlag(23, QStringLiteral("plaintext")));
    EXPECT_TRUE(d.hasFlag(3306, QStringLiteral("db")));
    EXPECT_TRUE(d.hasFlag(445, QStringLiteral("share")));
}

TEST(ServiceDetector, RefinesFromBanner) {
    ServiceDetector& d = ServiceDetector::instance();
    d.ensureLoaded();
    const QString refined =
        d.refineFromBanner(22, QStringLiteral("SSH-2.0-OpenSSH_9.6"));
    EXPECT_TRUE(refined.startsWith(QStringLiteral("SSH-2.0")));
}
