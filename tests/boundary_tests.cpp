#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

#include "boundary/InputParser.hpp"
#include "domain/Converter.hpp"
#include "domain/UnitRegistry.hpp"

using namespace boundary;
using namespace domain;

// --- 정상 파싱·변환 연계 (5+) ---

TEST_CASE("test_parse_meter_colon_value_success", "[parse][boundary]") {
    // Given: input "meter:2.5"
    // When: parse
    // Then: unit=meter, value=2.5
    const ParsedInput parsed = InputParser::parse("meter:2.5");
    REQUIRE(parsed.unit == "meter");
    REQUIRE(parsed.value == 2.5);
}

TEST_CASE("test_parse_feet_colon_value_success", "[parse][boundary]") {
    // Given: input "feet:3.28084" (1 meter = 3.28084 feet)
    // When: parse
    // Then: unit=feet, value=3.28084
    const ParsedInput parsed = InputParser::parse("feet:3.28084");
    REQUIRE(parsed.unit == "feet");
    REQUIRE(nearlyEqual(parsed.value, 3.28084, 1e-9));
}

TEST_CASE("test_parse_yard_colon_value_success", "[parse][boundary]") {
    // Given: input "yard:1.09361" (1 meter = 1.09361 yard)
    // When: parse
    // Then: unit=yard, value=1.09361
    const ParsedInput parsed = InputParser::parse("yard:1.09361");
    REQUIRE(parsed.unit == "yard");
    REQUIRE(nearlyEqual(parsed.value, 1.09361, 1e-9));
}

TEST_CASE("test_parse_then_meter_to_feet_chain", "[parse][boundary]") {
    // Given: 1 meter = 3.28084 feet, parsed meter:2.5
    // When: parse + convert
    // Then: 8.2021 feet
    const ParsedInput parsed = InputParser::parse("meter:2.5");
    const Converter converter(UnitRegistry::defaultRegistry());
    const double result = converter.convert(parsed.unit, parsed.value, "feet");
    REQUIRE(nearlyEqual(result, 8.2021, 1e-5));
}

TEST_CASE("test_parse_trim_whitespace_success", "[parse][boundary]") {
    // Given: "  meter : 2.5  "
    // When: parse
    // Then: trimmed success
    const ParsedInput parsed = InputParser::parse("  meter : 2.5  ");
    REQUIRE(parsed.unit == "meter");
    REQUIRE(parsed.value == 2.5);
}

// --- 경계값 파싱 (5+) ---

TEST_CASE("test_parse_meter_zero_throws_positive_required", "[parse][boundary]") {
    // Given: value = 0 (경계)
    // When: parse "meter:0"
    // Then: Value must be positive
    REQUIRE_THROWS_AS(InputParser::parse("meter:0"), std::invalid_argument);
}

TEST_CASE("test_parse_meter_large_value_success", "[parse][boundary]") {
    // Given: very large finite value
    // When: parse
    // Then: value preserved
    const ParsedInput parsed = InputParser::parse("meter:1e50");
    REQUIRE(parsed.value == 1e50);
}

TEST_CASE("test_parse_meter_six_decimal_precision", "[parse][boundary]") {
    // Given: value with 6 decimal places
    // When: parse "meter:1.123456"
    // Then: exact value
    const ParsedInput parsed = InputParser::parse("meter:1.123456");
    REQUIRE(nearlyEqual(parsed.value, 1.123456, 1e-9));
}

TEST_CASE("test_parse_min_positive_value", "[parse][boundary]") {
    // Given: 1 meter = 3.28084 feet, value = 1e-10
    // When: parse minimum positive
    // Then: success
    const ParsedInput parsed = InputParser::parse("meter:1e-10");
    REQUIRE(parsed.value > 0.0);
}

TEST_CASE("test_parse_meter_negative_throws", "[parse][boundary]") {
    // Given: value < 0
    // When: parse "meter:-1"
    // Then: Value must be positive: -1
    REQUIRE_THROWS_AS(InputParser::parse("meter:-1"), std::invalid_argument);
}

// --- 예외 파싱 (5+) ---

TEST_CASE("test_parse_invalid_format_no_colon_throws", "[parse][boundary]") {
    // Given: "meter2.5" (콜론 없음)
    // When: parse
    // Then: Invalid format
    REQUIRE_THROWS_AS(InputParser::parse("meter2.5"), std::invalid_argument);
}

TEST_CASE("test_parse_invalid_number_abc_throws", "[parse][boundary]") {
    // Given: "meter:abc"
    // When: parse
    // Then: Invalid number: abc
    REQUIRE_THROWS_AS(InputParser::parse("meter:abc"), std::invalid_argument);
}

TEST_CASE("test_parse_empty_unit_name_throws", "[parse][boundary]") {
    // Given: ":2.5" empty unit
    // When: parse
    // Then: Invalid unit name
    REQUIRE_THROWS_AS(InputParser::parse(":2.5"), std::invalid_argument);
}

TEST_CASE("test_parse_double_colon_throws", "[parse][boundary]") {
    // Given: "meter:2:5" multiple colons
    // When: parse
    // Then: Invalid format
    REQUIRE_THROWS_AS(InputParser::parse("meter:2:5"), std::invalid_argument);
}

TEST_CASE("test_convert_unknown_unit_parsec_throws", "[parse][boundary]") {
    // Given: parsec:1.0 parsed OK but not in registry
    // When: convert
    // Then: Unknown unit
    const ParsedInput parsed = InputParser::parse("parsec:1.0");
    const Converter converter(UnitRegistry::defaultRegistry());
    REQUIRE_THROWS_AS(converter.convert(parsed.unit, parsed.value, "feet"), std::invalid_argument);
}

TEST_CASE("test_convert_unknown_unit_mile_throws", "[parse][boundary]") {
    // Given: mile:1 (없는 단위)
    // When: convert
    // Then: Unknown unit: mile
    const ParsedInput parsed = InputParser::parse("mile:1");
    const Converter converter(UnitRegistry::defaultRegistry());
    REQUIRE_THROWS_AS(converter.convert(parsed.unit, parsed.value, "meter"), std::invalid_argument);
}
