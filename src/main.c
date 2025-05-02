#include <stdlib.h>

#include "cell.h"
#include "raylib.h"

#include "grid.h" 

Cell SAND_CELL = { 
    CELL_TYPE_SAND, 
    CELL_MOVEMENT_MOVE_DOWN | CELL_MOVEMENT_MOVE_DOWN_DIAGONAL, 
    YELLOW
};
Cell WATER_CELL = { 
    CELL_TYPE_WATER, 
    CELL_MOVEMENT_MOVE_DOWN | CELL_MOVEMENT_MOVE_DOWN_DIAGONAL | CELL_MOVEMENT_MOVE_SIDEWAYS,  
    BLUE 
};
Cell STONE_CELL = { 
    CELL_TYPE_STONE, 
    CELL_MOVEMENT_STATIONARY, 
    DARKGRAY 
};
Cell STEAM_CELL = { 
    CELL_TYPE_STEAM, 
    CELL_MOVEMENT_MOVE_UP | CELL_MOVEMENT_MOVE_UP_DIAGONAL | CELL_MOVEMENT_MOVE_SIDEWAYS,
    GRAY 
};

void place_elements(CellChunk* chunk, Cell* current_cell)
{
    int x = mouse_to_grid_x(GetMouseX());
    int y = mouse_to_grid_y(GetMouseY());
    int area = 5;
    
    int dx = (rand() % (2 * area + 1)) - area;
    int dy = (rand() % (2 * area + 1)) - area;

    int extentX = x + dx;
    int extentY = y + dy;

    if (in_bounds_of_chunk(extentX, extentY))
    {
        set_cell_in_chunk(chunk, extentX, extentY, current_cell);
    }
}

void remove_elements(CellChunk* chunk)
{
    int x = mouse_to_grid_x(GetMouseX());
    int y = mouse_to_grid_y(GetMouseY());

    if (in_bounds_of_chunk(x, y) && is_empty_in_chunk(chunk, x, y))
    {
        overwrite_cell_in_chunk(chunk, x, y, &DEFAULT_CELL);
    }
}

int main()
{
    InitWindow(512, 512, "Sand Simulator");
    //SetTargetFPS(60); // make my laptop happy

    // main grid
    CellChunk* chunk = create_chunk();
    Cell* current_cell = &SAND_CELL;

    const int target_fps = 60;
    const float time_step = 1.0f / target_fps;
    float accumulator = 0.0f;
    float previous_time = GetTime();

    while (!WindowShouldClose())
    {
        float current_time = GetTime();
        float frame_time = current_time - previous_time;
        previous_time = current_time;
        accumulator += frame_time;

        if (IsKeyPressed(KEY_ONE)) current_cell = &SAND_CELL;
        if (IsKeyPressed(KEY_TWO)) current_cell = &WATER_CELL;
        if (IsKeyPressed(KEY_THREE)) current_cell = &STONE_CELL;
        if (IsKeyPressed(KEY_FOUR)) current_cell = &STEAM_CELL;

        // place or remove pixels at mouse pos
        if (IsMouseButtonDown(0))
        {
            place_elements(chunk, current_cell);
        }
        else if (IsMouseButtonDown(1))
        {
            remove_elements(chunk);
        }
        
        // update cells at a fixed rate
        while (accumulator >= time_step)
        {
            update_chunk(chunk);
            accumulator -= time_step;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        draw_chunk(chunk);
        DrawText(TextFormat("Fps:%d", (int)(1/frame_time)), 0, 0, 20, RED);

        EndDrawing();
    }

    CloseWindow();  

    destroy_chunk(chunk);

    return 0;
}