#pragma once
#include "FileTape.h"   
#include <string>

struct AppConfig {
    DelayConfig delays;          
    std::size_t memory_limit_bytes = 1'048'576; // 1 МБ 
    std::size_t merge_degree = 16;    // k в k-way merge
};

AppConfig loadConfig(const std::string& filename);