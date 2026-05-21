#pragma once

#include <string>
#include <vector>

#include "domain/UnitRegistry.hpp"

namespace domain {

struct ConversionEntry {
    std::string unit;
    double value;
};

constexpr double kDomainEpsilon = 1.0e-9;
constexpr double kDefaultFeetPerMeter = 3.28084;
constexpr double kDefaultYardPerMeter = 1.09361;

class Converter {
public:
    explicit Converter(UnitRegistry registry);

    double convert(const std::string& fromUnit, double value, const std::string& toUnit) const;
    std::vector<ConversionEntry> convertAll(const std::string& fromUnit, double value) const;

    const UnitRegistry& registry() const { return registry_; }

private:
    double toMeterHub(double value, const std::string& unit) const;
    double fromMeterHub(double meterHubValue, const std::string& unit) const;

    UnitRegistry registry_;
};

bool nearlyEqual(double a, double b, double epsilon = kDomainEpsilon);

}  // namespace domain
