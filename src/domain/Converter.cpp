#include "domain/Converter.hpp"

#include <cmath>
#include <stdexcept>

namespace domain {

Converter::Converter(UnitRegistry registry) : registry_(std::move(registry)) {}

double Converter::toMeterHub(double value, const std::string& unit) const {
    const double factor = registry_.factorFor(unit);
    return value / factor;
}

double Converter::fromMeterHub(double meterHubValue, const std::string& unit) const {
    const double factor = registry_.factorFor(unit);
    return meterHubValue * factor;
}

double Converter::convert(const std::string& fromUnit, double value, const std::string& toUnit) const {
    if (value <= 0.0 || !std::isfinite(value)) {
        throw std::invalid_argument("Value must be positive");
    }
    const double meterHub = toMeterHub(value, fromUnit);
    return fromMeterHub(meterHub, toUnit);
}

std::vector<ConversionEntry> Converter::convertAll(const std::string& fromUnit, double value) const {
    const double meterHub = toMeterHub(value, fromUnit);
    std::vector<ConversionEntry> results;
    for (const auto& entry : registry_.allUnits()) {
        results.push_back({entry.first, fromMeterHub(meterHub, entry.first)});
    }
    return results;
}

bool nearlyEqual(double a, double b, double epsilon) {
    return std::fabs(a - b) <= epsilon;
}

}  // namespace domain
