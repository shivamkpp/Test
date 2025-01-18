#include "deribit_api.h"
#include "config.h"
#include <iostream>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include "orderbook.hpp"

using json = nlohmann::json;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

class DeribitClient {
public:
    DeribitClient() : orderbook() {
        // Set up WebSocket client
        client_.set_access_channels(websocketpp::log::alevel::none);
        client_.set_error_channels(websocketpp::log::elevel::fatal);
        
        client_.init_asio();
        client_.set_tls_init_handler(bind(&DeribitClient::on_tls_init, this, ::_1));
        client_.set_message_handler(bind(&DeribitClient::on_message, this, ::_1, ::_2));
        client_.set_open_handler(bind(&DeribitClient::on_open, this, ::_1));
        client_.set_close_handler(bind(&DeribitClient::on_close, this, ::_1));
    }

    void connect() {
        websocketpp::lib::error_code ec;
        connection_ = client_.get_connection("wss://test.deribit.com/ws/api/v2", ec);
        if (ec) {
            std::cout << "❌ Could not create connection: " << ec.message() << std::endl;
            return;
        }

        client_.connect(connection_);
        client_.run();
    }

    void display_orderbook(const std::vector<std::pair<double, double>>& bids,
                          const std::vector<std::pair<double, double>>& asks) {
        // Clear screen (cross-platform way)
        std::cout << "\033[2J\033[H";
        
        // Header
        std::cout << "\n📊 BTC-PERPETUAL Orderbook\n";
        std::cout << "═══════════════════════════════════════════════════════\n";
        
        // Column headers
        std::cout << std::setw(15) << std::left << "ASKS" 
                  << std::setw(15) << "Price" 
                  << std::setw(15) << "Size" 
                  << std::setw(15) << "Total($)" << "\n";
        std::cout << "───────────────────────────────────────────────────────\n";

        // Print asks (sells) in reverse order (lowest ask first)
        for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
            double total = it->first * it->second;
            std::cout << std::setw(15) << "🔴 SELL" 
                      << std::fixed 
                      << std::setw(15) << std::setprecision(1) << it->first 
                      << std::setw(15) << std::setprecision(4) << it->second
                      << std::setw(15) << std::setprecision(1) << total << "\n";
        }

        // Spread calculation
        if (!asks.empty() && !bids.empty()) {
            double spread = asks.front().first - bids.front().first;
            double spread_pct = (spread / bids.front().first) * 100;
            
            std::cout << "\n💫 SPREAD: $" << std::fixed << std::setprecision(1) << spread
                      << " (" << std::setprecision(3) << spread_pct << "%)\n\n";
        } else {
            std::cout << "\n\n";
        }

        // Print bids (buys)
        for (const auto& bid : bids) {
            double total = bid.first * bid.second;
            std::cout << std::setw(15) << "🟢 BUY" 
                      << std::fixed 
                      << std::setw(15) << std::setprecision(1) << bid.first 
                      << std::setw(15) << std::setprecision(4) << bid.second
                      << std::setw(15) << std::setprecision(1) << total << "\n";
        }

        // Footer
        std::cout << "═══════════════════════════════════════════════════════\n";
        
        // Current time
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::cout << "🕒 Last Update: " << std::put_time(std::localtime(&time), "%H:%M:%S") << "\n";
        std::cout << "Press Ctrl+C to exit\n";
    }

