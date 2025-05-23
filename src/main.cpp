#include "raylib.h"


#include "simulator/chunk_manager.h"
#include "utils/instrumentor.h"
#include "chunk_worker.h"

Rectangle handle_camera_view(const Camera2D& camera)
{
    Vector2 topLeft = GetScreenToWorld2D({ 0, 0 }, camera);
    Vector2 bottomRight = GetScreenToWorld2D({ (float)GetScreenWidth(), (float)GetScreenHeight() }, camera);

    return {
        topLeft.x,
        topLeft.y,
        bottomRight.x - topLeft.x,
        bottomRight.y - topLeft.y,
    };
}

void raylib()
{
    InitWindow(1280, 720, "Pixel Physics");

    ChunkManager sandbox;
    Vector2 movement = { 0, 0 };
    Camera2D camera = { 0 };
    camera.target = { 0, 0 };
    camera.offset = { 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f; 

    while (!WindowShouldClose())
    {
        float frame_time = GetFrameTime();

        if (IsKeyDown(KEY_D)) movement.x += 512.0f * frame_time;
        if (IsKeyDown(KEY_A)) movement.x -= 512.0f * frame_time;
        if (IsKeyDown(KEY_W)) movement.y -= 512.0f * frame_time;
        if (IsKeyDown(KEY_S)) movement.y += 512.0f * frame_time;
        
        const int brush_radius = 2;

        if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_C))
        {
            Vector2 pos = GetMousePosition();
            pos = GetScreenToWorld2D(pos, camera);

            auto [gx, gy] = sandbox.pos_to_grid(pos.x, pos.y);

            CellType type = IsKeyDown(KEY_SPACE) ? CellType::Stone : CellType::Sand;
            Color color = IsKeyDown(KEY_SPACE) ? GRAY : YELLOW;

            for (int y = -brush_radius; y <= brush_radius; ++y)
            {
                for (int x = -brush_radius; x <= brush_radius; ++x)
                {
                    sandbox.set_cell({ gx + x, gy + y }, { type, color });
                }
            }
        }

        camera.target = movement;

        sandbox.update<ChunkWorker>();

        BeginDrawing();
        ClearBackground(BLANK);

        sandbox.pre_draw(handle_camera_view(camera));

        // within camera draw
        BeginMode2D(camera);
        sandbox.draw(handle_camera_view(camera), true);
        EndMode2D();

        // global draw
        DrawFPS(0, 0);
        DrawText(TextFormat("FrameTime: %.5f", GetFrameTime() * 1000), 0, 20, 20, RED);
        DrawText(TextFormat("Chunks Active: %d", sandbox.get_total_chunks()), 0, 40, 20, GREEN);


        EndDrawing();
    }

    CloseWindow();
}

int main()
{
    PROFILE_BEGIN_SESSION("Pixel", "result.json", 2048);

    raylib();

    PROFILE_END_SESSION();
}