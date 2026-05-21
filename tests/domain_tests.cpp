#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <stdexcept>

#include "domain/Converter.hpp"
#include "domain/UnitRegistry.hpp"

using namespace domain;

namespace {

Converter makeConverter() {
    return Converter(UnitRegistry::defaultRegistry());
}

}  // namespace

// =============================================================================
// 정상 변환 (5+) — 1 meter = 3.28084 feet, 1 meter = 1.09361 yard
// =============================================================================

TEST_CASE("test_meter_to_feet_normal_returns_8_2021", "[ratio][domain][red]") {
    // Given: 1 meter = 3.28084 feet, source value 2.5 meter
    // When: convert("meter", 2.5, "feet")
    // Then: 2.5 × 3.28084 = 8.202100 feet (ε = 1e-5)
    const Converter converter = makeConverter();
    const double result = converter.convert("meter", 2.5, "feet");
    REQUIRE(nearlyEqual(result, 8.202100, 1e-5));
}

TEST_CASE("test_meter_to_yard_normal_returns_2_7340", "[ratio][domain][red]") {
    // Given: 1 meter = 1.09361 yard, source value 2.5 meter
    // When: convert("meter", 2.5, "yard")
    // Then: 2.5 × 1.09361 = 2.734025 yard (ε = 1e-5)
    const Converter converter = makeConverter();
    const double result = converter.convert("meter", 2.5, "yard");
    REQUIRE(nearlyEqual(result, 2.734025, 1e-5));
}

TEST_CASE("test_feet_to_meter_reverse_returns_0_3048", "[ratio][domain][red]") {
    // Given: 1 meter = 3.28084 feet → 1 feet = 1/3.28084 meter
    // When: convert("feet", 1.0, "meter") 역변환
    // Then: 0.304800 meter (ε = 1e-5)
    const Converter converter = makeConverter();
    const double result = converter.convert("feet", 1.0, "meter");
    REQUIRE(nearlyEqual(result, 0.304800, 1e-5));
}

TEST_CASE("test_meter_to_feet_one_meter_returns_3_28084", "[ratio][domain][red]") {
    // Given: 1 meter = 3.28084 feet
    // When: convert("meter", 1.0, "feet")
    // Then: exactly 3.28084 feet
    const Converter converter = makeConverter();
    const double result = converter.convert("meter", 1.0, "feet");
    REQUIRE(nearlyEqual(result, 3.28084, 1e-9));
}

TEST_CASE("test_feet_to_yard_via_meter_hub_returns_1_09361", "[ratio][domain][red]") {
    // Given: feet↔yard 직접 비율 없음, 1 meter = 3.28084 feet, 1 meter = 1.09361 yard
    // When: convert("feet", 3.28084, "yard") — meter 허브 경유만
    // Then: 1.09361 yard (ε = 1e-4)
    const Converter converter = makeConverter();
    const double result = converter.convert("feet", 3.28084, "yard");
    REQUIRE(nearlyEqual(result, 1.09361, 1e-4));
}

// =============================================================================
// 경계값 (5+) — 1 meter = 3.28084 feet
// =============================================================================

TEST_CASE("test_meter_to_feet_zero_value_throws", "[quantity][domain][red]") {
    // Given: 1 meter = 3.28084 feet, value = 0 (경계)
    // When: convert with zero
    // Then: std::invalid_argument (양수만 허용)
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("meter", 0.0, "feet"), std::invalid_argument);
}

TEST_CASE("test_meter_to_feet_large_value_finite", "[quantity][domain][red]") {
    // Given: 1 meter = 3.28084 feet, value = 1e100
    // When: convert very large value
    // Then: finite result, ratio preserved (result/1e100 ≈ 3.28084)
    const Converter converter = makeConverter();
    const double result = converter.convert("meter", 1e100, "feet");
    REQUIRE(std::isfinite(result));
    REQUIRE(nearlyEqual(result / 1e100, 3.28084, 1e-5));
}

TEST_CASE("test_meter_to_feet_six_decimal_precision", "[quantity][domain][red]") {
    // Given: 1 meter = 3.28084 feet, value = 1.123456 (소수 6자리)
    // When: convert
    // Then: 1.123456 × 3.28084 within ε = 1e-6
    const Converter converter = makeConverter();
    const double expected = 1.123456 * 3.28084;
    const double result = converter.convert("meter", 1.123456, "feet");
    REQUIRE(nearlyEqual(result, expected, 1e-6));
}

