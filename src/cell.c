#include "cell.h"

#include <stdlib.h>

#include "grid.h"
#include "raylib.h"

// === private ===

int random_dir()
{
    int directions[2] = { -1, 1 };   
    int index = random() % 2;

    return directions[index];
}

// ===============

bool move_cell_up(CellChunk* chunk, size_t x, size_t y, const Cell* cell)
{
    bool can_up = is_empty_in_chunk(chunk, x, y - 1);
    
    if (can_up)
    {
        set_cell_in_chunk(chunk, x, y - 1, cell);
    }

    return can_up;
}

bool move_cell_down(CellChunk* chunk, size_t x, size_t y, const Cell* cell)
{
    bool can_down = is_empty_in_chunk(chunk, x, y + 1);
    
    if (can_down)
    {
        set_cell_in_chunk(chunk, x, y + 1, cell);
    }

    return can_down;
}

bool move_cell_sideways(CellChunk* chunk, size_t x, size_t y, const Cell* cell)
{
    int dir = random_dir();

    if (is_empty_in_chunk(chunk, x + dir, y))
    {
        return set_cell_in_chunk(chunk, x + dir, y, cell);
    }

    return false;
}

bool move_cell_up_diagonal(CellChunk* chunk, size_t x, size_t y, const Cell* cell)
{
    int dir = random_dir();

    if (is_empty_in_chunk(chunk, x + dir, y - 1))
    {
        return set_cell_in_chunk(chunk, x + dir, y - 1, cell);
    }

    return false;
}

bool move_cell_down_diagonal(CellChunk* chunk, size_t x, size_t y, const Cell* cell)
{
    int dir = random_dir();

    if (is_empty_in_chunk(chunk, x + dir, y + 1))
    {
        return set_cell_in_chunk(chunk, x + dir, y + 1, cell);
    }

    return false;
}

void draw_cell(const CellChunk* chunk, size_t x, size_t y, Color colour)
{
    DrawRectangle(x * chunk->cell_size, y * chunk->cell_size, chunk->cell_size, chunk->cell_size, colour);
}