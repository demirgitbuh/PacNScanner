#pragma once

#include "core/Device.h"

namespace pacn {

// Rule-based risk scoring. Inspects open ports/services and produces a 0..100
// score, a RiskLevel and a list of human-readable findings. Deterministic and
// fully unit-testable.
class RiskEngine {
public:
    // Mutates device.riskScore / riskLevel / risks in place.
    void evaluate(Device& device) const;

    static RiskLevel levelForScore(int score);
};

}  // namespace pacn
