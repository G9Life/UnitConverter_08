#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include "data/ConfigLoader.hpp"
#include "domain/Converter.hpp"
#include "domain/UnitRegistry.hpp"

using namespace data;
using namespace domain;

namespace {

std::filesystem::path fixturePath(const std::string& name) {
#ifdef TEST_FIXTURE_DIR
    return std::filesystem::path(TEST_FIXTURE_DIR) / name;
#else
    return std::filesystem::path("tests/fixtures") / name;
#endif
}

}  // namespace

// =============================================================================
// JSON 설정 로드 (5+) — 1 meter = 3.28084 feet, 1 meter = 1.09361 yard
// =============================================================================

TEST_CASE("test_config_json_valid_loads_ratios", "[data][json][red]") {
    // Given: units_valid.json — 1 meter = 3.28084 feet, 1.09361 yard
    // When: loadJson
    // Then: success, registry has 3 units
    const auto result = ConfigLoader::loadJson(fixturePath("units_valid.json").string());
    REQUIRE(result.success);
    REQUIRE(result.registry.unitCount() == 3);
    const Converter converter(result.registry);
    REQUIRE(nearlyEqual(converter.convert("meter", 1.0, "feet"), 3.28084, 1e-9));
}

TEST_CASE("test_config_json_valid_convert_meter_to_yard", "[data][json][red]") {
    // Given: 1 meter = 1.09361 yard from JSON
    // When: convert after load
    // Then: 1.09361 yard for 1 meter
    const auto result = ConfigLoader::loadJson(fixturePath("units_valid.json").string());
    REQUIRE(result.success);
    const Converter converter(result.registry);
    REQUIRE(nearlyEqual(converter.convert("meter", 1.0, "yard"), 1.09361, 1e-5));
}

TEST_CASE("test_config_json_missing_file_fails_data001", "[data][json][red]") {
    // Given: non-existent path
    // When: loadJson
    // Then: ERR-DATA-001, default feet=3.28084 preserved
    const auto result = ConfigLoader::loadJson("nonexistent_config_file.json");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorCode == "ERR-DATA-001");
    REQUIRE(result.registry.hasUnit("feet"));
    REQUIRE(nearlyEqual(result.registry.factorFor("feet"), 3.28084, 1e-9));
}

TEST_CASE("test_config_json_invalid_schema_fails_data002", "[data][json][red]") {
    // Given: base_unit != meter
    // When: loadJson invalid schema
    // Then: ERR-DATA-002
    const auto result = ConfigLoader::loadJson(fixturePath("units_invalid_schema.json").string());
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorCode == "ERR-DATA-002");
}

TEST_CASE("test_config_json_feet_to_meter_reverse_after_load", "[data][json][red]") {
    // Given: 1 meter = 3.28084 feet loaded from JSON
    // When: convert feet → meter (역변환)
    // Then: 0.304800 meter (ε = 1e-5)
    const auto result = ConfigLoader::loadJson(fixturePath("units_valid.json").string());
    REQUIRE(result.success);
    const Converter converter(result.registry);
    REQUIRE(nearlyEqual(converter.convert("feet", 1.0, "meter"), 0.304800, 1e-5));
}

// =============================================================================
// YAML 설정 로드 (5+) — 1 meter = 3.28084 feet
// =============================================================================

TEST_CASE("test_config_yaml_valid_loads_ratios", "[data][yaml][red]") {
    // Given: units_valid.yaml — 1 meter = 3.28084 feet
    // When: loadYaml
    // Then: success, meter:2.5 → feet 8.202100
    const auto result = ConfigLoader::loadYaml(fixturePath("units_valid.yaml").string());
    REQUIRE(result.success);
    const Converter converter(result.registry);
    REQUIRE(nearlyEqual(converter.convert("meter", 2.5, "feet"), 8.202100, 1e-5));
}

TEST_CASE("test_config_yaml_valid_meter_to_yard", "[data][yaml][red]") {
    // Given: 1 meter = 1.09361 yard in YAML
    // When: convert 2.5 meter → yard
    // Then: 2.734025 yard (ε = 1e-5)
    const auto result = ConfigLoader::loadYaml(fixturePath("units_valid.yaml").string());
    REQUIRE(result.success);
    const Converter converter(result.registry);
    REQUIRE(nearlyEqual(converter.convert("meter", 2.5, "yard"), 2.734025, 1e-5));
}

TEST_CASE("test_config_yaml_missing_file_fails", "[data][yaml][red]") {
    // Given: missing YAML path
    // When: loadYaml
    // Then: ERR-DATA-001
    const auto result = ConfigLoader::loadYaml("no_such_units.yaml");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorCode == "ERR-DATA-001");
}

TEST_CASE("test_config_yaml_invalid_ratio_fails_data002", "[data][yaml][red]") {
    // Given: invalid ratio in YAML
    // When: loadYaml
    // Then: ERR-DATA-002
    const auto result = ConfigLoader::loadYaml(fixturePath("units_invalid.yaml").string());
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorCode == "ERR-DATA-002");
}

TEST_CASE("test_config_yaml_load_defaults_on_failure", "[data][yaml][red]") {
    // Given: invalid YAML
    // When: load fails
    // Then: default 3.28084 feet / 1.09361 yard factors preserved
    const auto result = ConfigLoader::loadYaml(fixturePath("units_invalid.yaml").string());
    REQUIRE_FALSE(result.success);
    REQUIRE(nearlyEqual(result.registry.factorFor("feet"), 3.28084, 1e-9));
    REQUIRE(nearlyEqual(result.registry.factorFor("yard"), 1.09361, 1e-9));
}
