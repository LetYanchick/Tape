#include "sorting.h"
#include <algorithm>
#include <queue>
#include <filesystem>
#include <stdexcept>
#include <iostream>


struct MergeElement {
    int value;
    std::size_t tape_index;
    bool operator>(const MergeElement& other) const {
        return value > other.value;
    }
};

void TapeSorter::sort(Tape& input, Tape& output,
    const TapeFactory& createFactory,
    const TapeFactory& openFactory,
    std::size_t memory_elements,
    std::size_t merge_degree,
    const std::string& temp_dir)
{
    if (memory_elements == 0)
        throw std::invalid_argument("memory_elements cannot be 0"); //

    std::filesystem::create_directories(temp_dir);

    std::vector<std::string> run_files;
    std::vector<int> buffer;
    buffer.reserve(memory_elements);
    int run_id = 0;

    while (!input.isEnd()) {
        buffer.clear();
        // Читаем memory_elements элементов или сколько осталось
        std::size_t count = 0;
        while (!input.isEnd() && count < memory_elements) {
            buffer.push_back(input.read());
            ++count;
        }
        std::sort(buffer.begin(), buffer.end());

        std::string run_filename = (std::filesystem::path(temp_dir) / ("run_" + std::to_string(run_id++) + ".dat")).string();

        auto tape = createFactory(run_filename); // создание ленты
        for (int v : buffer) {
            tape->write(v);
        };
        run_files.push_back(run_filename); // имя ленты
    }

     std::function<void(std::vector<std::string>, Tape&)> merge_tapes = [&](std::vector<std::string> tape_files, Tape& target) {
        if (tape_files.empty()) 
            return;
        if (tape_files.size() == 1) {
            // база
            auto src = openFactory(tape_files.front());
            src->rewind(0);
            while (!src->isEnd()) {
                target.write(src->read());
            }
            return;
        }

        if (tape_files.size() <= merge_degree) {
            std::vector<std::unique_ptr<Tape>> tapes;
            for (auto& f : tape_files) {
                tapes.push_back(openFactory(f));
            }
            for (auto& t : tapes) 
                t->rewind(0);

            // Min-heap, наименьший элемент наверху
            std::priority_queue<MergeElement, std::vector<MergeElement>, std::greater<MergeElement>> pq;
            // Кладём в кучу первые элементы всех лент
            for (std::size_t i = 0; i < tapes.size(); ++i) {
                if (!tapes[i]->isEnd()) {
                    pq.push({ tapes[i]->read(), i }); //{значение, номер ленты}
                }
            }

            while (!pq.empty()) {
                auto min = pq.top(); // берём минимум
                pq.pop(); //удаляем 
                target.write(min.value); // пишем в выход
                if (!tapes[min.tape_index]->isEnd()) {
                    pq.push({ tapes[min.tape_index]->read(), min.tape_index }); // добавляем следующий элемент с той же ленты
                }
            }
        }
        else {
            // Если временных лент больше, чем merge_degree,
            // разбиваем их на группы и каждая группа - это промежуточная лента, а потом эти промежуточные обрабатываются рекурсивно
            std::vector<std::string> next_level;
            for (std::size_t i = 0; i < tape_files.size(); i += merge_degree) {
                auto end = std::min(i + merge_degree, tape_files.size());
                std::vector<std::string> group(tape_files.begin() + i, tape_files.begin() + end);
                auto merged_file = (std::filesystem::path(temp_dir) / ("merged_" + std::to_string(i / merge_degree) + ".dat")).string();
                auto merged_tape = createFactory(merged_file);
                merge_tapes(group, *merged_tape); // рекурсия
                next_level.push_back(merged_file);
            }
            merge_tapes(next_level, target); 
        }
    };

    merge_tapes(run_files, output);
}