#pragma once

#include <cmath>
#include <vector>
#include <unordered_map>

#include "instrumentor.h"
#include "point.h"
#include "raylib.h"

#include "cell_chunk.h"

#define TWidth 64
#define THeight 64
#define TCellSize 4

//template<int TWidth, int THeight, int TCellSize>
class CellChunkManager
{
private:
    using Chunk = CellChunk<TWidth, THeight, TCellSize>;

public:
    ~CellChunkManager()
    {
        for (auto* chunk : m_chunks)
        {
            delete chunk;
        }
    }

    const Cell& get_cell(int x, int y)
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        const Point local_pos = grid_to_chunk_local(x, y);

        return get_chunk(chunk_x, chunk_y)->get_cell(local_pos);
    }    

    void set_cell(int x, int y, const Cell& cell)
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        const Point local_pos = grid_to_chunk_local(x, y);

        auto* chunk = get_chunk(chunk_x, chunk_y);
        Point notify;

        if (local_pos.x == 0)           notify.x = -1;
        if (local_pos.x == TWidth - 1)  notify.x = +1;
        if (local_pos.y == 0)           notify.y = -1;
        if (local_pos.y == THeight - 1) notify.y = +1;

        if (notify.x != 0)                  wake_up_chunk(x + notify.x, y);
        if (notify.y != 0)                  wake_up_chunk(x, y + notify.y);
        if (notify.x != 0 && notify.y != 0) wake_up_chunk(x + notify.x, y + notify.y);

        chunk->set_cell(local_pos, cell);
    }

    void wake_up_chunk(int x, int y)
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        const Point local_pos = grid_to_chunk_local(x, y);

        get_chunk(chunk_x, chunk_y)->wake_up(local_pos);
    }

    bool is_empty(int x, int y) const
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        const Point local_pos = grid_to_chunk_local(x, y);

        if (m_chunk_lookup.contains({ chunk_x, chunk_y }))
        {
            return m_chunk_lookup.at({ chunk_x, chunk_y })->is_empty(local_pos);
        }

        return true;
    }

    size_t get_total_chunks() const
    {
        return m_chunks.size();
    }

public:
    template<typename Func>
    void update(Func update)
    {
        PROFILE_FUNCTION();

        for (auto* chunk : m_chunks)
        {
            update_chunk(chunk, update);
        }

        for (auto* chunk : m_chunks)
        {
            chunk->apply_cells();
        }

        for (auto* chunk : m_chunks)
        {
            chunk->update_rect();
        }

        remove_empty_chunks();
    }

    void pre_draw(const Camera2D& camera)
    {
        PROFILE_FUNCTION();

        const Rectangle view = handle_camera_view(camera);

        for (auto* chunk : m_chunks)
        {
            const Point position = chunk->get_position();
            const Point size = { TWidth * TCellSize, THeight * TCellSize };
            const Rectangle chunkRect = { (float)position.x, (float)position.y, (float)size.x, (float)size.y };

            if (CheckCollisionRecs(view, chunkRect))
            {
                chunk->pre_draw(); 
            }
        }
    }

    void draw(const Camera2D& camera, bool debug = false)
    {
        PROFILE_FUNCTION();

        const Rectangle view = handle_camera_view(camera);

        for (auto* chunk : m_chunks)
        {
            const Point position = chunk->get_position();
            const Point size = { TWidth * TCellSize, THeight * TCellSize };
            const Rectangle chunkRect = { (float)position.x, (float)position.y, (float)size.x, (float)size.y };

            if (CheckCollisionRecs(view, chunkRect))
            {
                chunk->draw(debug);  
            }
        }
    }

public:
    Point pos_to_grid(float x, float y) const
    {
        return { 
            static_cast<int>(std::floor(x / TCellSize)), 
            static_cast<int>(std::floor(y / TCellSize)) 
        };
    }    

    Point grid_to_chunk(int x, int y) const
    {
        return { 
            x >= 0 ? x / TWidth : (x - TWidth + 1) / TWidth,
            y >= 0 ? y / THeight : (y - THeight + 1) / THeight,
        };
    }

    Point grid_to_chunk_local(int x, int y) const
    {
        return {
            ((x % TWidth + TWidth) % TWidth),
            ((y % THeight + THeight) % THeight)
        };
    }

    Point world_to_chunk(float x, float y) const
    {
        return {
            static_cast<int>(std::floor(x / (TWidth * TCellSize))),
            static_cast<int>(std::floor(y / (THeight * TCellSize)))
        };
    }

private:
    Chunk* create_chunk(int chunk_x, int chunk_y)
    {
        PROFILE_FUNCTION();

        int position_x = chunk_x * TWidth * TCellSize;
        int position_y = chunk_y * THeight * TCellSize;

        auto* chunk = new Chunk({ position_x, position_y });

        m_chunk_lookup.try_emplace({ chunk_x, chunk_y }, chunk);
        m_chunks.emplace_back(chunk);

        return chunk;
    }

    Chunk* get_chunk(int chunk_x, int chunk_y)
    {
        if (m_chunk_lookup.contains({ chunk_x, chunk_y }))
        {
            return m_chunk_lookup.at({ chunk_x, chunk_y });
        }

        return create_chunk(chunk_x, chunk_y); 
    }

    void remove_empty_chunks()
    {
        for (auto it = m_chunks.begin(); it != m_chunks.end();)
        {
            Chunk* chunk = *it;

            if (chunk->should_remove())
            {
                auto [px, py] = chunk->get_position();
                auto [wx, wy] = world_to_chunk(px, py);

                m_chunk_lookup.erase({ wx, wy });
                it = m_chunks.erase(it);

                delete chunk;
                chunk = nullptr;
            }
            else 
            {
                it++;
            }
        }
    }

    template<typename Func>
    void update_chunk(Chunk* chunk, Func update)
    {
        PROFILE_FUNCTION();

        const auto [position_x, position_y] = chunk->get_position();
        const IntRect& rect = chunk->get_current_rect();

        for (int x = rect.min_x; x <= rect.max_x; x++)
        {
            for (int y = rect.min_y; y <= rect.max_y; y++)
            {
                const Cell& cell = chunk->get_cell({ x, y });

                int world_x = x + (position_x / TCellSize);
                int world_y = y + (position_y / TCellSize);

                update(cell, world_x, world_y);
            }
        }
    }

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

private:
    std::unordered_map<Point, Chunk*> m_chunk_lookup;
    std::vector<Chunk*> m_chunks;
};