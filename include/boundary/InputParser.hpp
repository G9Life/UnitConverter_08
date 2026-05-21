#pragma once

#include <string>

namespace boundary {

struct ParsedInput {
    std::string unit;
    double value;
};

class InputParser {
public:
    static ParsedInput parse(const std::string& line);
};

}  // namespace boundary
