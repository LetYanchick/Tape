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
        tape.write(i);
    // позиция 5, size=5, isEnd

    tape.rewind(0);
    EXPECT_EQ(tape.position(), 0);
    EXPECT_EQ(tape.read(), 1);

    // два шага вперёд (после read мы уже на 1, двигаем ещё на 2 -> 3)
    tape.moveForward();
    tape.moveForward();
    EXPECT_EQ(tape.position(), 3);
    // Теперь читаем (с позиции 3)
    EXPECT_EQ(tape.read(), 4);   // на позиции 3 было 4? Уточним: после записи элементы: [1,2,3,4,5]. Позиция 0:1, 1:2, 2:3, 3:4. Значит, на позиции 3 – 4. read() возвращает 4 и перемещает на 4.
    // После read позиция 4 (читали 4, потом ++current_pos)
    // Двинем назад на 1 (на позицию 3) и прочитаем снова 4
    tape.moveBackward();
    EXPECT_EQ(tape.read(), 4);   // опять 4
    // Перемотка в середину
    tape.rewind(2);
    EXPECT_EQ(tape.read(), 3);   // позиция 2 -> 3

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
    tape.rewind(0);               // ← возвращаем головку в начало
    EXPECT_EQ(tape.read(), 7);    // теперь читаем первое (и единственное) число
    EXPECT_THROW(tape.read(), std::out_of_range); // повторное чтение вызовет исключение
    std::remove(fn.c_str());
}