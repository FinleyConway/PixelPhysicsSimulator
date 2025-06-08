#include <raylib.h>

#include "core/cell.hpp"
#include "simulation/chunk_manager.hpp"
#include "core/chunk_updater.hpp"

void input(ChunkManager& sandbox, Cell& current_cell, Camera2D& camera, Vector2& movement, bool& debug_mode, float frame_time)
{
    const int brush_radius = 2;

    if (IsKeyDown(KEY_D)) movement.x += 512.0f * frame_time;
    if (IsKeyDown(KEY_A)) movement.x -= 512.0f * frame_time;
    if (IsKeyDown(KEY_W)) movement.y -= 512.0f * frame_time;
    if (IsKeyDown(KEY_S)) movement.y += 512.0f * frame_time;
    
    if (IsKeyPressed(KEY_ONE))   current_cell = Cell::Sand;
    if (IsKeyPressed(KEY_TWO))   current_cell = Cell::Water;
    if (IsKeyPressed(KEY_THREE)) current_cell = Cell::Stone;
    if (IsKeyPressed(KEY_FOUR)) current_cell = Cell::Smoke;

    if (IsKeyPressed(KEY_F1)) debug_mode = !debug_mode;

    if (IsMouseButtonDown(0))
    {
        Vector2 pos = GetScreenToWorld2D(GetMousePosition(), camera);
        auto [gx, gy] = sandbox.pos_to_grid(pos.x, pos.y);

        for (int y = -brush_radius; y <= brush_radius; y++)
        {
            for (int x = -brush_radius; x <= brush_radius; x++)
            {
                sandbox.set_cell(gx + x, gy + y, current_cell);
            }
        }
    }
    else if (IsMouseButtonDown(1))
    {
        Vector2 pos = GetScreenToWorld2D(GetMousePosition(), camera);
        auto [gx, gy] = sandbox.pos_to_grid(pos.x, pos.y);

        for (int y = -brush_radius; y <= brush_radius; y++)
        {
            for (int x = -brush_radius; x <= brush_radius; x++)
            {
                sandbox.set_cell(gx + x, gy + y, Cell());
            }
        }
    }

    camera.target = movement;
}

Rectangle handle_camera_view(const Camera2D& camera)
{
    const Vector2 topLeft = GetScreenToWorld2D({ 0, 0 }, camera);
    const Vector2 bottomRight = GetScreenToWorld2D({ (float)GetScreenWidth(), (float)GetScreenHeight() }, camera);

    return {
        topLeft.x,
        topLeft.y,
        bottomRight.x - topLeft.x,
        bottomRight.y - topLeft.y,
    };
}

void update_sandbox(ChunkManager& manager, const Camera2D& camera, bool debug_mode, float frame_time)
{
    auto view = handle_camera_view(camera);

    // update chunk
    manager.update<ChunkUpdater>(frame_time);

    BeginDrawing();
    ClearBackground(BLANK);

    // set up chunks
    manager.pre_draw(view);

    // within camera, draw chunks
    BeginMode2D(camera);
    manager.draw(view, debug_mode);
    EndMode2D();

    // global draw information
    DrawFPS(0, 0);
    DrawText(TextFormat("FrameTime: %.5f", GetFrameTime() * 1000), 0, 20, 20, RED);
    DrawText(TextFormat("Chunks Active: %d", manager.get_total_chunks()), 0, 40, 20, GREEN);

    EndDrawing();
}

int main()
{
    InitWindow(1280, 720, "Pixel Physics");

    ChunkManager sandbox;
    bool debug_mode = false;
    Cell current_cell;

    Vector2 movement = { 
        -(1280.0f / 2.0f),
         -(720.0f / 2.0f) 
    };
    
    Camera2D camera = { 
        { 0, 0 },
        { 0, 0 },
        0, 1.0f
    };

    while (!WindowShouldClose())
    {
        float frame_time = GetFrameTime();

        input(sandbox, current_cell, camera, movement, debug_mode, frame_time);

        update_sandbox(sandbox, camera, debug_mode, frame_time);
    }

    CloseWindow();
}