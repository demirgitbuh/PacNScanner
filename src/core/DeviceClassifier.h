#pragma once

#include "core/Device.h"

namespace pacn {

// Infers a DeviceType from open ports, services, vendor and (optionally) whether
// the address is the gateway. Heuristic, deterministic, unit-testable.
class DeviceClassifier {
public:
    DeviceType classify(const Device& device, bool isGateway = false) const;
};

}  // namespace pacn
