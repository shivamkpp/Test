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

int main() {
    try {
        // Authenticate and get access token
        std::string token = authenticate();
        std::cout << "Authenticated successfully. Access token: " << token << std::endl;

        // Place BTC order
        place_btc_order(token);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}