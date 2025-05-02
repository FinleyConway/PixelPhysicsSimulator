#include "cell.h"

#include <stdlib.h>

#include "raylib.h"

#include "grid.h"

// === private ===

int random_dir()
{
    int directions[2] = { -1, 1 };   
    int index = random() % 2;

    return directions[index];
}

// ===============

bool move_cell_up(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell)
{
    unsigned int movement_amount = 0;

    for (unsigned int i = 1; i <= cell->velocity_y; i++)
    {
        if (is_empty_in_chunk(chunk, x, y - i))
        {
            movement_amount++;
        }
        else break;
    }

    if (movement_amount > 0)
    {
        return set_cell_in_chunk(chunk, x, y - movement_amount, cell);
    }

    return false;
}

bool move_cell_down(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell)
{
    unsigned int movement_amount = 0;

    for (unsigned int i = 1; i <= cell->velocity_y; i++)
    {
        if (is_empty_in_chunk(chunk, x, y + i))
        {
            movement_amount++;
        }
        else break;
    }

    if (movement_amount > 0)
    {
        return set_cell_in_chunk(chunk, x, y + movement_amount, cell);
    }

    return false;
}

bool move_cell_sideways(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell)
{
    int dir = random_dir();
    unsigned int movement_amount = 0;

    for (unsigned int i = 1; i <= cell->velocity_x; i++)
    {
        if (is_empty_in_chunk(chunk, x + dir * i, y))
        {
            movement_amount++;
        }
        else break;
    }

    if (movement_amount > 0)
    {
        return set_cell_in_chunk(chunk, x + dir * movement_amount, y, cell);
    }

    return false;
}

bool move_cell_up_diagonal(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell)
{
    int dir = random_dir(); 
    unsigned int max_steps = cell->velocity_x < cell->velocity_y ? cell->velocity_x : cell->velocity_y;
    unsigned int movement_amount = 0;

    for (unsigned int i = 1; i <= max_steps; i++)
    {
        if (is_empty_in_chunk(chunk, x + dir * i, y - i))
        {
            movement_amount++;
        }
        else break;
    }

    if (movement_amount > 0)
    {
        return set_cell_in_chunk(chunk, x + dir * movement_amount, y - movement_amount, cell);
    }

    return false;
}

bool move_cell_down_diagonal(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell)
{
    int dir = random_dir();
    unsigned int max_steps = cell->velocity_x < cell->velocity_y ? cell->velocity_x : cell->velocity_y;
    unsigned int movement_amount = 0;

    for (unsigned int i = 1; i <= max_steps; i++)
    {
        if (is_empty_in_chunk(chunk, x + dir * i, y + i))
        {
            movement_amount++;
        }
        else break;
    }

    if (movement_amount > 0)
    {
        return set_cell_in_chunk(chunk, x + dir * movement_amount, y + movement_amount, cell);
    }

    return false;
}

void draw_cell(unsigned int x, unsigned int y, Color colour)
{
    DrawRectangle(x * CHUNK_CELL_SIZE, y * CHUNK_CELL_SIZE, CHUNK_CELL_SIZE, CHUNK_CELL_SIZE, colour);
}