#include <iostream>

#include "cell.h"
#include "raylib.h"

#include "sandbox.h"

void raylib()
{
    InitWindow(1280, 720, "Pixel Physics");

    const int target_fps = 60;
    const float time_step = 1.0f / target_fps;
    float accumulator = 0.0f;
    float previous_time = GetTime();

    auto sandbox = Sandbox();
    Vector2 movement = { 0, 0 };
    Camera2D camera = { 0 };
    camera.target = { 0, 0 };
    camera.offset = { 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = .5f; 


    while (!WindowShouldClose())
    {
        float current_time = GetTime();
        float frame_time = current_time - previous_time;
        previous_time = current_time;
        accumulator += frame_time;

        if (IsMouseButtonDown(0))
        {
            Vector2 pos = GetMousePosition();
            pos = GetScreenToWorld2D(pos, camera);

            auto [x, y] = sandbox.pos_to_grid(pos.x, pos.y);
            pos.x = x;
            pos.y = y;

            sandbox.set_cell(pos.x, pos.y, Cell(
                CellType::Sand,
                YELLOW
            ));
        }

        if (IsMouseButtonDown(1))
        {
            Vector2 pos = GetMousePosition();
            pos = GetScreenToWorld2D(pos, camera);

            auto [x, y] = sandbox.pos_to_grid(pos.x, pos.y);
            pos.x = x;
            pos.y = y;

            sandbox.set_cell(pos.x, pos.y, Cell(
                CellType::Stone,
                GRAY
            ));
        }

        if (IsKeyDown(KEY_D)) movement.x += 128.0f * frame_time;
        if (IsKeyDown(KEY_A)) movement.x -= 128.0f * frame_time;
        if (IsKeyDown(KEY_W)) movement.y -= 128.0f * frame_time;
        if (IsKeyDown(KEY_S)) movement.y += 128.0f * frame_time;
        
        camera.target = movement;

        while (accumulator >= time_step)
        {
            sandbox.update([&](const Cell& cell, int32_t x, int32_t y)
            {
                if (cell.cell_type == CellType::Stone)
                {
                    sandbox.set_cell(x, y, cell);
                }

                if (cell.cell_type == CellType::Sand)
                {
                    if (sandbox.has_empty_cell(x, y + 1))
                    {
                        sandbox.set_cell(x, y + 1, cell);
                    }
                    else if (sandbox.has_empty_cell(x + 1, y + 1))
                    {
                        sandbox.set_cell(x + 1, y + 1, cell);
                    }
                    else if (sandbox.has_empty_cell(x - 1, y + 1))
                    {
                        sandbox.set_cell(x + 1, y + 1, cell);
                    }
                    else 
                    {
                        sandbox.set_cell(x, y, cell);
                    }
                }
            });

            accumulator -= time_step;
        }

        BeginDrawing();
        ClearBackground(BLANK);

        // pre draw
        sandbox.pre_draw();

        // within camera draw
        BeginMode2D(camera);
        sandbox.draw();
        sandbox.debug_draw();
        EndMode2D();

        // global draw
        DrawFPS(0, 0);
        DrawText(TextFormat("FrameTime: %.5f", GetFrameTime() * 1000), 0, 20, 20, RED);


        EndDrawing();
    }

    CloseWindow();
}

int main()
{
    raylib();
}