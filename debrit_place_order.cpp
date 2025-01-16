#include <iostream>
#include <string>
#include <curl/curl.h>
#include <json/json.h>

// Deribit API base URL
const std::string BASE_URL = "https://test.deribit.com";

// API credentials
const std::string CLIENT_ID = "98khmWgl";
const std::string CLIENT_SECRET = "TB1_-D1qLrO33sTdiJgTUQIJRXBQyIQIFEkDL7Hd16Q";

// Function to handle CURL response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to authenticate and get access token
std::string authenticate() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    // Updated endpoint to match Deribit's API specification
    std::string url = BASE_URL + "/api/v2/public/auth";

    // Updated JSON payload to match Deribit's expected format
    Json::Value jsonPayload;
    jsonPayload["jsonrpc"] = "2.0";
    jsonPayload["id"] = "authentication";
    jsonPayload["method"] = "public/auth";
    jsonPayload["params"] = Json::Value(Json::objectValue);
    jsonPayload["params"]["grant_type"] = "client_credentials";
    jsonPayload["params"]["client_id"] = CLIENT_ID;
    jsonPayload["params"]["client_secret"] = CLIENT_SECRET;

    Json::StreamWriterBuilder writer;
    std::string payload = Json::writeString(writer, jsonPayload);

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            throw std::runtime_error("CURL error: " + std::string(curl_easy_strerror(res)));
        }
    } else {
        throw std::runtime_error("Failed to initialize CURL.");
    }

    // Parse the response
    Json::Reader reader;
    Json::Value jsonResponse;
    reader.parse(readBuffer, jsonResponse);

    if (jsonResponse.isMember("result") && jsonResponse["result"].isMember("access_token")) {
        return jsonResponse["result"]["access_token"].asString();
    } else {
        throw std::runtime_error("Authentication failed: " + readBuffer);
    }
}

// Function to place a BTC order
void place_btc_order(const std::string& access_token) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    std::string url = BASE_URL + "/api/v2/private/buy";

    // Create order parameters
    Json::Value jsonPayload;
    jsonPayload["jsonrpc"] = "2.0";
    jsonPayload["id"] = "place_order";
    jsonPayload["method"] = "private/buy";
    jsonPayload["params"] = Json::Value(Json::objectValue);
    jsonPayload["params"]["instrument_name"] = "BTC-PERPETUAL";  // or "BTC-PERPETUAL" for futures
    jsonPayload["params"]["amount"] = 100;  // Amount in USD for BTC-PERPETUAL
    jsonPayload["params"]["type"] = "limit";  // or "market" for market orders
    jsonPayload["params"]["price"] = 35000;  // Price in USD (remove for market orders)
    jsonPayload["params"]["post_only"] = false;
    jsonPayload["params"]["reduce_only"] = false;

    Json::StreamWriterBuilder writer;
    std::string payload = Json::writeString(writer, jsonPayload);

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string auth_header = "Authorization: Bearer " + access_token;
        headers = curl_slist_append(headers, auth_header.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            throw std::runtime_error("CURL error: " + std::string(curl_easy_strerror(res)));
        }

        std::cout << "Order response: " << readBuffer << std::endl;
    }
}

// Update the sendPostRequest function to properly format the JSON-RPC request
std::string sendPostRequest(const std::string& url, const Json::Value& payload, const std::string& access_token) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    Json::Value jsonPayload;
    jsonPayload["jsonrpc"] = "2.0";
    jsonPayload["id"] = "request";
    // Extract the method name correctly from the URL
    std::string method = "private/";
    size_t lastSlash = url.find_last_of('/');
    if (lastSlash != std::string::npos) {
        method += url.substr(lastSlash + 1);
    }
    jsonPayload["method"] = method;
    jsonPayload["params"] = payload;

    Json::StreamWriterBuilder writer;
    std::string payloadStr = Json::writeString(writer, jsonPayload);

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string auth_header = "Authorization: Bearer " + access_token;
        headers = curl_slist_append(headers, auth_header.c_str());

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

void cancelOrder(const std::string& order_id, const std::string& access_token) {
    std::string url = BASE_URL + "/api/v2/private/cancel";

    Json::Value payload;
    payload["order_id"] = order_id;
    // Add required fields for Deribit API
    payload["instrument_name"] = "BTC-PERPETUAL";  // Add the instrument name

    std::string response = sendPostRequest(url, payload, access_token);

    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::istringstream s(response);
    std::string errs;
    if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
        // Check for errors in the response
        if (jsonResponse.isMember("error")) {
            std::cerr << "Error canceling order: " << jsonResponse["error"]["message"].asString() << std::endl;
        } else if (jsonResponse.isMember("result")) {
            std::cout << "Order canceled successfully: " << jsonResponse["result"].toStyledString() << std::endl;
        }
    } else {
        std::cerr << "Error parsing response: " << errs << std::endl;
    }
}

void showOrders(const std::string& instrument_name, const std::string& access_token) {
    std::string url = BASE_URL + "/api/v2/private/get_open_orders_by_instrument";

    Json::Value payload;
    payload["instrument_name"] = instrument_name;
    payload["type"] = "all";  // Add type parameter to show all orders

    std::string response = sendPostRequest(url, payload, access_token);

    Json::CharReaderBuilder reader;
    Json::Value jsonResponse;
    std::istringstream s(response);
    std::string errs;
    if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
        if (jsonResponse.isMember("error")) {
            std::cerr << "Error getting orders: " << jsonResponse["error"]["message"].asString() << std::endl;
        } else if (jsonResponse.isMember("result")) {
            std::cout << "Open orders:" << std::endl;
            const Json::Value& orders = jsonResponse["result"];
            for (const auto& order : orders) {
                std::cout << "Order ID: " << order["order_id"].asString() << std::endl;
                std::cout << "Price: " << order["price"].asString() << std::endl;
                std::cout << "Amount: " << order["amount"].asString() << std::endl;
                std::cout << "Direction: " << order["direction"].asString() << std::endl;
                std::cout << "-------------------" << std::endl;
            }
        }
    } else {
        std::cerr << "Error parsing response: " << errs << std::endl;
    }
}

// Update main function to include user interaction
int main() {
    try {
        // Authenticate and get access token
        std::string token = authenticate();
        std::cout << "Authenticated successfully.\n" << std::endl;

        while (true) {
            std::cout << "\nDeribit Trading Menu:" << std::endl;
            std::cout << "1. Place BTC Order" << std::endl;
            std::cout << "2. Show Open Orders" << std::endl;
            std::cout << "3. Cancel Order" << std::endl;
            std::cout << "4. Exit" << std::endl;
            std::cout << "Enter your choice (1-4): ";

            int choice;
            std::cin >> choice;

            switch (choice) {
                case 1: {
                    place_btc_order(token);
                    break;
                }
                case 2: {
                    showOrders("BTC-PERPETUAL", token);
                    break;
                }
                case 3: {
                    std::string order_id;
                    std::cout << "Enter order ID to cancel: ";
                    std::cin >> order_id;
                    cancelOrder(order_id, token);
                    break;
                }
                case 4: {
                    std::cout << "Exiting program..." << std::endl;
                    return 0;
                }
                default: {
                    std::cout << "Invalid choice. Please try again." << std::endl;
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}