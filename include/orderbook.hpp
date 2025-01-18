#pragma once
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class OrderBook {
public:
    OrderBook() = default;
    
    void update(const nlohmann::json& data);
    std::pair<std::vector<std::pair<double, double>>, std::vector<std::pair<double, double>>> 
    getTopLevels(int n = 5) const;

private:
    std::map<double, double, std::greater<double>> bids;
    std::map<double, double> asks;
    
    std::string format_with_commas(double value, int precision = 2);
}; 