#ifndef DERIBIT_API_H
#define DERIBIT_API_H

#include <string>
#include <json/json.h>

class DeribitAPI {
private:
    const std::string BASE_URL;
    const std::string CLIENT_ID;
    const std::string CLIENT_SECRET;
    std::string access_token;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    std::string sendPostRequest(const std::string& url, const Json::Value& payload, const std::string& access_token);

public:
    DeribitAPI(const std::string& client_id, const std::string& client_secret, 
               const std::string& base_url = "https://test.deribit.com");
    
    bool authenticate();
    void place_btc_order();
    void showOrders(const std::string& instrument_name);
    void cancelOrder(const std::string& order_id);
    void modifyOrder(const std::string& order_id, double amount, double price);
    void getOrderbook(const std::string& instrument_name);
    void viewCurrentPositions(const std::string& currency);
};

#endif // DERIBIT_API_H 