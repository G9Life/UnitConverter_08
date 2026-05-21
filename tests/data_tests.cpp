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

// --- JSON 설정 로드 (5+) ---

TEST_CASE("test_config_json_valid_loads_ratios", "[data][json]") {
    // Given: units_valid.json — 1 meter = 3.28084 feet, 1.09361 yard
    // When: loadJson
    // Then: success, registry has 3 units
    const auto result = ConfigLoader::loadJson(fixturePath("units_valid.json").string());
    REQUIRE(result.success);
    REQUIRE(result.registry.unitCount() == 3);
    const Converter converter(result.registry);
    REQUIRE(nearlyEqual(converter.convert("meter", 1.0, "feet"), kDefaultFeetPerMeter));
}

TEST_CASE("test_config_json_valid_convert_meter_to_yard", "[data][json]") {
    // Given: 1 meter = 1.09361 yard from config
    // When: convert after load
    // Then: 1.09361 yard
    const auto result = ConfigLoader::loadJson(fixturePath("units_valid.json").string());
    const Converter converter(result.registry);
    REQUIRE(nearlyEqual(converter.convert("meter", 1.0, "yard"), kDefaultYardPerMeter));
}

TEST_CASE("test_config_json_missing_file_fails_data001", "[data][json]") {
    // Given: non-existent path
    // When: loadJson
    // Then: ERR-DATA-001, defaults in result.registry
    const auto result = ConfigLoader::loadJson("nonexistent_config_file.json");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorCode == "ERR-DATA-001");
    REQUIRE(result.registry.hasUnit("feet"));
    REQUIRE(nearlyEqual(result.registry.factorFor("feet"), kDefaultFeetPerMeter));
}

TEST_CASE("test_config_json_invalid_schema_fails_data002", "[data][json]") {
    // Given: base_unit != meter
    // When: loadJson invalid schema
    // Then: ERR-DATA-002
    const auto result = ConfigLoader::loadJson(fixturePath("units_invalid_schema.json").string());
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorCode == "ERR-DATA-002");
}

TEST_CASE("test_config_json_load_keeps_default_on_failure", "[data][json]") {
    // Given: loadConfig(없는 경로) — TC-B-07
    // When: load fails
    // Then: 기본값 3.28084 / 1.09361 유지
    const auto result = ConfigLoader::loadJson("missing/units.json");
    REQUIRE_FALSE(result.success);
    REQUIRE(nearlyEqual(result.registry.factorFor("feet"), 3.28084));
    REQUIRE(nearlyEqual(result.registry.factorFor("yard"), 1.09361));
}

TEST_CASE("test_config_json_feet_to_meter_reverse_after_load", "[data][json]") {
    // Given: 1 meter = 3.28084 feet loaded from JSON
    // When: convert feet → meter
    // Then: 0.3048 meter (역변환)
    const auto result = ConfigLoader::loadJson(fixturePath("units_valid.json").string());
    const Converter converter(result.registry);
    REQUIRE(nearlyEqual(converter.convert("feet", 1.0, "meter"), 0.304799, 1e-5));
}

// --- YAML 설정 로드 (5+) ---

TEST_CASE("test_config_yaml_valid_loads_ratios", "[data][yaml]") {
    // Given: units_valid.yaml — 1 meter = 3.28084 feet
    // When: loadYaml
    // Then: success
    const auto result = ConfigLoader::loadYaml(fixturePath("units_valid.yaml").string());
    REQUIRE(result.success);
    const Converter converter(result.registry);
    REQUIRE(nearlyEqual(converter.convert("meter", 2.5, "feet"), 8.2021, 1e-5));
}

TEST_CASE("test_config_yaml_valid_meter_to_yard", "[data][yaml]") {
    // Given: 1 meter = 1.09361 yard in YAML
    // When: convert
    // Then: 2.7340 for 2.5 meter
    const auto result = ConfigLoader::loadYaml(fixturePath("units_valid.yaml").string());
    const Converter converter(result.registry);
    REQUIRE(nearlyEqual(converter.convert("meter", 2.5, "yard"), 2.734025, 1e-5));
}

TEST_CASE("test_config_yaml_missing_file_fails", "[data][yaml]") {
    // Given: missing YAML path
    // When: loadYaml
    // Then: ERR-DATA-001
    const auto result = ConfigLoader::loadYaml("no_such_units.yaml");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorCode == "ERR-DATA-001");
}

TEST_CASE("test_config_yaml_invalid_ratio_fails_data002", "[data][yaml]") {
    // Given: meter ratio 0 in YAML
    // When: loadYaml
    // Then: ERR-DATA-002
    const auto result = ConfigLoader::loadYaml(fixturePath("units_invalid.yaml").string());
    REQUIRE_FALSE(result.success);
    REQUIRE(result.errorCode == "ERR-DATA-002");
}

TEST_CASE("test_config_yaml_load_defaults_on_failure", "[data][yaml]") {
    // Given: invalid YAML, default registry fallback
    // When: load fails
    // Then: default feet/yard factors preserved
    const auto result = ConfigLoader::loadYaml(fixturePath("units_invalid.yaml").string());
    REQUIRE_FALSE(result.success);
    REQUIRE(nearlyEqual(result.registry.factorFor("feet"), 3.28084));
    REQUIRE(nearlyEqual(result.registry.factorFor("yard"), 1.09361));
}

TEST_CASE("test_config_load_dispatch_json_format", "[data]") {
    // Given: ConfigFormat::Json
    // When: load()
    // Then: same as loadJson success
    const auto result = ConfigLoader::load(fixturePath("units_valid.json").string(), ConfigFormat::Json);
    REQUIRE(result.success);
}
