#include "deribit_api.h"
#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <cmath>

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
    if (url.find("/auth") != std::string::npos) {
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
    std::cout << "Request payload: " << payloadStr << std::endl;

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
    payload["amount"] = 100;  // Minimum amount in USD
    payload["type"] = "limit";
    payload["price"] = 35000;
    payload["post_only"] = false;
    payload["reduce_only"] = false;

    std::string response = sendPostRequest(url, payload, access_token);
    std::cout << "Order response: " << response << std::endl;
}

void DeribitAPI::showOrders(const std::string& instrument_name) {
    std::string url = BASE_URL + "/api/v2/private/get_open_orders_by_instrument";

    Json::Value payload;
    payload["instrument_name"] = instrument_name;
    payload["type"] = "all";

    std::string response = sendPostRequest(url, payload, access_token);
    std::cout << "Open orders: " << response << std::endl;
}

void DeribitAPI::cancelOrder(const std::string& order_id) {
    std::string url = BASE_URL + "/api/v2/private/cancel";

    Json::Value payload;
    payload["order_id"] = order_id;

    std::string response = sendPostRequest(url, payload, access_token);
    std::cout << "Cancel response: " << response << std::endl;
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
    std::cout << "Modify response: " << response << std::endl;
}

void DeribitAPI::getOrderbook(const std::string& instrument_name) {
    std::string url = BASE_URL + "/api/v2/public/get_order_book";

    Json::Value payload;
    payload["instrument_name"] = instrument_name;
    payload["depth"] = 5;

    std::string response = sendPostRequest(url, payload, "");
    std::cout << "Orderbook: " << response << std::endl;
}

// ... [Previous authentication and other methods remain the same, just move them into class methods] 