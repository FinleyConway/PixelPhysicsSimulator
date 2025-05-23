#pragma once

#include <cmath>
#include <vector>
#include <unordered_map>

#include "instrumentor.h"
#include "point.h"
#include "raylib.h"
#include "chunk.h"
#include "chunk_context.h"

class ChunkManager
{
public:
    ~ChunkManager()
    {
        for (auto* chunk : m_chunks)
        {
            delete chunk;
        }
    }

    const Cell& get_cell(int x, int y)
    {
        const Point chunk_position = grid_to_chunk(x, y);
        const Point local_position = grid_to_chunk_local(x, y);

        return get_chunk(chunk_position)->get_cell(local_position);
    }    

    void set_cell(int x, int y, const Cell& cell)
    {
        const Point chunk_position = grid_to_chunk(x, y);
        const Point local_position = grid_to_chunk_local(x, y);

        auto* chunk = get_chunk(chunk_position);
        Point notify;

        if (local_position.x == 0)            notify.x = -1;
        if (local_position.x == c_width - 1)  notify.x = +1;
        if (local_position.y == 0)            notify.y = -1;
        if (local_position.y == c_height - 1) notify.y = +1;

        if (notify.x != 0)                  wake_up_chunk(x + notify.x, y);
        if (notify.y != 0)                  wake_up_chunk(x, y + notify.y);
        if (notify.x != 0 && notify.y != 0) wake_up_chunk(x + notify.x, y + notify.y);

        chunk->set_cell(local_position, cell);
    }

    void wake_up_chunk(int x, int y)
    {
        const Point chunk_position = grid_to_chunk(x, y);
        const Point local_position = grid_to_chunk_local(x, y);

        get_chunk(chunk_position)->wake_up(local_position);
    }

    bool is_empty(int x, int y) const
    {
        const Point chunk_position = grid_to_chunk(x, y);
        const Point local_position = grid_to_chunk_local(x, y);

        if (m_chunk_lookup.contains(chunk_position))
        {
            return m_chunk_lookup.at(chunk_position)->is_empty(local_position);
        }

        return true;
    }

    size_t get_total_chunks() const
    {
        return m_chunks.size();
    }

public:
    // template<typename Func>
    // void update(Func update)
    // {
    //     PROFILE_FUNCTION();

    //     for (auto* chunk : m_chunks)
    //     {
    //         update_chunk(chunk, update);
    //     }

    //     for (auto* chunk : m_chunks)
    //     {
    //         chunk->apply_cells();
    //     }

    //     for (auto* chunk : m_chunks)
    //     {
    //         chunk->update_rect();
    //     }

    //     remove_empty_chunks();
    // }

    template<typename Func>
    void update()
    {
        PROFILE_FUNCTION();

        for (auto* chunk : m_chunks)
        {
            //update_chunk(chunk, update);
            auto tmp = Func(*this, chunk);
            tmp.update_chunk();
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
            if (is_chunk_in_view(chunk, view))
            {
                chunk->pre_draw();
            }
        }
    }

    void draw(const Camera2D& camera, bool debug = false)
    {
        PROFILE_FUNCTION();

        const Rectangle view = handle_camera_view(camera);

        for (const auto* chunk : m_chunks)
        {
            if (is_chunk_in_view(chunk, view))
            {
                chunk->draw(debug);
            }
        }
    }

public:
    Point pos_to_grid(float x, float y) const
    {
        return { 
            static_cast<int>(std::floor(x / c_cell_size)), 
            static_cast<int>(std::floor(y / c_cell_size)) 
        };
    }    

    Point grid_to_chunk(int x, int y) const
    {
        return { 
            x >= 0 ? x / c_width : (x - c_width + 1) / c_width,
            y >= 0 ? y / c_height : (y - c_height + 1) / c_height,
        };
    }

    Point grid_to_chunk_local(int x, int y) const
    {
        return {
            ((x % c_width + c_width) % c_width),
            ((y % c_height + c_height) % c_height)
        };
    }

    Point world_to_chunk(float x, float y) const
    {
        return {
            static_cast<int>(std::floor(x / (c_width * c_cell_size))),
            static_cast<int>(std::floor(y / (c_height * c_cell_size)))
        };
    }

private:
    Chunk* create_chunk(Point chunk_position)
    {
        PROFILE_FUNCTION();

        const Point position = {
            chunk_position.x * c_width * c_cell_size,
            chunk_position.y * c_height * c_cell_size,
        };

        auto* chunk = new Chunk(position);

        m_chunk_lookup.try_emplace(chunk_position, chunk);
        m_chunks.emplace_back(chunk);

        return chunk;
    }

    Chunk* get_chunk(Point chunk_position)
    {
        if (m_chunk_lookup.contains(chunk_position))
        {
            return m_chunk_lookup.at(chunk_position);
        }

        return create_chunk(chunk_position); 
    }

    void remove_empty_chunks()
    {
        for (auto it = m_chunks.begin(); it != m_chunks.end();)
        {
            Chunk* chunk = *it;

            if (chunk->should_remove())
            {
                const Point position = chunk->get_position();
                const Point chunk_position = world_to_chunk(position.x, position.y);

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
    void update_chunk(Chunk* chunk, Func update)
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
                    x + (position.x / c_cell_size),
                    y + (position.y / c_cell_size)
                };

                update(cell, world_position.x, world_position.y);
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

    bool is_chunk_in_view(const Chunk* chunk, const Rectangle& view)
    {
        const Point position = chunk->get_position();
        const Point size = { c_width * c_cell_size, c_height * c_cell_size };
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

    static constexpr int c_width = ChunkContext::width;
    static constexpr int c_height = ChunkContext::height;
    static constexpr int c_cell_size = ChunkContext::cell_size;
};