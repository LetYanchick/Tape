#pragma once
#include <cstddef> // для std::size_t

class Tape {
public:
    virtual ~Tape() = default;

    virtual int read() = 0;

    virtual void write(int value) = 0;

    virtual void moveForward() = 0;

    virtual void moveBackward() = 0;

    virtual void rewind(std::size_t pos) = 0;

    virtual std::size_t position() const = 0;

    virtual std::size_t GetSize() const = 0;

    virtual bool isEnd() const = 0;
};