#include <iostream>
#include <memory>
#include <random>
#include <filesystem>
#include "config.h"
#include "FileTape.h"
#include "sorting.h"
#include <typeinfo>


// Создаёт входную ленту, заполненную случайными числами (если её нет)
void generateInput(const std::string& filename, std::size_t count, const DelayConfig& delays) {
    FileTape tape(filename,true, delays);
    std::mt19937 rng(42);  
    std::uniform_int_distribution<int> dist;
    for (std::size_t i = 0; i < count; ++i) {
        tape.write(dist(rng));
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 3) {
            std::cerr << "Parameters should be: "
                << " <input_file> <output_file> [config_file] [memory_limit] [merge_degree]\n";
            return 1;
        }
        std::string input_file = argv[1];
        std::string output_file = argv[2];
        std::string config_file = (argc > 3) ? argv[3] : "config.ini";

        AppConfig config = loadConfig(config_file);


        if (argc > 4) 
            config.memory_limit_bytes = std::stoull(argv[4]);
        if (argc > 5) 
            config.merge_degree = std::stoull(argv[5]);

        std::filesystem::create_directories("tmp");
        
        if (!std::filesystem::exists(input_file)) {
            generateInput(input_file, 100, config.delays);
        }



        FileTape input(input_file,false, config.delays);
        FileTape output(output_file,true, config.delays);

        // Пересчитываем байты в количество int'ов
        std::size_t memory_elements = config.memory_limit_bytes / sizeof(int);
        if (memory_elements == 0) 
            memory_elements = 1; // защита от 0

        std::cout << "M=" << memory_elements << " elements, K=" << config.merge_degree << "\n";

        auto start = std::chrono::high_resolution_clock::now();
        auto openFactory = [&config](const std::string& name) -> std::unique_ptr<Tape> {
            return std::make_unique<FileTape>(name, false, config.delays);
            };
        auto createFactory = [&config](const std::string& name) -> std::unique_ptr<Tape> {
            return std::make_unique<FileTape>(name, true, config.delays);
            };
        TapeSorter::sort(input, output, createFactory, openFactory, memory_elements, config.merge_degree, "tmp");
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        {
            FileTape check(output_file,false, config.delays);
            check.rewind(0);
            bool sorted = true;
            int prev = 0;
            if (!check.isEnd()) {
                prev = check.read();
                while (!check.isEnd()) {
                    int curr = check.read();
                    if (prev > curr) {
                        sorted = false;
                        break;
                    }
                    prev = curr;
                }
            }
            std::cout << "Sorting " << (sorted ? "succeded" : "FAILED") << "\n";
        }

        std::cout << "Elapsed: " << elapsed << " ms\n";

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}