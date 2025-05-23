#pragma once

#include "raylib.h"

enum class CellType
{
    Empty = 0,
    Sand,
    Stone,
};

struct Cell
{
    CellType type = CellType::Empty;
    Color colour = BLANK;
};