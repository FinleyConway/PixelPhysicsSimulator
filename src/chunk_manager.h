#pragma once

#include <cmath>
#include <vector>
#include <unordered_map>

#include "cell_chunk.h"
#include "raylib.h"

template<int TWidth, int THeight, int TCellSize>
class ChunkManager
{
using Chunk = CellChunk<TWidth, THeight, TCellSize>;

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

    void pre_draw(const Rectangle& view)
    {
        PROFILE_FUNCTION();

        for (auto* chunk : m_chunks)
        {
            if (is_chunk_in_view(chunk, view))
            {
                chunk->pre_draw();
            }
        }
    }

    void draw(const Rectangle& view, bool debug = false)
    {
        PROFILE_FUNCTION();

        for (auto* chunk : m_chunks)
        {
            if (is_chunk_in_view(chunk, view))
            {
                chunk->draw(debug);
            }
        }
    }

    const Cell& get_cell(Point grid_position)
    {
        const Point chunk_position = grid_to_chunk(grid_position);
        const Point local_position = grid_to_chunk_local(grid_position);

        return get_chunk(chunk_position)->get_cell(local_position);
    }    

    void set_cell(Point grid_position, const Cell& cell)
    {
        const Point chunk_position = grid_to_chunk(grid_position);
        const Point local_position = grid_to_chunk_local(grid_position);

        auto* chunk = get_chunk(chunk_position);
        Point notify;

        if (local_position.x == 0)           notify.x = -1;
        if (local_position.x == TWidth - 1)  notify.x = +1;
        if (local_position.y == 0)           notify.y = -1;
        if (local_position.y == THeight - 1) notify.y = +1;

        Point notify_offset = grid_position + notify;

        if (notify.x != 0)                  wake_up_chunk(notify_offset);
        if (notify.y != 0)                  wake_up_chunk(notify_offset);
        if (notify.x != 0 && notify.y != 0) wake_up_chunk(notify_offset);

        chunk->set_cell(local_position, cell);
    }

    bool is_empty(Point grid_position) const
    {
        const Point chunk_position = grid_to_chunk(grid_position);
        const Point local_position = grid_to_chunk_local(grid_position);

        if (m_chunk_lookup.contains(chunk_position))
        {
            return m_chunk_lookup.at(chunk_position)->is_empty(local_position);
        }

        return true;
    }

    void wake_up_chunk(Point grid_position)
    {
        const Point chunk_position = grid_to_chunk(grid_position);
        const Point local_position = grid_to_chunk_local(grid_position);

        get_chunk(chunk_position)->wake_up(local_position);
    }

    size_t get_total_chunks() const
    {
        return m_chunks.size();
    }

public:
    Point pos_to_grid(float x, float y) const
    {
        return { 
            static_cast<int>(std::floor(x / TCellSize)), 
            static_cast<int>(std::floor(y / TCellSize)) 
        };
    }    

    Point grid_to_chunk(Point grid_position) const
    {
        return { 
            grid_position.x >= 0 ? grid_position.x / TWidth : (grid_position.x - TWidth + 1) / TWidth,
            grid_position.y >= 0 ? grid_position.y / THeight : (grid_position.y - THeight + 1) / THeight,
        };
    }

    Point grid_to_chunk_local(Point grid_position) const
    {
        return {
            ((grid_position.x % TWidth + TWidth) % TWidth),
            ((grid_position.y % THeight + THeight) % THeight)
        };
    }

    Point world_to_chunk(Point world_position) const
    {
        return {
            static_cast<int>(std::floor(world_position.x / (TWidth * TCellSize))),
            static_cast<int>(std::floor(world_position.y / (THeight * TCellSize)))
        };
    }

private:
    Chunk* create_chunk(Point chunk_position)
    {
        PROFILE_FUNCTION();

        Point position = {
            chunk_position.x * TWidth * TCellSize,
            chunk_position.y * THeight * TCellSize
        };

        auto* chunk = new Chunk(position);

        m_chunk_lookup.try_emplace(chunk_position, chunk);
        m_chunks.emplace_back(chunk);

        return chunk;
    }

    Chunk* get_chunk(Point chunk_position)
    {
        PROFILE_FUNCTION();

        if (m_chunk_lookup.contains(chunk_position))
        {
            return m_chunk_lookup.at(chunk_position);
        }

        return create_chunk(chunk_position); 
    }

    void remove_empty_chunks()
    {
        PROFILE_FUNCTION();

        for (auto it = m_chunks.begin(); it != m_chunks.end();)
        {
            Chunk* chunk = *it;

            if (chunk->should_remove())
            {
                const Point chunk_world_position = chunk->get_position();
                const Point chunk_position = world_to_chunk(chunk_world_position);

                m_chunk_lookup.erase(chunk_position);
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
    void update_chunk(const Chunk* chunk, Func update)
    {
        PROFILE_FUNCTION();

        const Point position = chunk->get_position();
        const IntRect& rect = chunk->get_current_rect();

        for (int x = rect.min_x; x <= rect.max_x; x++)
        {
            for (int y = rect.min_y; y <= rect.max_y; y++)
            {
                const Cell& cell = chunk->get_cell({ x, y });
                const Point world_position = {
                    x + (position.x / TCellSize),
                    y + (position.y / TCellSize)
                };

                update(cell, world_position);
            }
        }
    }    

    bool is_chunk_in_view(const Chunk* chunk, const Rectangle& view) 
    {
        const Point position = chunk->get_position();
        const Point size = { TWidth * TCellSize, THeight * TCellSize };
        const Rectangle chunkRect = { 
            static_cast<float>(position.x), 
            static_cast<float>(position.y), 
            static_cast<float>(size.x), 
            static_cast<float>(size.y) 
        };

        return CheckCollisionRecs(view, chunkRect);
    }

private:
    std::unordered_map<Point, Chunk*> m_chunk_lookup;
    std::vector<Chunk*> m_chunks;
};