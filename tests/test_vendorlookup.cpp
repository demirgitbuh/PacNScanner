#include <gtest/gtest.h>

#include "core/VendorLookup.h"

using namespace pacn;

TEST(VendorLookup, ResolvesKnownOui) {
    VendorLookup& v = VendorLookup::instance();
    v.ensureLoaded();
    // 00:0C:29 is the VMware OUI present in the bundled subset.
    EXPECT_TRUE(v.vendor(QStringLiteral("00:0C:29:11:22:33"))
                    .contains(QStringLiteral("VMware")));
}

TEST(VendorLookup, DetectsVirtualMac) {
    VendorLookup& v = VendorLookup::instance();
    v.ensureLoaded();
    EXPECT_TRUE(v.isVirtualMac(QStringLiteral("08:00:27:aa:bb:cc")));   // VirtualBox
    EXPECT_FALSE(v.isVirtualMac(QStringLiteral("B8:27:EB:11:22:33")));  // Raspberry Pi
}

TEST(VendorLookup, UnknownReturnsEmpty) {
    VendorLookup& v = VendorLookup::instance();
    v.ensureLoaded();
    EXPECT_TRUE(v.vendor(QStringLiteral("FE:DC:BA:98:76:54")).isEmpty());
}
