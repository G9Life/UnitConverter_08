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

// --- 정상 변환 (5+) ---

TEST_CASE("test_meter_to_feet_normal_returns_8_2021", "[ratio][domain]") {
    // Given: 1 meter = 3.28084 feet, source 2.5 meter
    // When: convert meter → feet
    // Then: 2.5 × 3.28084 = 8.2021 (ε = 1e-9)
    const Converter converter = makeConverter();
    const double result = converter.convert("meter", 2.5, "feet");
    REQUIRE(nearlyEqual(result, 8.2021, 1e-5));
}

TEST_CASE("test_meter_to_yard_normal_returns_2_7340", "[ratio][domain]") {
    // Given: 1 meter = 1.09361 yard, source 2.5 meter
    // When: convert meter → yard
    // Then: 2.5 × 1.09361 = 2.734025 (ε)
    const Converter converter = makeConverter();
    const double result = converter.convert("meter", 2.5, "yard");
    REQUIRE(nearlyEqual(result, 2.734025, 1e-5));
}

TEST_CASE("test_feet_to_meter_reverse_returns_0_3048", "[ratio][domain]") {
    // Given: 1 meter = 3.28084 feet → 1 feet = 1/3.28084 meter
    // When: convert 1.0 feet → meter (역변환)
    // Then: 0.30480 meter (ε)
    const Converter converter = makeConverter();
    const double result = converter.convert("feet", 1.0, "meter");
    REQUIRE(nearlyEqual(result, 0.304799, 1e-5));
}

TEST_CASE("test_meter_to_feet_unit_ratio_one_meter", "[ratio][domain]") {
    // Given: 1 meter = 3.28084 feet
    // When: convert 1.0 meter → feet
    // Then: exactly 3.28084 feet
    const Converter converter = makeConverter();
    const double result = converter.convert("meter", 1.0, "feet");
    REQUIRE(nearlyEqual(result, kDefaultFeetPerMeter));
}

TEST_CASE("test_feet_to_yard_via_meter_hub", "[ratio][domain]") {
    // Given: feet↔yard는 meter 허브 경유만 (직접 비율 없음)
    // When: 3.28084 feet → yard
    // Then: meter 허브 경유 결과 ≈ 1.09361 yard
    const Converter converter = makeConverter();
    const double result = converter.convert("feet", 3.28084, "yard");
    REQUIRE(nearlyEqual(result, kDefaultYardPerMeter, 1e-4));
}

TEST_CASE("test_convert_all_meter_returns_three_units", "[ratio][domain]") {
    // Given: Registry에 meter, feet, yard (1 meter = 3.28084 feet)
    // When: convertAll("meter", 1.0)
    // Then: 3건 반환, feet = 3.28084
    const Converter converter = makeConverter();
    const auto results = converter.convertAll("meter", 1.0);
    REQUIRE(results.size() == 3);
    bool foundFeet = false;
    for (const auto& entry : results) {
        if (entry.unit == "feet") {
            foundFeet = true;
            REQUIRE(nearlyEqual(entry.value, kDefaultFeetPerMeter));
        }
    }
    REQUIRE(foundFeet);
}

// --- 경계값 (5+) ---

TEST_CASE("test_meter_to_feet_zero_value_throws", "[quantity][domain]") {
    // Given: 1 meter = 3.28084 feet, value = 0 (경계)
    // When: convert with zero
    // Then: std::invalid_argument
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("meter", 0.0, "feet"), std::invalid_argument);
}

TEST_CASE("test_meter_to_feet_large_value_finite", "[quantity][domain]") {
    // Given: 1 meter = 3.28084 feet, value = 1e100
    // When: convert very large value
    // Then: finite result, ratio preserved
    const Converter converter = makeConverter();
    const double result = converter.convert("meter", 1e100, "feet");
    REQUIRE(std::isfinite(result));
    REQUIRE(nearlyEqual(result / 1e100, kDefaultFeetPerMeter, 1e-5));
}

TEST_CASE("test_meter_to_feet_six_decimal_precision", "[quantity][domain]") {
    // Given: 1 meter = 3.28084 feet, value = 1.123456 (소수 6자리)
    // When: convert
    // Then: 1.123456 × 3.28084 within ε
    const Converter converter = makeConverter();
    const double expected = 1.123456 * kDefaultFeetPerMeter;
    const double result = converter.convert("meter", 1.123456, "feet");
    REQUIRE(nearlyEqual(result, expected, 1e-6));
}

