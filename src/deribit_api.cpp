#include "deribit_api.h"
#include <iostream>
#include <iomanip>
#include <curl/curl.h>
#include <sstream>
#include <cmath>
#include <ctime>
#include <algorithm>

DeribitAPI::DeribitAPI(const std::string& client_id, const std::string& client_secret, const std::string& base_url)
    : BASE_URL(base_url), CLIENT_ID(client_id), CLIENT_SECRET(client_secret) {}

size_t DeribitAPI::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string DeribitAPI::sendPostRequest(const std::string& url, const Json::Value& payload, const std::string& access_token) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    Json::Value jsonPayload;
    if (url.find("/public/") != std::string::npos) {
        jsonPayload = payload;
    } else {
        jsonPayload["jsonrpc"] = "2.0";
        jsonPayload["id"] = "request";
        std::string method = "private/";
        size_t lastSlash = url.find_last_of('/');
        if (lastSlash != std::string::npos) {
            method += url.substr(lastSlash + 1);
        }
        jsonPayload["method"] = method;
        jsonPayload["params"] = payload;
    }

    Json::StreamWriterBuilder writer;
    std::string payloadStr = Json::writeString(writer, jsonPayload);

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!access_token.empty()) {
            std::string auth_header = "Authorization: Bearer " + access_token;
            headers = curl_slist_append(headers, auth_header.c_str());
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            throw std::runtime_error("CURL error: " + std::string(curl_easy_strerror(res)));
        }
    }
    return readBuffer;
}

bool DeribitAPI::authenticate() {
    std::string url = BASE_URL + "/api/v2/public/auth";

    Json::Value payload;
    payload["jsonrpc"] = "2.0";
    payload["id"] = "auth";
    payload["method"] = "public/auth";
    payload["params"] = Json::Value(Json::objectValue);
    payload["params"]["grant_type"] = "client_credentials";
    payload["params"]["client_id"] = CLIENT_ID;
    payload["params"]["client_secret"] = CLIENT_SECRET;

    std::cout << "Authenticating with:" << std::endl;
    std::cout << "Client ID: " << CLIENT_ID << std::endl;
    std::cout << "Base URL: " << BASE_URL << std::endl;

    std::string response = sendPostRequest(url, payload, "");
    std::cout << "Auth response: " << response << std::endl;

    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::istringstream s(response);
    std::string errs;
    if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
        if (jsonResponse.isMember("result")) {
            access_token = jsonResponse["result"]["access_token"].asString();
            std::cout << "Successfully authenticated!" << std::endl;
            return true;
        } else if (jsonResponse.isMember("error")) {
            std::cerr << "Authentication error: " << jsonResponse["error"]["message"].asString() << std::endl;
            return false;
        }
    }
    
    std::cerr << "Failed to parse authentication response" << std::endl;
    return false;
}

void DeribitAPI::place_btc_order() {
    std::string url = BASE_URL + "/api/v2/private/buy";

    Json::Value payload;
    payload["instrument_name"] = "BTC-PERPETUAL";
    payload["amount"] = 100;
    payload["type"] = "limit";
    payload["price"] = 35000;
    payload["post_only"] = false;
    payload["reduce_only"] = false;

    std::string response = sendPostRequest(url, payload, access_token);
    
    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::istringstream s(response);
    std::string errs;
    if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
        if (jsonResponse.isMember("error")) {
            std::cout << "\n❌ Order Placement Failed" << std::endl;
            std::cout << "Error: " << jsonResponse["error"]["message"].asString() << std::endl;
        } else if (jsonResponse.isMember("result")) {
            const Json::Value& order = jsonResponse["result"]["order"];
            if (order.isObject()) {
                std::cout << "\n✅ Order Placed Successfully" << std::endl;
                std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
                std::cout << "Order ID: " << order["order_id"].asString() << std::endl;
                std::cout << "Instrument: " << order["instrument_name"].asString() << std::endl;
                std::cout << "Direction: " << order["direction"].asString() << std::endl;
                std::cout << "Price: $" << std::fixed << std::setprecision(2) << order["price"].asDouble() << std::endl;
                std::cout << "Amount: $" << order["amount"].asDouble() << std::endl;
                std::cout << "Type: " << order["order_type"].asString() << std::endl;
                std::cout << "Status: " << order["order_state"].asString() << std::endl;
                std::cout << "Time in Force: " << order["time_in_force"].asString() << std::endl;
                std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            }
        }
    }
}

