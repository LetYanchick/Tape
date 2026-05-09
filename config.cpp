#include "config.h"
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <chrono>

AppConfig loadConfig(const std::string& filename) {
    AppConfig cfg;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config");
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') 
            continue;
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            if (key == "read_delay")       
                cfg.delays.read_delay = std::chrono::microseconds(std::stoull(value));
            else if (key == "write_delay") 
                cfg.delays.write_delay = std::chrono::microseconds(std::stoull(value));
            else if (key == "move_delay")  
                cfg.delays.move_delay = std::chrono::microseconds(std::stoull(value));
            else if (key == "rewind_delay")
                cfg.delays.rewind_delay = std::chrono::microseconds(std::stoull(value));
            else if (key == "memory_limit")
                cfg.memory_limit_bytes = std::stoull(value);
            else if (key == "merge_degree")
                cfg.merge_degree = std::stoull(value);
        }
    }
    return cfg;
}