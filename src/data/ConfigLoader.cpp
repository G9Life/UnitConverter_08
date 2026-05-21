#include "data/ConfigLoader.hpp"

#include <cmath>
#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>

namespace data {

namespace {

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return {};
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool parseBaseUnit(const std::string& content, std::string& baseUnit) {
    static const std::regex pattern(R"re("base_unit"\s*:\s*"([a-zA-Z][a-zA-Z0-9_]*)")re");
    std::smatch match;
    if (!std::regex_search(content, match, pattern)) {
        return false;
    }
    baseUnit = match[1].str();
    return true;
}

bool parseYamlBaseUnit(const std::string& content, std::string& baseUnit) {
    static const std::regex pattern(R"(base_unit\s*:\s*([a-zA-Z][a-zA-Z0-9_]*))");
    std::smatch match;
    if (!std::regex_search(content, match, pattern)) {
        return false;
    }
    baseUnit = match[1].str();
    return true;
}

bool parseUnitsObject(const std::string& content, std::map<std::string, double>& units) {
    static const std::regex unitPattern(
        R"re("([a-zA-Z][a-zA-Z0-9_]*)"\s*:\s*([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?))re");
    const auto begin = std::sregex_iterator(content.begin(), content.end(), unitPattern);
    const auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        const std::string unit = (*it)[1].str();
        if (unit == "base_unit") {
            continue;
        }
        units[unit] = std::stod((*it)[2].str());
    }
    return !units.empty();
}

bool parseYamlUnits(const std::string& content, std::map<std::string, double>& units) {
    static const std::regex unitPattern(R"(([a-zA-Z][a-zA-Z0-9_]*)\s*:\s*([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?))");
    const auto begin = std::sregex_iterator(content.begin(), content.end(), unitPattern);
    const auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        const std::string unit = (*it)[1].str();
        if (unit == "base_unit" || unit == "units") {
            continue;
        }
        units[unit] = std::stod((*it)[2].str());
    }
    return !units.empty();
}

ConfigLoadResult failure(const std::string& code) {
    ConfigLoadResult result;
    result.success = false;
    result.errorCode = code;
    result.registry = domain::UnitRegistry::defaultRegistry();
    return result;
}

ConfigLoadResult applyUnits(const std::map<std::string, double>& units, const std::string& baseUnit) {
    if (baseUnit != "meter") {
        return failure("ERR-DATA-002");
    }

    domain::UnitRegistry registry;
    registry.clear();

    for (const auto& entry : units) {
        const std::string& unit = entry.first;
        const double raw = entry.second;
        if (raw <= 0.0 || !std::isfinite(raw)) {
            return failure("ERR-DATA-002");
        }

        if (unit == "meter") {
            if (raw != 1.0) {
                return failure("ERR-DATA-002");
            }
            registry.setFactor(unit, 1.0);
        } else {
            const double hubFactor = raw >= 1.0 ? raw : (1.0 / raw);
            registry.setFactor(unit, hubFactor);
        }
    }

    if (!registry.hasUnit("meter") || !registry.hasUnit("feet") || !registry.hasUnit("yard")) {
        return failure("ERR-DATA-002");
    }

    ConfigLoadResult result;
    result.success = true;
    result.registry = registry;
    return result;
}

}  // namespace

ConfigLoadResult ConfigLoader::loadJson(const std::string& path) {
    const std::string content = readFile(path);
    if (content.empty()) {
        return failure("ERR-DATA-001");
    }

    std::string baseUnit;
    if (!parseBaseUnit(content, baseUnit)) {
        return failure("ERR-DATA-002");
    }

    std::map<std::string, double> units;
    if (!parseUnitsObject(content, units)) {
        return failure("ERR-DATA-002");
    }

    return applyUnits(units, baseUnit);
}

ConfigLoadResult ConfigLoader::loadYaml(const std::string& path) {
    const std::string content = readFile(path);
    if (content.empty()) {
        return failure("ERR-DATA-001");
    }

    std::string baseUnit;
    if (!parseYamlBaseUnit(content, baseUnit)) {
        return failure("ERR-DATA-002");
    }

    std::map<std::string, double> units;
    if (!parseYamlUnits(content, units)) {
        return failure("ERR-DATA-002");
    }

    return applyUnits(units, baseUnit);
}

ConfigLoadResult ConfigLoader::load(const std::string& path, ConfigFormat format) {
    if (format == ConfigFormat::Yaml) {
        return loadYaml(path);
    }
    return loadJson(path);
}

}  // namespace data
