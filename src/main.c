#include <stdlib.h>
#include <string.h>

#include "raylib.h"

#include "grid.h" 

#define CELL_SIZE 4
#define ROW 128
#define COL 128
#define GRID_SIZE ROW * COL 

void place_elements(ElementGrid* grid, Element current_element)
{
    int x = GetMouseX() / CELL_SIZE;
    int y = GetMouseY() / CELL_SIZE;
    int area = 1;

    for (int i = -area; i < area; i++)
    {
        for (int j = -area; j < area; j++)
        {
            int extentX = x + i;
            int extentY = y + j; 
            bool should_place = rand() % 2 == 1;
            
            if (in_bounds(grid, extentX, extentY) && should_place)
            {
                set_element(grid, extentX, extentY, current_element);
            }
        }
    }
}

void remove_elements(ElementGrid* grid)
{
    int x = GetMouseX() / CELL_SIZE;
    int y = GetMouseY() / CELL_SIZE;

    if (in_bounds(grid, x, y))
    {
        set_element(grid, x, y, 0);
    }
}

int random_dir()
{
    int directions[2] = { -1, 1 };   
    int index = rand() % 2;

    return directions[index];
}

void update_sand(const ElementGrid* current_grid, ElementGrid* next_grid, int x, int y)
{
    // makes it less robotic
    int dir = random_dir();

    // is below empty?
    if (is_empty(current_grid, x, y + 1))
    {
        set_element(next_grid, x, y + 1, ELEMENT_SAND);
    }
    // is diagonal empty?
    else if (is_empty(current_grid, x + dir, y + 1))
    {
        set_element(next_grid, x + dir, y + 1, ELEMENT_SAND);
    }
    // divent move
    else
    {
        set_element(next_grid, x, y, ELEMENT_SAND);
    }         
}

void update_water(const ElementGrid* current_grid, ElementGrid* next_grid, int x, int y)
{
    int dir = random_dir();

    // had to double check the next_grid to prevent overwriting
    // is below empty?
    if (is_empty(current_grid, x, y + 1) && is_empty(next_grid, x, y + 1)) 
    {
        set_element(next_grid, x, y + 1, ELEMENT_WATER);
    }
    // is diagonal empty?
    else if (is_empty(current_grid, x + dir, y + 1) && is_empty(next_grid, x + dir, y + 1)) 
    {
        set_element(next_grid, x + dir, y + 1, ELEMENT_WATER);
    }
    // is horizontal empty?
    else if (is_empty(current_grid, x + dir, y) && is_empty(next_grid, x + dir, y)) 
    {
        set_element(next_grid, x + dir, y, ELEMENT_WATER);
    }
    // divent move
    else
    {
        set_element(next_grid, x, y, ELEMENT_WATER);
    }
}

int main()
{
    InitWindow(512, 512, "Sand Simulator");

    // main grid
    Element data[GRID_SIZE] = { 0 };
    ElementGrid current_grid = {
        data,
        ROW, COL
    };

    Element current_element = ELEMENT_SAND;

    while (!WindowShouldClose())
    {
        // bad practice, forces the app to run at 0.01 seconds per frame
        WaitTime(0.01);

        if (IsKeyPressed(KEY_ONE)) current_element = ELEMENT_SAND;
        if (IsKeyPressed(KEY_TWO)) current_element = ELEMENT_WATER;
        if (IsKeyPressed(KEY_THREE)) current_element = ELEMENT_STONE;

        // place or remove pixels at mouse pos
        if (IsMouseButtonDown(0))
        {
            place_elements(&current_grid, current_element);
        }
        else if (IsMouseButtonDown(1))
        {
            remove_elements(&current_grid);
        }
        
        // create a temp array to create the next current grid
        Element next_data[GRID_SIZE] = { 0 };
        ElementGrid next_grid = {
            next_data,
            ROW, COL
        };

        // check all pixels
        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y++)
            {   
                Element current_element = get_element(&current_grid, x, y);

                if (current_element == ELEMENT_WATER)
                {
                    update_water(&current_grid, &next_grid, x, y);
                }
                else if (current_element == ELEMENT_SAND)
                {
                    update_sand(&current_grid, &next_grid, x, y);      
                }
                else if (current_element == ELEMENT_STONE)
                {
                    set_element(&next_grid, x, y, ELEMENT_STONE);
                }
            }
        }

        // copy next state to the new grid state
        memcpy(current_grid.data, next_grid.data, GRID_SIZE * sizeof(Element));

        BeginDrawing();
        ClearBackground(BLACK);

        // draw new grid
        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y++)
            {
                if (get_element(&current_grid, x, y) == ELEMENT_SAND)
                {
                    DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, YELLOW);
                }
                else if (get_element(&current_grid, x, y) == ELEMENT_WATER)
                {
                    DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, BLUE);
                }
                else if (get_element(&current_grid, x, y) == ELEMENT_STONE)
                {
                    DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, GRAY);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();  

    return 0;
}