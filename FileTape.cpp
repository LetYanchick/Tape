#include "FileTape.h"
#include <thread>
#include <stdexcept>
#include <iostream>
#include <filesystem>

FileTape::FileTape(const std::string& filename, bool truncate, const DelayConfig& delays)
    : delays(delays) {
    if (truncate) {
        // создание
        std::ofstream createFile(filename, std::ios::binary | std::ios::trunc);
        if (!createFile.is_open())
            throw std::runtime_error("Cannot create file " + filename);
        createFile.close();
        // открытие для чтения и записи
        my_file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    }
    else {
        // Открываем существующий
        my_file.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    }
    if (!my_file.is_open())
        throw std::runtime_error("Cannot open file " + filename);
    
    if (!truncate) {
        // вычисление размера
        my_file.seekg(0, std::ios::end);
        std::streampos fileSize = my_file.tellg();
        my_size = static_cast<std::size_t>(fileSize) / sizeof(int);
        my_file.seekg(0);
    }
}

FileTape::~FileTape() {
    if (my_file.is_open()) 
        my_file.close();
}

void FileTape::applyDelay(std::chrono::microseconds delay) {
    if (delay.count() > 0) {
        std::this_thread::sleep_for(delay);
    }
}

void FileTape::seekTo(std::size_t pos) { // это для синхронизации указателей чтения и записи
    my_file.seekg(pos * sizeof(int));
    my_file.seekp(pos * sizeof(int));
}

int FileTape::read() {
    if (isEnd())
        throw std::out_of_range("Read past end of tape");
    applyDelay(delays.read_delay);
    seekTo(current_pos);
    int value;
    my_file.read(reinterpret_cast<char*>(&value), sizeof(int));
    ++current_pos;
    return value;
}

void FileTape::write(int value) {
    applyDelay(delays.write_delay);
    seekTo(current_pos);
    my_file.write(reinterpret_cast<const char*>(&value), sizeof(int));
    ++current_pos;
    if (current_pos > my_size) 
        my_size = current_pos;
    my_file.flush(); // сбрасываем буфер на диск сразу, чтобы другой FileTape мог прочитать актуальные данные
}

void FileTape::moveForward() {
    if (isEnd()) 
        throw std::out_of_range("Move forward past end");
    applyDelay(delays.move_delay);
    ++current_pos;
    seekTo(current_pos);
}

void FileTape::moveBackward() {
    if (current_pos == 0) 
        throw std::out_of_range("Move backward past begin");
    applyDelay(delays.move_delay);
    --current_pos;
    seekTo(current_pos);
}

void FileTape::rewind(std::size_t position) {
    if (position > my_size) 
        throw std::out_of_range("Rewind out of range");
    std::size_t distance = (position > current_pos) ? (position - current_pos) : (current_pos - position); // чтобы была положительная
    applyDelay(delays.rewind_delay * distance);   // за каждую позицию по rewind_delay
    current_pos = position;
    seekTo(current_pos);
}

bool FileTape::isEnd() const {
    return current_pos >= my_size;
}

std::size_t FileTape::position() const {
    return current_pos;
}

std::size_t FileTape::GetSize() const {
    return my_size;
}