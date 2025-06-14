#pragma once

#include "utils/colour.hpp"
#include "utils/point.hpp"

enum class CellType
{
    Empty = 0,
    Sand,
    Stone,
    Wood,
    Water,
    Fire,
    Smoke,
};

struct Cell
{
    CellType type = CellType::Empty;
    Colour colour = Colour::Blank;
    Point velocity;
    float life_time = -1; // seconds

    constexpr Cell() = default;
    constexpr Cell(CellType type, Colour colour, float life_time = -1) : type(type), colour(colour), life_time(life_time) { }

    const static Cell Empty;
    const static Cell Sand;
    const static Cell Stone;
    const static Cell Wood;
    const static Cell Water;
    const static Cell Fire;
    const static Cell Smoke;
};

constexpr Cell Cell::Empty  = Cell(CellType::Empty, Colour::Blank);
constexpr Cell Cell::Sand   = Cell(CellType::Sand, Colour::Yellow);
constexpr Cell Cell::Stone  = Cell(CellType::Stone, Colour::DarkGrey);
constexpr Cell Cell::Wood   = Cell(CellType::Wood, Colour::Brown);
constexpr Cell Cell::Water  = Cell(CellType::Water, Colour::SkyBlue);
constexpr Cell Cell::Fire   = Cell(CellType::Fire, Colour::Orange);
constexpr Cell Cell::Smoke  = Cell(CellType::Smoke, Colour::LightGrey, 3);
