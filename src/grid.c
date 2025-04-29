#include "grid.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"

CellChunk create_chunk(size_t width, size_t height, unsigned int cell_size)
{
    assert(width != 0 && height != 0);
    assert(cell_size != 0);

    size_t grid_size = width * height;

    CellChunk grid = {
        malloc(grid_size * sizeof(Cell)),
        malloc(grid_size * sizeof(Cell)),
        width,
        height,
        cell_size
    };

    return grid;
}

void destroy_chunk(CellChunk* chunk)
{
    // free grid
    free(chunk->current_grid);
    free(chunk->next_grid);

    // set size to 0
    chunk->width = 0;
    chunk->height = 0; 
}

bool in_bounds_of_chunk(const CellChunk* chunk, size_t x, size_t y)
{
    return x >= 0 && x < chunk->width && y >= 0 && y < chunk->height;
}

Cell* get_cell_in_chunk(const CellChunk* chunk, size_t x, size_t y)
{
    assert(in_bounds_of_chunk(chunk, x, y));

    // make sure the array doesnt realloc or relocate so that the pointer doesnt invalidate!
    return &chunk->current_grid[x + y * chunk->width];
}

bool overwrite_cell_in_chunk(CellChunk* chunk, size_t x, size_t y, const Cell* cell)
{
    if (cell == NULL) return false;

    if (in_bounds_of_chunk(chunk, x, y))
    {
        chunk->current_grid[x + y * chunk->width] = *cell;

        return true;
    }

    return false;
}

bool set_cell_in_chunk(CellChunk* chunk, size_t x, size_t y, const Cell* cell)
{
    if (cell == NULL) return false;

    if (in_bounds_of_chunk(chunk, x, y))
    {
        chunk->next_grid[x + y * chunk->width] = *cell;

        return true;
    }

    return false;
}

bool is_empty_in_chunk(const CellChunk* chunk, size_t x, size_t y)
{
    bool current = chunk->current_grid[x + y * chunk->width].type == CELL_TYPE_EMPTY;
    bool next = chunk->next_grid[x + y * chunk->width].type == CELL_TYPE_EMPTY;

    return in_bounds_of_chunk(chunk, x, y) && current && next;
}

void update_chunk(CellChunk* chunk)
{
    // check all pixels
    for (size_t x = 0; x < chunk->width; x++)
    {
        for (size_t y = 0; y < chunk->height; y++)
        {   
            Cell* current_cell = get_cell_in_chunk(chunk, x, y);
            ElementProperty property = current_cell->property;

            if      (property & ELEMENT_PROPERTY_MOVE_UP            && move_cell_up(chunk, x, y, current_cell)) {}
            else if (property & ELEMENT_PROPERTY_MOVE_UP_DIAGONAL   && move_cell_up_diagonal(chunk, x, y, current_cell)) {}
            else if (property & ELEMENT_PROPERTY_MOVE_DOWN          && move_cell_down(chunk, x, y, current_cell)) {}
            else if (property & ELEMENT_PROPERTY_MOVE_DOWN_DIAGONAL && move_cell_down_diagonal(chunk, x, y, current_cell)) {}
            else if (property & ELEMENT_PROPERTY_MOVE_SIDEWAYS      && move_cell_sideways(chunk, x, y, current_cell)) {}
            else if (property != ELEMENT_PROPERTY_MOVE_NONE)
            {
                set_cell_in_chunk(chunk, x, y, current_cell); // will keep cells inside window
            }
        }
    }

    size_t grid_size = chunk->width * chunk->height;

    // copy next state to the new grid state
    memcpy(chunk->current_grid, chunk->next_grid, grid_size * sizeof(Cell));

    // reset next_grid
    memset(chunk->next_grid, 0, grid_size * sizeof(Cell));
}

void draw_chunk(const CellChunk* chunk)
{
    // prolly draw to a texture and render it

    for (size_t x = 0; x < chunk->width; x++)
    {
        for (size_t y = 0; y < chunk->height; y++)
        {
            Cell* current_cell = get_cell_in_chunk(chunk, x, y);

            draw_cell(chunk, x, y, current_cell->colour);
        }
    }
}