#include "raylib.h"

#include "cell_chunk_manager.h"
#include "instrumentor.h"

void raylib()
{
    InitWindow(1280, 720, "Pixel Physics");

    CellChunkManager sandbox;
    Vector2 movement = { 0, 0 };
    Camera2D camera = { 0 };
    camera.target = { 0, 0 };
    camera.offset = { 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = .5f; 

    while (!WindowShouldClose())
    {
        float frame_time = GetFrameTime();

        if (IsKeyDown(KEY_D)) movement.x += 512.0f * frame_time;
        if (IsKeyDown(KEY_A)) movement.x -= 512.0f * frame_time;
        if (IsKeyDown(KEY_W)) movement.y -= 512.0f * frame_time;
        if (IsKeyDown(KEY_S)) movement.y += 512.0f * frame_time;
        
        if (IsKeyDown(KEY_SPACE))
        {
            Vector2 pos = GetMousePosition();
            pos = GetScreenToWorld2D(pos, camera);

            auto [gx, gy] = sandbox.pos_to_grid(pos.x, pos.y);

            sandbox.set_cell(gx, gy, {
                CellType::Stone,
                GRAY
            });
        }

        if (IsKeyDown(KEY_C))
        {
            Vector2 pos = GetMousePosition();
            pos = GetScreenToWorld2D(pos, camera);

            auto [gx, gy] = sandbox.pos_to_grid(pos.x, pos.y);

            sandbox.set_cell(gx, gy, {
                CellType::Sand,
                YELLOW
            });
        }

        camera.target = movement;

        sandbox.update([&](const Cell& cell, int x, int y)
        {
            if (cell.type == CellType::Sand)
            {
                if (sandbox.is_empty(x, y + 1))
                {
                    sandbox.set_cell(x, y, { CellType::Empty, BLANK });
                    sandbox.set_cell(x, y + 1, cell);
                }
                else if (sandbox.is_empty(x + 1, y))
                {
                    sandbox.set_cell(x, y, { CellType::Empty, BLANK });
                    sandbox.set_cell(x + 1, y, cell);
                }
            }
        });

        BeginDrawing();
        ClearBackground(BLANK);

        sandbox.pre_draw(camera);

        // within camera draw
        BeginMode2D(camera);
        sandbox.draw(camera, true);
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
    PROFILE_BEGIN_SESSION("Pixel", "result", 2048);

    raylib();

    PROFILE_END_SESSION();
}