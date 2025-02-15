#pragma once

#include <stdint.h>

class Color
{
public:

    Color() = default;
    Color( const uint8_t r, const uint8_t g, const uint8_t b );

    uint8_t getRed() const;
    uint8_t getGreen() const;
    uint8_t getBlue() const;

    bool operator==(const Color& color) const;
    Color operator+(const Color& color) const;
    Color operator*(const uint8_t multiplier) const;
    Color operator/(const uint8_t divisor) const;

private:
    static const uint8_t maxValue;

    uint8_t red_;
    uint8_t green_;
    uint8_t blue_;
};

namespace color
{
    const Color RED( 64, 0, 0 );
    const Color GREEN( 0, 64, 0 );
    const Color BLUE( 0, 0, 64 );
    const Color YELLOW = RED + GREEN;
    const Color MAGENTA = RED + BLUE;
    const Color CYAN = GREEN + BLUE;
    const Color WHITE = RED + GREEN + BLUE;
    const Color ORANGE( 64, 32, 0 );
    const Color SPRING_GREEN( 32, 64, 0 );
    const Color TURQUOISE( 0, 64, 32 );
    const Color OCEAN( 0, 32, 64 );
    const Color VIOLET( 32, 0, 64 );
    const Color RASPBERRY( 64, 0, 32 );
}
