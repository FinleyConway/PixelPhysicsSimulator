#include "cell_chunk.h"
#include "raylib.h"
#include "sandbox.h"


void raylib()
{
    InitWindow(512, 512, "Pixel Physics");
    auto sandbox = Sandbox();

    const int target_fps = 60;
    const float time_step = 1.0f / target_fps;
    float accumulator = 0.0f;
    float previous_time = GetTime();

    Camera2D camera;
    camera.zoom = 1;

    while (!WindowShouldClose())
    {
        float current_time = GetTime();
        float frame_time = current_time - previous_time;
        previous_time = current_time;
        accumulator += frame_time;

        if (IsMouseButtonDown(1))
        {
            Vector2 pos = GetMousePosition();
            pos = GetScreenToWorld2D(pos, camera);

            
            auto [x, y] = sandbox.pos_to_grid(pos.x, pos.y);
            pos.x = x;
            pos.y = y;

            sandbox.set_cell(pos.x, pos.y, Cell(
                CellType::Bob,
                WHITE
            ));
        }

        // update cells at a fixed rate
        while (accumulator >= time_step)
        {
            auto& chunk = sandbox.get_or_create_chunk(0, 0);
            chunk.update();
            accumulator -= time_step;
        }

        //std::cout << sandbox.has_cell(1, 0) << std::endl;


        BeginDrawing();
        ClearBackground(BLANK);
        BeginMode2D(camera);

        auto& chunk = sandbox.get_or_create_chunk(0, 0);
        chunk.draw(); 
        DrawText(TextFormat("Fps:%d", (int)(1/frame_time)), 0, 0, 20, RED);

        EndMode2D();
        EndDrawing();
    }

    CloseWindow();
}

int main()
{
    raylib();
}