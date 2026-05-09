#pragma once
#include "Tape.h"
#include <fstream>
#include <string>
#include <chrono>

struct DelayConfig {
    // Задержки: по дефолту 0, а вообще берутся из конфига
    std::chrono::microseconds read_delay{ 0 };
    std::chrono::microseconds write_delay{ 0 };
    std::chrono::microseconds move_delay{ 0 };
    std::chrono::microseconds rewind_delay{ 0 };
};

class FileTape : public Tape {
public:
    FileTape(const std::string& filename, bool truncate, const DelayConfig& delays);
    ~FileTape() override;

    int read() override;
    void write(int value) override;
    void moveForward() override;
    void moveBackward() override;
    void rewind(std::size_t position) override;
    bool isEnd() const override;
    std::size_t position() const override;
    std::size_t GetSize() const override;

private:
    void applyDelay(std::chrono::microseconds delay);
    void seekTo(std::size_t pos); 

    std::fstream my_file; //файл-лента
    std::size_t my_size = 0;        //элементов в ленте
    std::size_t current_pos = 0; // индекс текущего элемента на ленте
    DelayConfig delays; 
};