void DeribitAPI::showOrders(const std::string& instrument_name) {
    std::string url = BASE_URL + "/api/v2/private/get_open_orders_by_instrument";

    Json::Value payload;
    payload["instrument_name"] = instrument_name;
    payload["type"] = "all";

    std::string response = sendPostRequest(url, payload, access_token);
    
    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::istringstream s(response);
    std::string errs;
    if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
        if (jsonResponse.isMember("result")) {
            const Json::Value& orders = jsonResponse["result"];
            if (orders.size() == 0) {
                std::cout << "\n📝 No Open Orders" << std::endl;
                return;
            }

            std::cout << "\n📝 Open Orders" << std::endl;
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            for (const auto& order : orders) {
                std::cout << "Order ID: " << order["order_id"].asString() << std::endl;
                std::cout << "Price: $" << order["price"].asString() << std::endl;
                std::cout << "Amount: " << order["amount"].asString() << " USD" << std::endl;
                std::cout << "Direction: " << order["direction"].asString() << std::endl;
                std::cout << "Type: " << order["order_type"].asString() << std::endl;
                std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            }
        }
    }
}

void DeribitAPI::cancelOrder(const std::string& order_id) {
    std::string url = BASE_URL + "/api/v2/private/cancel";

    Json::Value payload;
    payload["order_id"] = order_id;

    std::string response = sendPostRequest(url, payload, access_token);
    
    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::istringstream s(response);
    std::string errs;
    if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
        if (jsonResponse.isMember("error")) {
            std::cout << "\n❌ Cancel Order Failed" << std::endl;
            std::cout << "Error: " << jsonResponse["error"]["message"].asString() << std::endl;
        } else if (jsonResponse.isMember("result")) {
            std::cout << "\n✅ Order Cancelled Successfully" << std::endl;
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            std::cout << "Order ID: " << jsonResponse["result"]["order_id"].asString() << std::endl;
            std::cout << "State: " << jsonResponse["result"]["state"].asString() << std::endl;
        }
    }
}

void DeribitAPI::modifyOrder(const std::string& order_id, double amount, double price) {
    std::string url = BASE_URL + "/api/v2/private/edit";

    // Ensure amount is a multiple of 10 USD
    double usd_amount = std::floor(amount / 10.0) * 10.0;

    Json::Value payload;
    payload["order_id"] = order_id;
    payload["amount"] = usd_amount;
    payload["price"] = price;
    payload["post_only"] = false;
    payload["reduce_only"] = false;

    std::string response = sendPostRequest(url, payload, access_token);
    
    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::istringstream s(response);
    std::string errs;
    if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
        if (jsonResponse.isMember("error")) {
            std::cout << "\n❌ Modify Order Failed" << std::endl;
            std::cout << "Error: " << jsonResponse["error"]["message"].asString() << std::endl;
        } else if (jsonResponse.isMember("result")) {
            std::cout << "\n✅ Order Modified Successfully" << std::endl;
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            std::cout << "Order ID: " << jsonResponse["result"]["order_id"].asString() << std::endl;
            std::cout << "New Price: $" << jsonResponse["result"]["price"].asString() << std::endl;
            std::cout << "New Amount: " << jsonResponse["result"]["amount"].asString() << " USD" << std::endl;
        }
    }
}

