#include <gtest/gtest.h>
#include <cstdio>
#include <chrono>
#include <fstream>
#include "FileTape.h"

static DelayConfig no_delays() {
    return {
        std::chrono::microseconds(0),
        std::chrono::microseconds(0),
        std::chrono::microseconds(0),
        std::chrono::microseconds(0)
    };
}

// Тест: создание и запись/чтение
TEST(FileTapeTest, CreateWriteRead) {
    const std::string fn = "test_tape1.dat";
    std::remove(fn.c_str());

    // Создаём ленту (truncate = true)
    {
        FileTape tape(fn, true, no_delays());
        EXPECT_EQ(tape.GetSize(), 0);
        EXPECT_TRUE(tape.isEnd());   // size=0, current_pos=0 => isEnd

        tape.write(10);
        tape.write(20);
        EXPECT_EQ(tape.GetSize(), 2);
        EXPECT_TRUE(tape.isEnd());   // после двух записей позиция 2 == size
    }

    // Открываем существующую (truncate = false)
    {
        FileTape tape(fn, false, no_delays());
        ASSERT_EQ(tape.GetSize(), 2);
        EXPECT_FALSE(tape.isEnd());  // позиция 0
        EXPECT_EQ(tape.read(), 10);
        EXPECT_EQ(tape.read(), 20);
        EXPECT_TRUE(tape.isEnd());
    }

    std::remove(fn.c_str());
}

// Тест: перемещения и перемотка
TEST(FileTapeTest, MoveAndRewind) {
    const std::string fn = "test_tape2.dat";
    std::remove(fn.c_str());

    FileTape tape(fn, true, no_delays());
    for (int i = 1; i <= 5; ++i)
        tape.write(i); // позиции от 0 до 4 и элементы [1,2,3,4,5].

    tape.rewind(0);
    EXPECT_EQ(tape.position(), 0);
    EXPECT_EQ(tape.read(), 1); // позиция теперь 1 (после чтения прибавляется)

    tape.moveForward(); // поз 2
    tape.moveForward(); // поз 3
    EXPECT_EQ(tape.position(), 3);
    EXPECT_EQ(tape.read(), 4);   // на позиции 3 записано 4, и после этого позиция + 1 (т.е. поз = 4)

    tape.moveBackward(); // поз 3
    EXPECT_EQ(tape.read(), 4);   // 4
    tape.rewind(2); 
    EXPECT_EQ(tape.read(), 3);   // на позиции 2 элемент 3

    std::remove(fn.c_str());
}

// Тест: задержки не портят данные
TEST(FileTapeTest, DelaysDontBreak) {
    const std::string fn = "test_tape3.dat";
    std::remove(fn.c_str());

    DelayConfig delays{
        std::chrono::microseconds(100),   // read
        std::chrono::microseconds(200),   // write
        std::chrono::microseconds(50),    // move
        std::chrono::microseconds(10)     // rewind
    };
    FileTape tape(fn, true, delays);
    tape.write(100);
    tape.write(200);
    tape.rewind(0);
    EXPECT_EQ(tape.read(), 100);
    EXPECT_EQ(tape.read(), 200);

    std::remove(fn.c_str());
}

// Тест: исключение при чтении за концом
TEST(FileTapeTest, ReadPastEndThrows) {
    const std::string fn = "test_tape4.dat";
    std::remove(fn.c_str());
    FileTape tape(fn, true, no_delays());
    tape.write(7);
    tape.rewind(0);               // возвращаемся в начало
    EXPECT_EQ(tape.read(), 7);    // теперь читаем первое (и единственное) число
    EXPECT_THROW(tape.read(), std::out_of_range); // повторное чтение вызовет исключение
    std::remove(fn.c_str());
}