#include "grid.h"

#include <assert.h>
#include <string.h>

#include "raylib.h"

#include "element_update.h"

ElementGrid create_grid()
{
    ElementGrid grid;
    
    memset(grid.current_grid, 0, GRID_SIZE * sizeof(Element));
    memset(grid.next_grid, 0, GRID_SIZE * sizeof(Element));

    return grid;
}

bool in_bounds_of_grid(int x, int y)
{
    return x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT;
}

Element get_element_in_grid(const ElementGrid* grid, int x, int y)
{
    assert(in_bounds_of_grid(x, y));

    return grid->current_grid[x + y * GRID_WIDTH];
}

bool overwrite_element_in_grid(ElementGrid* grid, int x, int y, Element element)
{
    if (in_bounds_of_grid(x, y))
    {
        grid->current_grid[x + y * GRID_WIDTH] = element;

        return true;
    }

    return false;
}

bool set_element_in_grid(ElementGrid* grid, int x, int y, Element element)
{
    if (in_bounds_of_grid(x, y))
    {
        grid->next_grid[x + y * GRID_WIDTH] = element;

        return true;
    }

    return false;
}

bool is_empty_in_grid(const ElementGrid* grid, int x, int y)
{
    bool current = grid->current_grid[x + y * GRID_WIDTH] == ELEMENT_EMPTY;
    bool next = grid->next_grid[x + y * GRID_WIDTH] == ELEMENT_EMPTY;

    return in_bounds_of_grid(x, y) && current && next;
}

void update_grid(ElementGrid* grid)
{
    // check all pixels
    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {   
            Element current_element = get_element_in_grid(grid, x, y);

            if (current_element == ELEMENT_SAND)
            {
                update_sand(grid, x, y);      
            }
            else if (current_element == ELEMENT_WATER)
            {
                update_water(grid, x, y);
            }
            else if (current_element == ELEMENT_STONE)
            {
                set_element_in_grid(grid, x, y, ELEMENT_STONE);
            }
        }
    }

    // copy next state to the new grid state
    memcpy(grid->current_grid, grid->next_grid, GRID_SIZE * sizeof(Element));

    // reset next_grid
    memset(grid->next_grid, 0, GRID_SIZE * sizeof(Element));
}

void draw_element(int x, int y, Color colour)
{
    DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, colour);
}

void draw_grid(const ElementGrid* grid)
{
    // prolly draw to a texture and render it

    for (int x = 0; x < GRID_WIDTH; x++)
    {
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            Element current_element = get_element_in_grid(grid, x, y);

            switch (current_element)
            {
                case ELEMENT_SAND: draw_element(x, y, YELLOW); break;
                case ELEMENT_WATER: draw_element(x, y, BLUE); break;
                case ELEMENT_STONE: draw_element(x, y, GRAY); break;
                default: break;
            }
        }
    }
}