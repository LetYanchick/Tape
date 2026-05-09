#pragma once
#include <string>
#include "Tape.h"
#include <memory>
#include <functional>
#include <vector>


class TapeSorter {
public:
    using TapeFactory = std::function<std::unique_ptr<Tape>(const std::string&)>; // это функция, принимающая имя файла и возвращающая ленту, можно использовать с любой реализацией Tape

    static void sort(Tape& input, Tape& output,
        const TapeFactory& openFactory,    // открыть существующий
        const TapeFactory& createFactory,  // создать новый
        std::size_t memory_elements, // размер на блок
        std::size_t merge_degree, //k
        const std::string& temp_dir);
};