#include "orderbook.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

void OrderBook::update(const nlohmann::json& data) {
    try {
        // Header with timestamp and metadata
        std::cout << "\n╔════════════════════════════════════════════╗";
        std::cout << "\n║ 📊 ORDERBOOK UPDATE                        ║";
        std::cout << "\n╠════════════════════════════════════════════╣";
        std::cout << "\n║ 🕒 Timestamp: " << data["timestamp"].get<uint64_t>();
        std::cout << "\n║ 📝 Type: " << data["type"].get<std::string>();
        std::cout << "\n║ 🔄 Change ID: " << data["change_id"].get<uint64_t>();
        std::cout << "\n║ 💱 Instrument: " << data["instrument_name"].get<std::string>();
        std::cout << "\n╠════════════════════════════════════════════╣\n";

        // Handle asks updates
        if (data.contains("asks") && !data["asks"].empty()) {
            std::cout << "\n🔴 ASKS (SELL) Updates:\n";
            std::cout << "┌──────────┬────────────────┬────────────────┐\n";
            std::cout << "│ ACTION   │     PRICE      │      SIZE      │\n";
            std::cout << "├──────────┼────────────────┼────────────────┤\n";
            
            for (const auto& ask : data["asks"]) {
                std::string action = ask[0].get<std::string>();
                double price = ask[1].get<double>();
                double size = ask[2].get<double>();

                // Format each row with fixed widths
                std::cout << "│ " << std::left << std::setw(8) << action 
                         << "│ " << std::right << std::setw(12) << format_with_commas(price, 1) 
                         << " │ " << std::setw(12) << format_with_commas(size, 4) 
                         << " │\n";

                // Update the orderbook
                if (action == "delete" || size == 0) {
                    asks.erase(price);
                } else {
                    asks[price] = size;
                }
            }
            std::cout << "└──────────┴────────────────┴────────────────┘\n";
        }

        // Handle bids updates
        if (data.contains("bids") && !data["bids"].empty()) {
            std::cout << "\n🟢 BIDS (BUY) Updates:\n";
            std::cout << "┌──────────┬────────────────┬────────────────┐\n";
            std::cout << "│ ACTION   │     PRICE      │      SIZE      │\n";
            std::cout << "├──────────┼────────────────┼────────────────┤\n";
            
            for (const auto& bid : data["bids"]) {
                std::string action = bid[0].get<std::string>();
                double price = bid[1].get<double>();
                double size = bid[2].get<double>();

                // Format each row with fixed widths
                std::cout << "│ " << std::left << std::setw(8) << action 
                         << "│ " << std::right << std::setw(12) << format_with_commas(price, 1) 
                         << " │ " << std::setw(12) << format_with_commas(size, 4) 
                         << " │\n";

                // Update the orderbook
                if (action == "delete" || size == 0) {
                    bids.erase(price);
                } else {
                    bids[price] = size;
                }
            }
            std::cout << "└──────────┴────────────────┴────────────────┘\n";
        }

        std::cout << "\n╚════════════════════════════════════════════╝\n";

    } catch (const std::exception& e) {
        std::cerr << "❌ Error in update: " << e.what() << std::endl;
    }
}

// Helper function to format numbers with commas
std::string OrderBook::format_with_commas(double value, int precision) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    std::string str = ss.str();
    
    size_t pos = str.find('.');
    if (pos == std::string::npos) {
        pos = str.length();
    }
    
    for (int i = static_cast<int>(pos) - 3; i > 0; i -= 3) {
        str.insert(static_cast<size_t>(i), ",");
    }
    
    return str;
}

std::pair<std::vector<std::pair<double, double>>, std::vector<std::pair<double, double>>>
OrderBook::getTopLevels(int n) const {
    std::vector<std::pair<double, double>> top_bids;
    std::vector<std::pair<double, double>> top_asks;

    // Get top n bids
    for (auto it = bids.begin(); it != bids.end() && top_bids.size() < n; ++it) {
        top_bids.emplace_back(it->first, it->second);
    }

    // Get top n asks
    for (auto it = asks.begin(); it != asks.end() && top_asks.size() < n; ++it) {
        top_asks.emplace_back(it->first, it->second);
    }

    return {top_bids, top_asks};
} 