private:
    context_ptr on_tls_init(websocketpp::connection_hdl) {
        context_ptr ctx = websocketpp::lib::make_shared<websocketpp::lib::asio::ssl::context>(
            websocketpp::lib::asio::ssl::context::sslv23);
            
        ctx->set_options(
            websocketpp::lib::asio::ssl::context::default_workarounds |
            websocketpp::lib::asio::ssl::context::no_sslv2 |
            websocketpp::lib::asio::ssl::context::no_sslv3 |
            websocketpp::lib::asio::ssl::context::single_dh_use
        );
        return ctx;
    }

    void on_open(websocketpp::connection_hdl hdl) {
        // Subscribe to BTC-PERPETUAL orderbook with standard format
        json subscription_msg = {
            {"jsonrpc", "2.0"},
            {"method", "public/subscribe"},
            {"params", {
                {"channels", {"book.BTC-PERPETUAL.100ms"}}  // Changed from raw to standard format
            }},
            {"id", 42}
        };

        std::string msg = subscription_msg.dump();
        std::cout << "Sending subscription: " << msg << std::endl;
        client_.send(hdl, msg, websocketpp::frame::opcode::text);
        std::cout << "✅ Connected to Deribit and sent subscription request\n";
    }

    void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
        try {
            json data = json::parse(msg->get_payload());
            
            // Handle subscription confirmation
            if (data.contains("method") && data["method"] == "subscription") {
                if (data.contains("params") && data["params"].contains("data")) {
                    auto& book_data = data["params"]["data"];
                    
                    // Print header
                    std::cout << "\n┌────────────────────────────────────────┐";
                    std::cout << "\n│ 📡 SUBSCRIPTION UPDATE                  │";
                    std::cout << "\n├────────────────────────────────────────┤";
                    std::cout << "\n│ 🕒 Time: " << format_timestamp(book_data["timestamp"].get<uint64_t>());
                    std::cout << "\n│ 📝 Type: " << book_data["type"].get<std::string>();
                    std::cout << "\n│ 🔄 Change ID: " << book_data["change_id"].get<uint64_t>();
                    std::cout << "\n│ 💱 " << book_data["instrument_name"].get<std::string>();
                    std::cout << "\n├────────────────────────────────────────┤\n";

                    // Update orderbook and display changes
                    orderbook.update(book_data);
                    
                    // Display current state
                    auto [bids, asks] = orderbook.getTopLevels(5);
                    if (!bids.empty() || !asks.empty()) {
                        display_orderbook(bids, asks);
                    }
                    
                    std::cout << "└────────────────────────────────────────┘\n";
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "❌ Error processing message: " << e.what() << std::endl;
        }
    }

    void on_close(websocketpp::connection_hdl) {
        std::cout << "⚠️ WebSocket connection closed\n";
    }

    std::string format_timestamp(uint64_t timestamp) {
        time_t time = timestamp / 1000;  // Convert to seconds
        struct tm* tm_info = localtime(&time);
        char buffer[26];
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        return std::string(buffer);
    }

    client client_;
    client::connection_ptr connection_;
    OrderBook orderbook;
};


int main() {
    try {
        // Load configuration
        if (!Config::loadConfig("config.json")) {
            std::cerr << "Failed to load configuration" << std::endl;
            return 1;
        }

        // Initialize API with loaded credentials
        DeribitAPI api(Config::CLIENT_ID, Config::CLIENT_SECRET, Config::BASE_URL);
        
        // Authenticate
        if (!api.authenticate()) {
            std::cerr << "Authentication failed" << std::endl;
            return 1;
        }

        while (true) {
            std::cout << "\nDeribit Trading Menu:" << std::endl;
            std::cout << "1. Place BTC Order" << std::endl;
            std::cout << "2. Show Open Orders" << std::endl;
            std::cout << "3. Cancel Order" << std::endl;
            std::cout << "4. Modify Order" << std::endl;
            std::cout << "5. View Orderbook" << std::endl;
            std::cout << "6. View Current Positions" << std::endl;
            std::cout << "7. Data Streaming" << std::endl;
            std::cout << "8. Exit" << std::endl;
            std::cout << "Enter your choice (1-8): ";

            int choice;
            std::cin >> choice;

            switch (choice) {
                case 1:
                    api.place_btc_order();
                    break;
                case 2:
                    api.showOrders("BTC-PERPETUAL");
                    break;
                case 3: {
                    std::string order_id;
                    std::cout << "Enter order ID to cancel: ";
                    std::cin >> order_id;
                    api.cancelOrder(order_id);
                    break;
                }
                case 4: {
                    std::string order_id;
                    double amount, price;
                    std::cout << "Enter order ID to modify: ";
                    std::cin >> order_id;
                    std::cout << "Enter new amount in USD (min 100): ";
                    std::cin >> amount;
                    std::cout << "Enter new price: ";
                    std::cin >> price;
                    api.modifyOrder(order_id, amount, price);
                    break;
                }
                case 5: {
                    std::string instrument = "BTC-PERPETUAL";  // Default instrument
                    api.getOrderbook(instrument);
                    break;
                }
                case 6: {
                    std::string currency = "BTC";  // Default to BTC
                    api.viewCurrentPositions(currency);
                    break;
                }
                case 7:{
                        try {
                DeribitClient client;
                client.connect();
            } catch (const std::exception& e) {
                std::cerr << "❌ Error: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "❌ Unknown error occurred\n";
            }

                }
                case 8:
                    return 0;
                default:
                    std::cout << "Invalid choice" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
} 