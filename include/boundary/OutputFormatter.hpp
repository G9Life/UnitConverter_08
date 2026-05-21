#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "domain/Converter.hpp"

namespace boundary {

double roundHalfUp4(double value);

std::string formatConversionLine(double sourceValue,
                                 const std::string& sourceUnit,
                                 double convertedValue,
                                 const std::string& targetUnit);

void writeConversionTable(std::ostream& out,
                          double sourceValue,
                          const std::string& sourceUnit,
                          const std::vector<domain::ConversionEntry>& results);

}  // namespace boundary
