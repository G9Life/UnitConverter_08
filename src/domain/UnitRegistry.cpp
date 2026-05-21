#include "domain/UnitRegistry.hpp"

#include <cmath>
#include <stdexcept>

namespace domain {

namespace {
constexpr double kMeterFactor = 1.0;
constexpr double kFeetPerMeter = 3.28084;
constexpr double kYardPerMeter = 1.09361;
}  // namespace

UnitRegistry UnitRegistry::defaultRegistry() {
    UnitRegistry registry;
    registry.loadDefaults();
    return registry;
}

bool UnitRegistry::hasUnit(const std::string& unit) const {
    return factors_.find(unit) != factors_.end();
}

double UnitRegistry::factorFor(const std::string& unit) const {
    const auto it = factors_.find(unit);
    if (it == factors_.end()) {
        throw std::invalid_argument("Unknown unit: " + unit);
    }
    return it->second;
}

std::size_t UnitRegistry::unitCount() const {
    return factors_.size();
}

std::map<std::string, double> UnitRegistry::allUnits() const {
    return factors_;
}

void UnitRegistry::registerUnit(const std::string& unit, double metersPerUnit) {
    if (unit.empty()) {
        throw std::invalid_argument("Invalid unit name: " + unit);
    }
    if (metersPerUnit <= 0.0 || !std::isfinite(metersPerUnit)) {
        throw std::invalid_argument("Invalid ratio for unit: " + unit);
    }
    if (hasUnit(unit)) {
        throw std::invalid_argument("Unit already registered: " + unit);
    }
    setFactor(unit, 1.0 / metersPerUnit);
}

void UnitRegistry::setFactor(const std::string& unit, double hubFactor) {
    if (hubFactor <= 0.0 || !std::isfinite(hubFactor)) {
        throw std::invalid_argument("Invalid ratio for unit: " + unit);
    }
    factors_[unit] = hubFactor;
}

void UnitRegistry::clear() {
    factors_.clear();
}

void UnitRegistry::loadDefaults() {
    factors_.clear();
    factors_["meter"] = kMeterFactor;
    factors_["feet"] = kFeetPerMeter;
    factors_["yard"] = kYardPerMeter;
}

}  // namespace domain
