#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct Config {
    static std::string CLIENT_ID;
    static std::string CLIENT_SECRET;
    static std::string BASE_URL;
    
    static bool loadConfig(const std::string& filename);
};

#endif // CONFIG_H 