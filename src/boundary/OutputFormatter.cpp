#include "boundary/OutputFormatter.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace boundary {

double roundHalfUp4(double value) {
    return std::round(value * 10000.0) / 10000.0;
}

std::string formatConversionLine(double sourceValue,
                                 const std::string& sourceUnit,
                                 double convertedValue,
                                 const std::string& targetUnit) {
    const double display = roundHalfUp4(convertedValue);
    std::ostringstream line;
    line << std::fixed << std::setprecision(4)
         << sourceValue << ' ' << sourceUnit << " = "
         << display << ' ' << targetUnit;
    return line.str();
}

void writeConversionTable(std::ostream& out,
                          double sourceValue,
                          const std::string& sourceUnit,
                          const std::vector<domain::ConversionEntry>& results) {
    for (const auto& entry : results) {
        out << formatConversionLine(sourceValue, sourceUnit, entry.value, entry.unit)
            << std::endl;
    }
}

}  // namespace boundary
