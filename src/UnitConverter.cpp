#include <iostream>
#include <string>

#include "boundary/InputParser.hpp"
#include "boundary/OutputFormatter.hpp"
#include "domain/Converter.hpp"
#include "domain/UnitRegistry.hpp"

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
        boundary::writeConversionTable(std::cout, parsed.value, parsed.unit, results);
    } catch (const std::invalid_argument& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