TEST_CASE("test_meter_to_feet_min_positive_value", "[quantity][domain][red]") {
    // Given: 1 meter = 3.28084 feet, value = 1e-10
    // When: convert minimum positive
    // Then: finite positive result
    const Converter converter = makeConverter();
    const double result = converter.convert("meter", 1e-10, "feet");
    REQUIRE(result > 0.0);
    REQUIRE(std::isfinite(result));
}

TEST_CASE("test_meter_to_feet_nan_value_throws", "[quantity][domain][red]") {
    // Given: value = NaN
    // When: convert
    // Then: reject non-finite
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("meter", NAN, "feet"), std::invalid_argument);
}

// =============================================================================
// 예외 (5+) — Domain 거부
// =============================================================================

TEST_CASE("test_convert_negative_value_throws", "[domain][red]") {
    // Given: 1 meter = 3.28084 feet, value = -1.0
    // When: convert("meter", -1.0, "feet")
    // Then: std::invalid_argument
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("meter", -1.0, "feet"), std::invalid_argument);
}

TEST_CASE("test_convert_unknown_source_unit_throws", "[domain][red]") {
    // Given: parsec not in registry
    // When: convert("parsec", 1.0, "feet")
    // Then: std::invalid_argument Unknown unit
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("parsec", 1.0, "feet"), std::invalid_argument);
}

TEST_CASE("test_convert_unknown_target_unit_throws", "[domain][red]") {
    // Given: mile not in registry
    // When: convert("meter", 1.0, "mile")
    // Then: std::invalid_argument
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("meter", 1.0, "mile"), std::invalid_argument);
}

TEST_CASE("test_register_unit_duplicate_throws", "[register][domain][red]") {
    // Given: cubit registered (1 cubit = 0.4572 meter)
    // When: registerUnit("cubit", 0.4572) again
    // Then: std::invalid_argument
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("cubit", 0.4572);
    REQUIRE_THROWS_AS(registry.registerUnit("cubit", 0.4572), std::invalid_argument);
}

TEST_CASE("test_register_unit_invalid_ratio_throws", "[register][domain][red]") {
    // Given: ratio <= 0
    // When: registerUnit("bad", 0.0)
    // Then: std::invalid_argument
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    REQUIRE_THROWS_AS(registry.registerUnit("bad", 0.0), std::invalid_argument);
}

// =============================================================================
// 동적 등록 (5+) — registerUnit 후 변환
// =============================================================================

TEST_CASE("test_register_cubit_then_convert_to_meter", "[register][domain][red]") {
    // Given: registerUnit("cubit", 0.4572) — 1 cubit = 0.4572 meter
    // When: convert("cubit", 1.0, "meter")
    // Then: 0.457200 meter (ε = 1e-5)
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("cubit", 0.4572);
    const Converter converter(registry);
    const double result = converter.convert("cubit", 1.0, "meter");
    REQUIRE(nearlyEqual(result, 0.457200, 1e-5));
}

TEST_CASE("test_register_cubit_convert_all_count_four", "[register][domain][red]") {
    // Given: default meter/feet/yard + cubit (1 meter = 3.28084 feet)
    // When: convertAll("meter", 1.0) after register
    // Then: 4 entries
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("cubit", 0.4572);
    const Converter converter(registry);
    const auto results = converter.convertAll("meter", 1.0);
    REQUIRE(results.size() == 4);
}

TEST_CASE("test_register_cubit_to_feet_via_meter", "[register][domain][red]") {
    // Given: 1 cubit = 0.4572 meter, 1 meter = 3.28084 feet
    // When: convert("cubit", 1.0, "feet")
    // Then: 0.4572 × 3.28084 feet (ε = 1e-5)
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("cubit", 0.4572);
    const Converter converter(registry);
    const double result = converter.convert("cubit", 1.0, "feet");
    REQUIRE(nearlyEqual(result, 0.4572 * 3.28084, 1e-5));
}

TEST_CASE("test_register_inch_small_ratio", "[register][domain][red]") {
    // Given: registerUnit("inch", 0.0254) — 1 inch = 0.0254 meter
    // When: convert("inch", 100.0, "meter")
    // Then: 2.540000 meter (ε = 1e-5)
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("inch", 0.0254);
    const Converter converter(registry);
    const double result = converter.convert("inch", 100.0, "meter");
    REQUIRE(nearlyEqual(result, 2.540000, 1e-5));
}

TEST_CASE("test_register_has_unit_after_registration", "[register][domain][red]") {
    // Given: registerUnit("cubit", 0.4572)
    // When: hasUnit("cubit")
    // Then: true and unitCount == 4
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("cubit", 0.4572);
    REQUIRE(registry.hasUnit("cubit"));
    REQUIRE(registry.unitCount() == 4);
}
