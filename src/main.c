#include <stdlib.h>

#include "cell.h"
#include "raylib.h"

#include "grid.h" 

Cell DEFAULT_CELL = {
    CELL_TYPE_EMPTY,
    ELEMENT_PROPERTY_MOVE_NONE,
    BLACK
};

Cell SAND_CELL = { 
    CELL_TYPE_SAND, 
    ELEMENT_PROPERTY_MOVE_DOWN | ELEMENT_PROPERTY_MOVE_DOWN_DIAGONAL, 
    YELLOW
};
Cell WATER_CELL = { 
    CELL_TYPE_WATER, 
    ELEMENT_PROPERTY_MOVE_DOWN | ELEMENT_PROPERTY_MOVE_DOWN_DIAGONAL | ELEMENT_PROPERTY_MOVE_SIDEWAYS,  
    BLUE 
};
Cell STONE_CELL = { 
    CELL_TYPE_STONE, 
    ELEMENT_PROPERTY_STATIONARY, 
    DARKGRAY 
};
Cell STEAM_CELL = { 
    CELL_TYPE_STEAM, 
    ELEMENT_PROPERTY_MOVE_UP | ELEMENT_PROPERTY_MOVE_UP_DIAGONAL | ELEMENT_PROPERTY_MOVE_SIDEWAYS,
    GRAY 
};

void place_elements(CellChunk* chunk, Cell* current_cell)
{
    int x = GetMouseX() / chunk->cell_size;
    int y = GetMouseY() / chunk->cell_size;
    int area = 5;

    for (int i = -area; i < area; i++)
    {
        for (int j = -area; j < area; j++)
        {
            int extentX = x + i;
            int extentY = y + j; 
            bool should_place = random() % 2 == 1;
            
            if (in_bounds_of_chunk(chunk, x, y) && in_bounds_of_chunk(chunk, extentX, extentY) && should_place)
            {
                set_cell_in_chunk(chunk, extentX, extentY, current_cell);
            }
        }
    }
}

void remove_elements(CellChunk* chunk)
{
    int x = GetMouseX() / chunk->cell_size;
    int y = GetMouseY() / chunk->cell_size;

    if (in_bounds_of_chunk(chunk, x, y) && is_empty_in_chunk(chunk, x, y))
    {
        overwrite_cell_in_chunk(chunk, x, y, &DEFAULT_CELL);
    }
}

int main()
{
    InitWindow(512, 512, "Sand Simulator");
    SetTargetFPS(60); // make my laptop happy

    // main grid
    CellChunk chunk = create_chunk(128, 128, 4);
    Cell* current_cell = &SAND_CELL;

    const int target_fps = 60;
    const float time_step = (float)1 / target_fps;
    float accumulator = 0.0f;
    float current_time = GetTime();

    while (!WindowShouldClose())
    {
        float newTime = GetTime();
        float frameTime = newTime - current_time;
        current_time = newTime;

        if (frameTime > 0.25f) frameTime = 0.25f;

        accumulator += frameTime;



        if (IsKeyPressed(KEY_ONE)) current_cell = &SAND_CELL;
        if (IsKeyPressed(KEY_TWO)) current_cell = &WATER_CELL;
        if (IsKeyPressed(KEY_THREE)) current_cell = &STONE_CELL;
        if (IsKeyPressed(KEY_FOUR)) current_cell = &STEAM_CELL;

        // place or remove pixels at mouse pos
        if (IsMouseButtonDown(0))
        {
            place_elements(&chunk, current_cell);
        }
        else if (IsMouseButtonDown(1))
        {
            remove_elements(&chunk);
        }

        // update cells at a fixed rate
        while (accumulator >= time_step)
        {
            update_chunk(&chunk);
            accumulator -= time_step;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        draw_chunk(&chunk);

        EndDrawing();
    }

    CloseWindow();  

    destroy_chunk(&chunk);

    return 0;
}