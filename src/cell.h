#pragma once

#include "raylib.h"

enum class CellType
{
    Empty = 0,
    Sand,
    Stone,
    Water,
};

struct Cell
{
    CellType type = CellType::Empty;
    Color colour = BLANK;
};
