#include "raylib.h"

#include "chunk_worker.h"
#include "chunk_manager.h"

class TestWorker : public ChunkWorker
{
public:
    TestWorker(ChunkManager& manager, Chunk* chunk) : ChunkWorker(manager, chunk) { }

protected:
    void update_cell(const Cell& cell, int x, int y)
    {
        bool up = is_empty(x, y - 1);
        bool up_left = is_empty(x - 1, y - 1);
        bool up_right = is_empty(x + 1, y - 1);

        bool left = is_empty(x - 1, y);
        bool right = is_empty(x + 1, y);

        bool down = is_empty(x, y + 1);
        bool down_left = is_empty(x - 1, y + 1);
        bool down_right = is_empty(x + 1, y + 1);

        if (cell.type == CellType::Sand)
        {
            if (down)
            {
                move_cell(x, y, x, y + 1, cell);
            }
            else if (down_left || down_right)
            {
                bool options[2] = { false, false };

                if (down_left) options[0] = true;
                if (down_right) options[1] = true;

                if (options[0] && options[1])
                {
                    int r = rand() % 2;

                    if (r == 0) move_cell(x, y, x - 1, y + 1, cell); 
                    if (r == 1) move_cell(x, y, x + 1, y + 1, cell); 
                }
                else if (options[0])
                {
                    move_cell(x, y, x - 1, y + 1, cell); 
                }
                else if (options[1])
                {
                    move_cell(x, y, x + 1, y + 1, cell); 
                }
            }
        }

        if (cell.type == CellType::Water)
        {
            if (down)
            {
                move_cell(x, y, x, y + 1, cell);
            }
            else if (down_left || down_right)
            {
                bool options[2] = { false, false };

                if (down_left) options[0] = true;
                if (down_right) options[1] = true;

                if (options[0] && options[1])
                {
                    int r = rand() % 2;

                    if (r == 0) move_cell(x, y, x - 1, y + 1, cell); 
                    if (r == 1) move_cell(x, y, x + 1, y + 1, cell); 
                }
                else if (options[0])
                {
                    move_cell(x, y, x - 1, y + 1, cell); 
                }
                else if (options[1])
                {
                    move_cell(x, y, x + 1, y + 1, cell); 
                }
            }
            else if (left || right)
            {
                bool options[2] = { false, false };

                if (left) options[0] = true;
                if (right) options[1] = true;

                if (options[0] && options[1])
                {
                    int r = rand() % 2;

                    if (r == 0) move_cell(x, y, x - 1, y, cell); 
                    if (r == 1) move_cell(x, y, x + 1, y, cell); 
                }
                else if (options[0])
                {
                    move_cell(x, y, x - 1, y, cell); 
                }
                else if (options[1])
                {
                    move_cell(x, y, x + 1, y, cell); 
                }
            }
        }
    }
};

void raylib()
{
    InitWindow(1280, 720, "Pixel Physics");

    ChunkManager sandbox;
    Vector2 movement = { 0 - 1280 / 2, 0 - 720 / 2 };
    Camera2D camera = { 0 };
    camera.target = { 0, 0 };
    camera.offset = { 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f; 

    float time_step = 1.0f / 60.0f;
    float accumulator = 0;
    bool debug_mode = false;

    Cell current_cell = Cell();

    while (!WindowShouldClose())
    {
        float frame_time = GetFrameTime();
        accumulator += frame_time;

        if (IsKeyDown(KEY_D)) movement.x += 512.0f * frame_time;
        if (IsKeyDown(KEY_A)) movement.x -= 512.0f * frame_time;
        if (IsKeyDown(KEY_W)) movement.y -= 512.0f * frame_time;
        if (IsKeyDown(KEY_S)) movement.y += 512.0f * frame_time;
        
        const int brush_radius = 2;

        if (IsKeyPressed(KEY_ONE))   current_cell = { CellType::Sand, YELLOW };
        if (IsKeyPressed(KEY_TWO))   current_cell = { CellType::Water, BLUE };
        if (IsKeyPressed(KEY_THREE)) current_cell = { CellType::Stone, GRAY };

        if (IsKeyPressed(KEY_F1)) debug_mode = !debug_mode;

        if (IsMouseButtonDown(0))
        {
            Vector2 pos = GetMousePosition();
            pos = GetScreenToWorld2D(pos, camera);

            auto [gx, gy] = sandbox.pos_to_grid(pos.x, pos.y);

            for (int y = -brush_radius; y <= brush_radius; ++y)
            {
                for (int x = -brush_radius; x <= brush_radius; ++x)
                {
                    sandbox.set_cell(gx + x, gy + y, current_cell);
                }
            }
        }
        else if (IsMouseButtonDown(1))
        {
            Vector2 pos = GetMousePosition();
            pos = GetScreenToWorld2D(pos, camera);

            auto [gx, gy] = sandbox.pos_to_grid(pos.x, pos.y);

            for (int y = -brush_radius; y <= brush_radius; ++y)
            {
                for (int x = -brush_radius; x <= brush_radius; ++x)
                {
                    sandbox.set_cell(gx + x, gy + y, Cell());
                }
            }
        }

        camera.target = movement;

        // update every 1/60th
        while (accumulator > time_step)
        {
            sandbox.update<TestWorker>();
            accumulator -= time_step;
        }

        BeginDrawing();
        ClearBackground(BLANK);

        sandbox.pre_draw(camera);

        // within camera draw
        BeginMode2D(camera);
        sandbox.draw(camera, debug_mode);
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
    raylib();
}