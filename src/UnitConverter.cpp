#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include "boundary/InputParser.hpp"
#include "domain/Converter.hpp"
#include "domain/UnitRegistry.hpp"

namespace {

double roundHalfUp4(double value) {
    return std::round(value * 10000.0) / 10000.0;
}

}  // namespace

int main() {
    std::cout << "Insert value for converting (ex: meter:2.5): ";

    std::string input;
    if (!std::getline(std::cin, input)) {
        return 1;
    }

    try {
        const boundary::ParsedInput parsed = boundary::InputParser::parse(input);
        const domain::Converter converter(domain::UnitRegistry::defaultRegistry());
        const auto results = converter.convertAll(parsed.unit, parsed.value);

        for (const auto& entry : results) {
            const double display = roundHalfUp4(entry.value);
            std::cout << std::fixed << std::setprecision(4)
                      << parsed.value << " " << parsed.unit << " = "
                      << display << " " << entry.unit << std::endl;
        }
    } catch (const std::invalid_argument& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
