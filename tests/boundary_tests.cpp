#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "boundary/InputParser.hpp"
#include "domain/Converter.hpp"
#include "domain/UnitRegistry.hpp"

using namespace boundary;
using namespace domain;

namespace {

void requireErrPrefix(const std::exception& ex, const std::string& prefix) {
    const std::string message = ex.what();
    REQUIRE(message.rfind(prefix, 0) == 0);
}

}  // namespace

// =============================================================================
// 정상 파싱·변환 연계 (5+) — 1 meter = 3.28084 feet
// =============================================================================

TEST_CASE("test_parse_meter_colon_value_success", "[parse][boundary][red]") {
    // Given: input "meter:2.5"
    // When: InputParser::parse
    // Then: unit=meter, value=2.5
    const ParsedInput parsed = InputParser::parse("meter:2.5");
    REQUIRE(parsed.unit == "meter");
    REQUIRE(parsed.value == 2.5);
}

TEST_CASE("test_parse_feet_colon_value_success", "[parse][boundary][red]") {
    // Given: input "feet:3.28084" (1 meter = 3.28084 feet)
    // When: parse
    // Then: unit=feet, value=3.28084
    const ParsedInput parsed = InputParser::parse("feet:3.28084");
    REQUIRE(parsed.unit == "feet");
    REQUIRE(nearlyEqual(parsed.value, 3.28084, 1e-9));
}

TEST_CASE("test_parse_yard_colon_value_success", "[parse][boundary][red]") {
    // Given: input "yard:1.09361" (1 meter = 1.09361 yard)
    // When: parse
    // Then: unit=yard, value=1.09361
    const ParsedInput parsed = InputParser::parse("yard:1.09361");
    REQUIRE(parsed.unit == "yard");
    REQUIRE(nearlyEqual(parsed.value, 1.09361, 1e-9));
}

TEST_CASE("test_parse_then_meter_to_feet_chain", "[parse][boundary][red]") {
    // Given: 1 meter = 3.28084 feet, parsed "meter:2.5"
    // When: parse + convert to feet
    // Then: 8.202100 feet (ε = 1e-5)
    const ParsedInput parsed = InputParser::parse("meter:2.5");
    const Converter converter(UnitRegistry::defaultRegistry());
    const double result = converter.convert(parsed.unit, parsed.value, "feet");
    REQUIRE(nearlyEqual(result, 8.202100, 1e-5));
}

TEST_CASE("test_parse_trim_whitespace_success", "[parse][boundary][red]") {
    // Given: "  meter : 2.5  "
    // When: parse with trim
    // Then: unit=meter, value=2.5
    const ParsedInput parsed = InputParser::parse("  meter : 2.5  ");
    REQUIRE(parsed.unit == "meter");
    REQUIRE(parsed.value == 2.5);
}

// =============================================================================
// 경계값 파싱 (5+)
// =============================================================================

TEST_CASE("test_parse_meter_zero_throws_positive_required", "[parse][boundary][red]") {
    // Given: value = 0 (경계)
    // When: parse "meter:0"
    // Then: ERR-INPUT-003 prefix + Value must be positive
    try {
        InputParser::parse("meter:0");
        FAIL("Expected std::invalid_argument for meter:0");
    } catch (const std::invalid_argument& ex) {
        requireErrPrefix(ex, "ERR-INPUT-003");
        REQUIRE(std::string(ex.what()).find("Value must be positive") != std::string::npos);
    }
}

TEST_CASE("test_parse_meter_large_value_success", "[parse][boundary][red]") {
    // Given: very large finite value
    // When: parse "meter:1e50"
    // Then: value preserved
    const ParsedInput parsed = InputParser::parse("meter:1e50");
    REQUIRE(parsed.value == 1e50);
}

TEST_CASE("test_parse_meter_six_decimal_precision", "[parse][boundary][red]") {
    // Given: value with 6 decimal places
    // When: parse "meter:1.123456"
    // Then: exact 1.123456
    const ParsedInput parsed = InputParser::parse("meter:1.123456");
    REQUIRE(nearlyEqual(parsed.value, 1.123456, 1e-9));
}

TEST_CASE("test_parse_min_positive_value", "[parse][boundary][red]") {
    // Given: 1 meter = 3.28084 feet, value = 1e-10
    // When: parse minimum positive
    // Then: success, value > 0
    const ParsedInput parsed = InputParser::parse("meter:1e-10");
    REQUIRE(parsed.value > 0.0);
}

TEST_CASE("test_parse_meter_infinity_throws", "[parse][boundary][red]") {
    // Given: non-finite value
    // When: parse "meter:inf"
    // Then: reject with invalid number or positive constraint
    REQUIRE_THROWS_AS(InputParser::parse("meter:inf"), std::invalid_argument);
}

// =============================================================================
// 예외 파싱·변환 (5+) — PRD ERR-INPUT prefix
// =============================================================================

TEST_CASE("test_parse_invalid_format_no_colon_throws", "[parse][boundary][red]") {
    // Given: "meter2.5" (콜론 없음)
    // When: parse
    // Then: ERR-INPUT-001 prefix
    try {
        InputParser::parse("meter2.5");
        FAIL("Expected invalid format");
    } catch (const std::invalid_argument& ex) {
        requireErrPrefix(ex, "ERR-INPUT-001");
    }
}

TEST_CASE("test_parse_meter_negative_throws", "[parse][boundary][red]") {
    // Given: "meter:-1.0"
    // When: parse
    // Then: ERR-INPUT-003 prefix
    try {
        InputParser::parse("meter:-1.0");
        FAIL("Expected negative rejection");
    } catch (const std::invalid_argument& ex) {
        requireErrPrefix(ex, "ERR-INPUT-003");
    }
}

TEST_CASE("test_parse_invalid_number_abc_throws", "[parse][boundary][red]") {
    // Given: "meter:abc"
    // When: parse
    // Then: ERR-INPUT-004 prefix
    try {
        InputParser::parse("meter:abc");
        FAIL("Expected invalid number");
    } catch (const std::invalid_argument& ex) {
        requireErrPrefix(ex, "ERR-INPUT-004");
    }
}

TEST_CASE("test_convert_unknown_unit_parsec_throws", "[parse][boundary][red]") {
    // Given: parsec:1.0 parsed, 1 meter = 3.28084 feet registry
    // When: convert to feet
    // Then: ERR-INPUT-002 Unknown unit: parsec
    const ParsedInput parsed = InputParser::parse("parsec:1.0");
    const Converter converter(UnitRegistry::defaultRegistry());
    try {
        converter.convert(parsed.unit, parsed.value, "feet");
        FAIL("Expected unknown unit");
    } catch (const std::invalid_argument& ex) {
        requireErrPrefix(ex, "ERR-INPUT-002");
        REQUIRE(std::string(ex.what()).find("parsec") != std::string::npos);
    }
}

TEST_CASE("test_convert_unknown_unit_mile_throws", "[parse][boundary][red]") {
    // Given: mile:1.0 (없는 단위)
    // When: convert to meter
    // Then: ERR-INPUT-002 Unknown unit: mile
    const ParsedInput parsed = InputParser::parse("mile:1.0");
    const Converter converter(UnitRegistry::defaultRegistry());
    try {
        converter.convert(parsed.unit, parsed.value, "meter");
        FAIL("Expected unknown unit mile");
    } catch (const std::invalid_argument& ex) {
        requireErrPrefix(ex, "ERR-INPUT-002");
        REQUIRE(std::string(ex.what()).find("mile") != std::string::npos);
    }
}