TEST_CASE("test_meter_to_feet_min_positive_value", "[quantity][domain]") {
    // Given: 1 meter = 3.28084 feet, value = 1e-10
    // When: convert minimum positive
    // Then: finite positive result
    const Converter converter = makeConverter();
    const double result = converter.convert("meter", 1e-10, "feet");
    REQUIRE(result > 0.0);
    REQUIRE(std::isfinite(result));
}

TEST_CASE("test_meter_to_feet_nan_value_throws", "[quantity][domain]") {
    // Given: value = NaN
    // When: convert
    // Then: reject non-finite
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("meter", NAN, "feet"), std::invalid_argument);
}

TEST_CASE("test_meter_to_feet_infinity_value_throws", "[quantity][domain]") {
    // Given: value = +Inf
    // When: convert
    // Then: reject non-finite
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("meter", INFINITY, "feet"), std::invalid_argument);
}

// --- 예외 (5+) ---

TEST_CASE("test_convert_unknown_source_unit_throws", "[domain]") {
    // Given: parsec not in registry
    // When: convert parsec → feet
    // Then: std::invalid_argument (Unknown unit)
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("parsec", 1.0, "feet"), std::invalid_argument);
}

TEST_CASE("test_convert_unknown_target_unit_throws", "[domain]") {
    // Given: mile not in registry
    // When: convert meter → mile
    // Then: std::invalid_argument
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("meter", 1.0, "mile"), std::invalid_argument);
}

TEST_CASE("test_convert_negative_value_throws", "[domain]") {
    // Given: value = -1.0
    // When: convert meter → feet
    // Then: std::invalid_argument
    const Converter converter = makeConverter();
    REQUIRE_THROWS_AS(converter.convert("meter", -1.0, "feet"), std::invalid_argument);
}

TEST_CASE("test_register_unit_duplicate_throws", "[register][domain]") {
    // Given: cubit already registered
    // When: register cubit again
    // Then: std::invalid_argument
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("cubit", 0.4572);
    REQUIRE_THROWS_AS(registry.registerUnit("cubit", 0.4572), std::invalid_argument);
}

TEST_CASE("test_register_unit_invalid_ratio_throws", "[register][domain]") {
    // Given: ratio <= 0
    // When: registerUnit with zero ratio
    // Then: std::invalid_argument
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    REQUIRE_THROWS_AS(registry.registerUnit("bad", 0.0), std::invalid_argument);
}

// --- 동적 등록 (5+) ---

TEST_CASE("test_register_cubit_then_convert_to_meter", "[register][domain]") {
    // Given: registerUnit("cubit", 0.4572) — 1 cubit = 0.4572 meter
    // When: convert 1 cubit → meter
    // Then: 0.4572 meter
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("cubit", 0.4572);
    const Converter converter(registry);
    const double result = converter.convert("cubit", 1.0, "meter");
    REQUIRE(nearlyEqual(result, 0.4572, 1e-5));
}

TEST_CASE("test_register_cubit_convert_all_count_four", "[register][domain]") {
    // Given: default 3 units + cubit
    // When: convertAll after register
    // Then: 4 entries
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("cubit", 0.4572);
    const Converter converter(registry);
    const auto results = converter.convertAll("meter", 1.0);
    REQUIRE(results.size() == 4);
}

TEST_CASE("test_register_cubit_to_feet_via_meter", "[register][domain]") {
    // Given: 1 cubit = 0.4572 meter, 1 meter = 3.28084 feet
    // When: 1 cubit → feet
    // Then: 0.4572 × 3.28084 feet
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("cubit", 0.4572);
    const Converter converter(registry);
    const double result = converter.convert("cubit", 1.0, "feet");
    REQUIRE(nearlyEqual(result, 0.4572 * kDefaultFeetPerMeter, 1e-5));
}

TEST_CASE("test_register_inch_small_ratio", "[register][domain]") {
    // Given: 1 inch = 0.0254 meter
    // When: register and convert 100 inch → meter
    // Then: 2.54 meter
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("inch", 0.0254);
    const Converter converter(registry);
    const double result = converter.convert("inch", 100.0, "meter");
    REQUIRE(nearlyEqual(result, 2.54, 1e-5));
}

TEST_CASE("test_register_has_unit_after_registration", "[register][domain]") {
    // Given: cubit registered
    // When: hasUnit("cubit")
    // Then: true
    UnitRegistry registry = UnitRegistry::defaultRegistry();
    registry.registerUnit("cubit", 0.4572);
    REQUIRE(registry.hasUnit("cubit"));
    REQUIRE(registry.unitCount() == 4);
}
