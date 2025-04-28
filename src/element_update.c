#include "element_update.h"

#include <stdlib.h>

int random_dir()
{
    int directions[2] = { -1, 1 };   
    int index = rand() % 2;

    return directions[index];
}

void update_sand(ElementGrid* grid, int x, int y)
{
    // makes it less robotic
    int dir = random_dir();

    // is below empty?
    if (is_empty_in_grid(grid, x, y + 1))
    {
        set_element_in_grid(grid, x, y + 1, ELEMENT_SAND);
    }
    // is diagonal empty?
    else if (is_empty_in_grid(grid, x + dir, y + 1))
    {
        set_element_in_grid(grid, x + dir, y + 1, ELEMENT_SAND);
    }
    // divent move
    else
    {
        set_element_in_grid(grid, x, y, ELEMENT_SAND);
    }         
}

void update_water(ElementGrid* grid, int x, int y)
{
    int dir = random_dir();

    // is below empty?
    if (is_empty_in_grid(grid, x, y + 1)) 
    {
        set_element_in_grid(grid, x, y + 1, ELEMENT_WATER);
    }
    // is diagonal empty?
    else if (is_empty_in_grid(grid, x + dir, y + 1)) 
    {
        set_element_in_grid(grid, x + dir, y + 1, ELEMENT_WATER);
    }
    // is horizontal empty?
    else if (is_empty_in_grid(grid, x + dir, y)) 
    {
        set_element_in_grid(grid, x + dir, y, ELEMENT_WATER);
    }
    // divent move
    else
    {
        set_element_in_grid(grid, x, y, ELEMENT_WATER);
    }
}