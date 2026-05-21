#pragma once

#include <map>
#include <string>

namespace domain {

// factor: multiply meter-hub value to obtain this unit (1 meter = 3.28084 feet → feet factor = 3.28084)
class UnitRegistry {
public:
    static UnitRegistry defaultRegistry();

    bool hasUnit(const std::string& unit) const;
    double factorFor(const std::string& unit) const;
    std::size_t unitCount() const;
    std::map<std::string, double> allUnits() const;

    void registerUnit(const std::string& unit, double metersPerUnit);
    void setFactor(const std::string& unit, double hubFactor);
    void clear();
    void loadDefaults();

private:
    std::map<std::string, double> factors_;
};

}  // namespace domain
