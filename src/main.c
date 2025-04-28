#include <stdlib.h>

#include "raylib.h"

#include "grid.h" 

void place_elements(ElementGrid* grid, Element current_element)
{
    int x = GetMouseX() / CELL_SIZE;
    int y = GetMouseY() / CELL_SIZE;
    int area = 5;

    for (int i = -area; i < area; i++)
    {
        for (int j = -area; j < area; j++)
        {
            int extentX = x + i;
            int extentY = y + j; 
            bool should_place = rand() % 2 == 1;
            
            if (in_bounds_of_grid(extentX, extentY) && should_place)
            {
                set_element_in_grid(grid, extentX, extentY, current_element);
            }
        }
    }
}

void remove_elements(ElementGrid* grid)
{
    int x = GetMouseX() / CELL_SIZE;
    int y = GetMouseY() / CELL_SIZE;

    if (in_bounds_of_grid(x, y))
    {
        overwrite_element_in_grid(grid, x, y, ELEMENT_EMPTY);
    }
}

int main()
{
    InitWindow(512, 512, "Sand Simulator");

    // main grid
    ElementGrid grid = create_grid();
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
            place_elements(&grid, current_element);
        }
        else if (IsMouseButtonDown(1))
        {
            remove_elements(&grid);
        }

        update_grid(&grid);

        BeginDrawing();
        ClearBackground(BLACK);

        draw_grid(&grid);

        EndDrawing();
    }

    CloseWindow();  

    return 0;
}