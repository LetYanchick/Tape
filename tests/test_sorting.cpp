#include <gtest/gtest.h>
#include <algorithm>
#include <cstdio>
#include <chrono>
#include <vector>
#include <string>
#include <filesystem>
#include "sorting.h"
#include "FileTape.h"

namespace fs = std::filesystem;

static DelayConfig no_delays() {
    return {
        std::chrono::microseconds(0),
        std::chrono::microseconds(0),
        std::chrono::microseconds(0),
        std::chrono::microseconds(0)
    };
}

// Фабрики для тестов: одна создаёт, другая открывает
static auto makeCreateFactory(const DelayConfig& d = no_delays()) {
    return [d](const std::string& name) -> std::unique_ptr<Tape> {
        return std::make_unique<FileTape>(name, true, d);
    };
}
static auto makeOpenFactory(const DelayConfig& d = no_delays()) {
    return [d](const std::string& name) -> std::unique_ptr<Tape> {
        return std::make_unique<FileTape>(name, false, d);
    };
}

// Вспомогательная функция для считывания всех чисел из ленты в вектор
static std::vector<int> readAll(Tape& tape) {
    std::vector<int> result;
    tape.rewind(0);
    while (!tape.isEnd()) {
        result.push_back(tape.read());
    }
    return result;
}

// Тест: сортировка небольшого обратного массива
TEST(TapeSorterTest, SortSmallReverse) {
    const std::string inFile = "test_in1.dat";
    const std::string outFile = "test_out1.dat";
    const std::string tmpDir = "tmp_test1";
    std::remove(inFile.c_str());
    std::remove(outFile.c_str());
    fs::remove_all(tmpDir);

    // Готовим входную ленту
    {
        FileTape input(inFile, true, no_delays());
        std::vector<int> data = {5, 3, 8, 1, 9, 2, 7, 4, 6, 0};
        for (int v : data) input.write(v);
    }

    {
        FileTape input(inFile, false, no_delays());
        FileTape output(outFile, true, no_delays());

        TapeSorter::sort(input, output,
                         makeCreateFactory(), makeOpenFactory(),
                         /*memory_elements*/ 3,
                         /*merge_degree*/ 2,
                         tmpDir);
    }

    // Проверяем выходную ленту
    FileTape result(outFile, false, no_delays());
    auto sorted = readAll(result);
    ASSERT_EQ(sorted.size(), 10);
    EXPECT_TRUE(std::is_sorted(sorted.begin(), sorted.end()));
    std::vector<int> expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    EXPECT_EQ(sorted, expected);

    // Очистка
    std::remove(inFile.c_str());
    std::remove(outFile.c_str());
    fs::remove_all(tmpDir);
}

// Тест: пустой вход
TEST(TapeSorterTest, SortEmptyInput) {
    const std::string inFile = "test_in2.dat";
    const std::string outFile = "test_out2.dat";
    const std::string tmpDir = "tmp_test2";
    std::remove(inFile.c_str());
    std::remove(outFile.c_str());
    fs::remove_all(tmpDir);

    // Создаём пустую входную ленту
    {
        FileTape input(inFile, true, no_delays());
    }

    {
        FileTape input(inFile, false, no_delays());
        FileTape output(outFile, true, no_delays());

        TapeSorter::sort(input, output,
                         makeCreateFactory(), makeOpenFactory(),
                         10, 4, tmpDir);
    }

    FileTape result(outFile, false, no_delays());
    auto sorted = readAll(result);
    EXPECT_TRUE(sorted.empty());

    std::remove(inFile.c_str());
    std::remove(outFile.c_str());
    fs::remove_all(tmpDir);
}

// Тест: один элемент
TEST(TapeSorterTest, SortSingleElement) {
    const std::string inFile = "test_in3.dat";
    const std::string outFile = "test_out3.dat";
    const std::string tmpDir = "tmp_test3";
    std::remove(inFile.c_str());
    std::remove(outFile.c_str());
    fs::remove_all(tmpDir);

    {
        FileTape input(inFile, true, no_delays());
        input.write(42);
    }

    {
        FileTape input(inFile, false, no_delays());
        FileTape output(outFile, true, no_delays());

        TapeSorter::sort(input, output,
                         makeCreateFactory(), makeOpenFactory(),
                         5, 2, tmpDir);
    }

    FileTape result(outFile, false, no_delays());
    auto sorted = readAll(result);
    ASSERT_EQ(sorted.size(), 1);
    EXPECT_EQ(sorted[0], 42);

    std::remove(inFile.c_str());
    std::remove(outFile.c_str());
    fs::remove_all(tmpDir);
}

// Тест: уже отсортированный массив
TEST(TapeSorterTest, SortAlreadySorted) {
    const std::string inFile = "test_in4.dat";
    const std::string outFile = "test_out4.dat";
    const std::string tmpDir = "tmp_test4";
    std::remove(inFile.c_str());
    std::remove(outFile.c_str());
    fs::remove_all(tmpDir);

    {
        FileTape input(inFile, true, no_delays());
        for (int i = 1; i <= 10; ++i) input.write(i);
    }

    {
        FileTape input(inFile, false, no_delays());
        FileTape output(outFile, true, no_delays());

        TapeSorter::sort(input, output,
                         makeCreateFactory(), makeOpenFactory(),
                         3, 2, tmpDir);
    }

    FileTape result(outFile, false, no_delays());
    auto sorted = readAll(result);
    ASSERT_EQ(sorted.size(), 10);
    EXPECT_TRUE(std::is_sorted(sorted.begin(), sorted.end()));

    std::remove(inFile.c_str());
    std::remove(outFile.c_str());
    fs::remove_all(tmpDir);
}