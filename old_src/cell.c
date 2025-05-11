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

bool move_cell_up(CellChunk* chunk, unsigned int x, unsigned int y, Cell* cell)
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
        cell->direction_x = 0;
        cell->direction_y = 1;

        return set_cell_in_chunk(chunk, x, y - movement_amount, cell);
    }

    return false;
}

bool move_cell_down(CellChunk* chunk, unsigned int x, unsigned int y, Cell* cell)
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
        cell->direction_x = 0;
        cell->direction_y = -1;

        return set_cell_in_chunk(chunk, x, y + movement_amount, cell);
    }

    return false;
}

bool move_cell_sideways(CellChunk* chunk, unsigned int x, unsigned int y, Cell* cell)
{
    bool right = is_empty_in_chunk(chunk, x + 1, y);
    bool left = is_empty_in_chunk(chunk, x - 1, y);

    if (cell->direction_x == 1 || (right && cell->direction_x == 0))
    {
        if (right)
        {
            cell->direction_x = 1;

            return set_cell_in_chunk(chunk, x + 1, y, cell);
        }
        else if (left) 
        {
            cell->direction_x = -1;

            return set_cell_in_chunk(chunk, x - 1, y, cell);
        }

        cell->direction_x = 0;
        
        return false;
    }

    if (cell->direction_x == -1 || (left && cell->direction_x == 0))
    {
        if (left)
        {
            cell->direction_x = -1;

            return set_cell_in_chunk(chunk, x - 1, y, cell);
        }
        else if (right) 
        {
            cell->direction_x = 1;

            return set_cell_in_chunk(chunk, x + 1, y, cell);
        }

        cell->direction_x = 0;
        
        return false;
    }

    cell->direction_x = 0;

    return false;
}

bool move_cell_up_diagonal(CellChunk* chunk, unsigned int x, unsigned int y, Cell* cell)
{
    int dir = random_dir();

    if (is_empty_in_chunk(chunk, x + dir, y - 1))
    {
        cell->direction_x = dir;
        cell->direction_y = 1;

        return set_cell_in_chunk(chunk, x + dir, y - 1, cell);
    }

    return false;
}

bool move_cell_down_diagonal(CellChunk* chunk, unsigned int x, unsigned int y, Cell* cell)
{
    int dir = random_dir();

    if (is_empty_in_chunk(chunk, x + dir, y + 1))
    {
        cell->direction_x = dir;
        cell->direction_y = -1;

        return set_cell_in_chunk(chunk, x + dir, y + 1, cell);
    }

    return false;
}

void draw_cell(unsigned int x, unsigned int y, Color colour)
{
    DrawRectangle(x * CHUNK_CELL_SIZE, y * CHUNK_CELL_SIZE, CHUNK_CELL_SIZE, CHUNK_CELL_SIZE, colour);
}