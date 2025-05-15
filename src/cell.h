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
    Cell() : cell_type(CellType::Empty), colour(BLANK) { }
    Cell(CellType cell_type, Color colour) : cell_type(cell_type), colour(colour) {}

    CellType cell_type ;
    Color colour;
};