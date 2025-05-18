#include "raylib.h"

#include "sandbox.h"
#include "instrumentor.h"

void raylib()
{
    InitWindow(1280, 720, "Pixel Physics");

    auto sandbox = Sandbox();
    Vector2 movement = { 0, 0 };
    Camera2D camera = { 0 };
    camera.target = { 0, 0 };
    camera.offset = { 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = .5f; 

    while (!WindowShouldClose())
    {
        float frame_time = GetFrameTime();

        int place_width = 5;  
        int palce_height = 5; 

        if (IsMouseButtonDown(0))
        {
            Vector2 pos = GetMousePosition();
            pos = GetScreenToWorld2D(pos, camera);

            auto [gx, gy] = sandbox.pos_to_grid(pos.x, pos.y);

            for (int dx = -place_width / 2; dx <= place_width / 2; ++dx) {
                for (int dy = -palce_height / 2; dy <= palce_height / 2; ++dy) {
                    int x = gx + dx;
                    int y = gy + dy;
                    sandbox.set_cell(x, y, Cell(
                        CellType::Sand,
                        YELLOW
                    ));
                }
            }
        }

        if (IsMouseButtonDown(1))
        {
            Vector2 pos = GetMousePosition();
            pos = GetScreenToWorld2D(pos, camera);

            auto [gx, gy] = sandbox.pos_to_grid(pos.x, pos.y);

            for (int dx = -place_width / 2; dx <= place_width / 2; ++dx) {
                for (int dy = -palce_height / 2; dy <= palce_height / 2; ++dy) {
                    int x = gx + dx;
                    int y = gy + dy;
                    sandbox.set_cell(x, y, Cell(
                        CellType::Stone,
                        GRAY
                    ));
                }
            }
        }

        if (IsKeyDown(KEY_D)) movement.x += 512.0f * frame_time;
        if (IsKeyDown(KEY_A)) movement.x -= 512.0f * frame_time;
        if (IsKeyDown(KEY_W)) movement.y -= 512.0f * frame_time;
        if (IsKeyDown(KEY_S)) movement.y += 512.0f * frame_time;
        
        camera.target = movement;

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

        BeginDrawing();
        ClearBackground(BLANK);

        // pre draw
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