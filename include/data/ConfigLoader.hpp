#pragma once

#include <string>

#include "domain/UnitRegistry.hpp"

namespace data {

enum class ConfigFormat { Json, Yaml };

struct ConfigLoadResult {
    bool success;
    std::string errorCode;
    domain::UnitRegistry registry;
};

class ConfigLoader {
public:
    static ConfigLoadResult load(const std::string& path, ConfigFormat format = ConfigFormat::Json);
    static ConfigLoadResult loadJson(const std::string& path);
    static ConfigLoadResult loadYaml(const std::string& path);
};

}  // namespace data
