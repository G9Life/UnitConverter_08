#include "boundary/InputParser.hpp"

#include <cmath>
#include <cctype>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace boundary {

namespace {

std::string trim(const std::string& text) {
    std::size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }
    std::size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return text.substr(start, end - start);
}

bool isValidUnitName(const std::string& unit) {
    static const std::regex pattern(R"(^[a-zA-Z][a-zA-Z0-9_]*$)");
    return unit.size() >= 1 && unit.size() <= 32 && std::regex_match(unit, pattern);
}

}  // namespace

ParsedInput InputParser::parse(const std::string& line) {
    const std::size_t colonPos = line.find(':');
    if (colonPos == std::string::npos || line.find(':', colonPos + 1) != std::string::npos) {
        throw std::invalid_argument("Invalid format. Use unit:value (ex: meter:2.5)");
    }

    const std::string unit = trim(line.substr(0, colonPos));
    const std::string valueToken = trim(line.substr(colonPos + 1));

    if (!isValidUnitName(unit)) {
        throw std::invalid_argument("Invalid unit name: " + unit);
    }
    if (valueToken.empty()) {
        throw std::invalid_argument("Invalid number: " + valueToken);
    }

    std::size_t consumed = 0;
    double value = 0.0;
    try {
        value = std::stod(valueToken, &consumed);
    } catch (...) {
        throw std::invalid_argument("Invalid number: " + valueToken);
    }
    if (consumed != valueToken.size()) {
        throw std::invalid_argument("Invalid number: " + valueToken);
    }
    if (!std::isfinite(value) || value <= 0.0) {
        throw std::invalid_argument("Value must be positive: " + valueToken);
    }

    return {unit, value};
}

}  // namespace boundary