void DeribitAPI::getOrderbook(const std::string& instrument_name) {
    std::string url = BASE_URL + "/api/v2/public/get_order_book";

    Json::Value payload;
    payload["jsonrpc"] = "2.0";
    payload["method"] = "public/get_order_book";
    payload["id"] = 8066;
    
    Json::Value params;
    params["instrument_name"] = instrument_name;
    params["depth"] = 5;
    payload["params"] = params;

    std::string response = sendPostRequest(url, payload, "");

    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::istringstream s(response);
    std::string errs;
    if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
        if (jsonResponse.isMember("result")) {
            const Json::Value& result = jsonResponse["result"];
            
            std::cout << "\n📊 Orderbook for " << instrument_name << std::endl;
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            
            // Display Asks (Sell Orders)
            std::cout << "🔴 SELLS (Asks):" << std::endl;
            const Json::Value& asks = result["asks"];
            for (int i = 0; i < std::min(5, (int)asks.size()); i++) {
                double price = asks[i][0].asDouble();
                double amount = asks[i][1].asDouble();
                std::cout << std::fixed << std::setprecision(2)
                         << "  $" << std::setw(10) << price 
                         << " | Amount: " << std::setw(10) << amount << " USD" << std::endl;
            }
            
            std::cout << "\n🟢 BUYS (Bids):" << std::endl;
            const Json::Value& bids = result["bids"];
            for (int i = 0; i < std::min(5, (int)bids.size()); i++) {
                double price = bids[i][0].asDouble();
                double amount = bids[i][1].asDouble();
                std::cout << std::fixed << std::setprecision(2)
                         << "  $" << std::setw(10) << price 
                         << " | Amount: " << std::setw(10) << amount << " USD" << std::endl;
            }
            
            // Show spread
            if (!asks.empty() && !bids.empty()) {
                double bestAsk = asks[0][0].asDouble();
                double bestBid = bids[0][0].asDouble();
                double spread = bestAsk - bestBid;
                std::cout << "\n📈 Spread: $" << std::fixed << std::setprecision(2) << spread << std::endl;
            }
            
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        } else if (jsonResponse.isMember("error")) {
            std::cout << "❌ Error: " << jsonResponse["error"]["message"].asString() << std::endl;
        }
    } else {
        std::cerr << "Error parsing response: " << errs << std::endl;
    }
}

void DeribitAPI::viewCurrentPositions(const std::string& currency) {
    std::string url = BASE_URL + "/api/v2/private/get_positions";

    Json::Value payload;
    payload["currency"] = currency;
    payload["kind"] = "future";

    std::string response = sendPostRequest(url, payload, access_token);

    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::istringstream s(response);
    std::string errs;
    if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
        if (jsonResponse.isMember("result")) {
            const Json::Value& positions = jsonResponse["result"];
            
            if (positions.size() == 0) {
                std::cout << "\n📊 No Open Positions for " << currency << std::endl;
                return;
            }

            std::cout << "\n📊 Current Positions for " << currency << std::endl;
            std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;

            for (const auto& position : positions) {
                std::cout << "🔹 Position Details:" << std::endl;
                
                // Instrument
                if (position.isMember("instrument_name"))
                    std::cout << "Instrument: " << position["instrument_name"].asString() << std::endl;
                
                // Direction and Size
                if (position.isMember("direction") && position.isMember("size")) {
                    std::string direction = position["direction"].asString();
                    double size = position["size"].asDouble();
                    std::cout << "Direction: " << (direction == "buy" ? "🟢 Long" : "🔴 Short") << std::endl;
                    std::cout << "Size: " << std::fixed << std::setprecision(8) << std::abs(size) << " " << currency << std::endl;
                }
                
                // Entry Price
                if (position.isMember("average_price"))
                    std::cout << "Entry Price: $" << std::fixed << std::setprecision(2) 
                             << position["average_price"].asDouble() << std::endl;
                
                // Current Price
                if (position.isMember("mark_price"))
                    std::cout << "Mark Price: $" << std::fixed << std::setprecision(2) 
                             << position["mark_price"].asDouble() << std::endl;
                
                // PnL
                if (position.isMember("total_profit_loss")) {
                    double pnl = position["total_profit_loss"].asDouble();
                    std::string pnlColor = pnl >= 0 ? "🟢" : "🔴";
                    std::cout << "PnL: " << pnlColor << " " << std::fixed << std::setprecision(8) 
                             << pnl << " " << currency << std::endl;
                }
                
                // Liquidation Price
                if (position.isMember("estimated_liquidation_price")) {
                    double liq_price = position["estimated_liquidation_price"].asDouble();
                    if (liq_price > 0) {
                        std::cout << "Liquidation Price: $" << std::fixed << std::setprecision(2) 
                                 << liq_price << std::endl;
                    }
                }

                std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
            }
        } else if (jsonResponse.isMember("error")) {
            std::cout << "\n❌ Error fetching positions" << std::endl;
            std::cout << "Error: " << jsonResponse["error"]["message"].asString() << std::endl;
        }
    } else {
        std::cerr << "Error parsing response: " << errs << std::endl;
    }
}

// ... [Previous authentication and other methods remain the same, just move them into class methods] 