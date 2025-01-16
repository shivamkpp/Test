#include "deribit_api.h"
#include "config.h"
#include <iostream>

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
            std::cout << "6. Exit" << std::endl;
            std::cout << "Enter your choice (1-6): ";

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
                case 6:
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