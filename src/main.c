#include "raylib.h"
#include <stdlib.h>
#include <string.h>

#define CELL_SIZE 4
#define ROW 128
#define COL 128
#define GRID_SIZE ROW * COL 

// TODO: Add various elements then just sand
typedef enum Element
{
    ELEMENT_EMPTY = 0,
    ELEMENT_SAND,
    ELEMENT_WATER,
} Element;

Element get_element(Element* grid, int x, int y)
{
    return grid[x + y * COL];
}

bool in_bounds(int x, int y)
{
    return x >= 0 && x < ROW && y >= 0 && y < COL;
}

bool is_empty(Element* grid, int x, int y)
{
    if (in_bounds(x, y))
    {
        return get_element(grid, x, y) == 0;
    }

    return false;
}

bool set_element(Element* grid, int x, int y, Element element)
{
    if (in_bounds(x, y))
    {
        grid[x + y * COL] = element;
    }

    return false;
}

void place_elements(Element* grid)
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
            
            if (in_bounds(extentX, extentY) && should_place)
            {
                set_element(grid, extentX, extentY, 1);
            }
        }
    }
}

void remove_elements(Element* grid)
{
    int x = GetMouseX() / CELL_SIZE;
    int y = GetMouseY() / CELL_SIZE;

    if (in_bounds(x, y))
    {
        set_element(grid, x, y, 0);
    }
}

void update_sand(Element* current_grid, Element* next_grid, int x, int y)
{
    // makes it less robotic
    int random_dir = rand() % 2 == 1 ? 1 : -1;

    // is below empty?
    if (is_empty(current_grid, x, y + 1))
    {
        set_element(next_grid, x, y + 1, 1);
    }
    // is below right side empty?
    else if (is_empty(current_grid, x + random_dir, y + 1))
    {
        set_element(next_grid, x + random_dir, y + 1, 1);
    }
    // is below left side empty?
    else if (is_empty(current_grid, x - random_dir, y + 1))
    {
        set_element(next_grid, x - random_dir, y + 1, 1);
    }
    // divent move
    else
    {
        set_element(next_grid, x, y, 1);
    }         
}

int main()
{
    InitWindow(512, 512, "Sand Simulator");

    // main grid
    Element current_grid[GRID_SIZE] = {0};  

    while (!WindowShouldClose())
    {
        // bad practice, forces the app to run at 0.01 seconds per frame
        WaitTime(0.01);

        // place or remove pixels at mouse pos
        if (IsMouseButtonDown(0))
        {
            place_elements(current_grid);
        }
        else if (IsMouseButtonDown(1))
        {
            remove_elements(current_grid);
        }
        
        // create a temp array to create the next current grid
        Element next_grid[GRID_SIZE] = {0};

        // check all pixels
        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y++)
            {   
                Element current_element = get_element(current_grid, x, y);

                if (current_element == ELEMENT_SAND)
                {
                    update_sand(current_grid, next_grid, x, y);      
                }
            }
        }

        // copy next state to the new grid state
        memcpy(current_grid, next_grid, GRID_SIZE * sizeof(int));

        BeginDrawing();
        ClearBackground(BLACK);

        // draw new grid
        for (int x = 0; x < ROW; x++)
        {
            for (int y = 0; y < COL; y++)
            {
                if (get_element(current_grid, x, y) == ELEMENT_SAND)
                {
                    DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, YELLOW);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();  

    return 0;
}