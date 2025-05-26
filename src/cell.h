#pragma once

#include <raylib.h>

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
    Color colour = BLANK;
};
