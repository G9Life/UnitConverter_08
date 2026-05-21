// RED phase linkage only — not GREEN implementation.
// CMake: UNIT_CONVERTER_RED_PHASE=ON → this file replaces src/*.cpp for unit_converter_core.
// GREEN: set UNIT_CONVERTER_RED_PHASE=OFF to link production sources.

#include "boundary/InputParser.hpp"
#include "data/ConfigLoader.hpp"
#include "domain/Converter.hpp"
#include "domain/UnitRegistry.hpp"

#include <cmath>
#include <stdexcept>

namespace domain {

UnitRegistry UnitRegistry::defaultRegistry() {
    return UnitRegistry{};
}

bool UnitRegistry::hasUnit(const std::string& /*unit*/) const {
    return false;
}

double UnitRegistry::factorFor(const std::string& unit) const {
    throw std::invalid_argument("Unknown unit: " + unit);
}

std::size_t UnitRegistry::unitCount() const {
    return 0;
}

std::map<std::string, double> UnitRegistry::allUnits() const {
    return {};
}

void UnitRegistry::registerUnit(const std::string& /*unit*/, double /*metersPerUnit*/) {}

void UnitRegistry::setFactor(const std::string& /*unit*/, double /*hubFactor*/) {}

void UnitRegistry::clear() {}

void UnitRegistry::loadDefaults() {}

Converter::Converter(UnitRegistry registry) : registry_(std::move(registry)) {}

double Converter::toMeterHub(double /*value*/, const std::string& /*unit*/) const {
    return 0.0;
}

double Converter::fromMeterHub(double /*meterHubValue*/, const std::string& /*unit*/) const {
    return 0.0;
}

double Converter::convert(const std::string& /*fromUnit*/, double /*value*/, const std::string& /*toUnit*/) const {
    return 0.0;
}

std::vector<ConversionEntry> Converter::convertAll(const std::string& /*fromUnit*/, double /*value*/) const {
    return {};
}

bool nearlyEqual(double /*a*/, double /*b*/, double /*epsilon*/) {
    return false;
}

}  // namespace domain

namespace boundary {

ParsedInput InputParser::parse(const std::string& /*line*/) {
    return {"", 0.0};
}

}  // namespace boundary

namespace data {

ConfigLoadResult ConfigLoader::load(const std::string& /*path*/, ConfigFormat /*format*/) {
    return {false, "ERR-DATA-001", domain::UnitRegistry{}};
}

ConfigLoadResult ConfigLoader::loadJson(const std::string& /*path*/) {
    return {false, "ERR-DATA-001", domain::UnitRegistry{}};
}

ConfigLoadResult ConfigLoader::loadYaml(const std::string& /*path*/) {
    return {false, "ERR-DATA-001", domain::UnitRegistry{}};
}

}  // namespace data
