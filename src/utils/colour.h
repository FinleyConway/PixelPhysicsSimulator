#pragma once

#include <cstdint>

struct Colour
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;

    const static Colour LightGrey;
    const static Colour Grey;
    const static Colour DarkGrey;
    const static Colour Yellow;
    const static Colour Gold;
    const static Colour Orange;
    const static Colour Pink;
    const static Colour Red;
    const static Colour Maroon;
    const static Colour Green;
    const static Colour Lime;
    const static Colour DarkGreen;
    const static Colour SkyBlue;
    const static Colour Blue;
    const static Colour DarkBlue;
    const static Colour Purple;
    const static Colour Violet;
    const static Colour DarkPurple;
    const static Colour Beige;
    const static Colour Brown;
    const static Colour DarkBrown;
    const static Colour White;
    const static Colour Black;
    const static Colour Blank;
    const static Colour Magenta;
};

constexpr Colour Colour::LightGrey   = Colour(200, 200, 200, 255);
constexpr Colour Colour::Grey        = Colour(130, 130, 130, 255);
constexpr Colour Colour::DarkGrey    = Colour(80, 80, 80, 255);
constexpr Colour Colour::Yellow      = Colour(253, 249, 0, 255);
constexpr Colour Colour::Gold        = Colour(255, 203, 0, 255);
constexpr Colour Colour::Orange      = Colour(255, 161, 0, 255);
constexpr Colour Colour::Pink        = Colour(255, 109, 194, 255);
constexpr Colour Colour::Red         = Colour(230, 41, 55, 255);
constexpr Colour Colour::Maroon      = Colour(190, 33, 55, 255);
constexpr Colour Colour::Green       = Colour(0, 228, 48, 255);
constexpr Colour Colour::Lime        = Colour(0, 158, 47, 255);
constexpr Colour Colour::DarkGreen   = Colour(0, 117, 44, 255);
constexpr Colour Colour::SkyBlue     = Colour(102, 191, 255, 255);
constexpr Colour Colour::Blue        = Colour(0, 121, 241, 255);
constexpr Colour Colour::DarkBlue    = Colour(0, 82, 172, 255);
constexpr Colour Colour::Purple      = Colour(200, 122, 255, 255);
constexpr Colour Colour::Violet      = Colour(135, 60, 190, 255);
constexpr Colour Colour::DarkPurple  = Colour(112, 31, 126, 255);
constexpr Colour Colour::Beige       = Colour(211, 176, 131, 255);
constexpr Colour Colour::Brown       = Colour(127, 106, 79, 255);
constexpr Colour Colour::DarkBrown   = Colour(76, 63, 47, 255);
constexpr Colour Colour::White       = Colour(255, 255, 255, 255);
constexpr Colour Colour::Black       = Colour(0, 0, 0, 255);
constexpr Colour Colour::Blank       = Colour(0, 0, 0, 0);
constexpr Colour Colour::Magenta     = Colour(255, 0, 255, 255);