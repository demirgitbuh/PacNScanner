#pragma once

#include "core/Device.h"

namespace pacn {

// Lightweight, optional OS guessing from TTL values and service banners.
// Not a substitute for active fingerprinting, but useful and cheap.
class OsFingerprinter {
public:
    // Returns a human label ("Windows", "Linux/Unix", "Network device", ...)
    // or an empty string when undetermined.
    QString guess(const Device& device) const;

    static QString fromTtl(int ttl);
};

}  // namespace pacn
