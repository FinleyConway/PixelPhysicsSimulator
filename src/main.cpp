#include "raylib.h"

#include "cell_chunk.h"
#include "sandbox.h"


void raylib()
{
    InitWindow(512, 512, "Pixel Physics");
    auto sandbox = Sandbox();

    const int target_fps = 60;
    const float time_step = 1.0f / target_fps;
    float accumulator = 0.0f;
    float previous_time = GetTime();

    Vector2 movement = { 0, 0 };
    Camera2D camera = { 0 };
    camera.target = { 0, 0 };
    camera.offset = { 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f; 


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

        if (IsKeyDown(KEY_RIGHT)) movement.x += 128.0f * frame_time;
        if (IsKeyDown(KEY_LEFT)) movement.x -= 128.0f * frame_time;
        if (IsKeyDown(KEY_UP)) movement.y -= 128.0f * frame_time;
        if (IsKeyDown(KEY_DOWN)) movement.y += 128.0f * frame_time;
        
        camera.target = movement;
        camera.offset = { 512.f/2, 512.f/2 };

        // update cells at a fixed rate
        while (accumulator >= time_step)
        {
            sandbox.update();

            accumulator -= time_step;
        }

        BeginDrawing();
        ClearBackground(BLANK);

        // pre draw
        sandbox.pre_draw();

        // within camera draw
        BeginMode2D(camera);

        sandbox.draw();

        EndMode2D();

        // global draw
        DrawText(TextFormat("Fps:%d", (int)(1/frame_time)), 0, 0, 20, RED);

        EndDrawing();
    }

    CloseWindow();
}

int main()
{
    raylib();
}