#include "config.h"
#include <fstream>
#include <json/json.h>

// Initialize static members
std::string Config::CLIENT_ID;
std::string Config::CLIENT_SECRET;
std::string Config::BASE_URL;

bool Config::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;

    if (!Json::parseFromStream(reader, file, &root, &errs)) {
        return false;
    }

    // Load values from JSON
    CLIENT_ID = root["client_id"].asString();
    CLIENT_SECRET = root["client_secret"].asString();
    BASE_URL = root["base_url"].asString();

    return true;
} 