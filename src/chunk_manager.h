#pragma once

#include <cmath>
#include <unordered_map>
#include <boost/container/static_vector.hpp>

#include "instrumentor.h"
#include "point.h"
#include "raylib.h"
#include "chunk.h"
#include "chunk_context.h"

class ChunkManager
{
public:
    ChunkManager()
    {
        m_chunk_lookup.reserve(c_max_chunks);
    }

    ~ChunkManager()
    {
        for (auto* chunk : m_chunks)
        {
            delete chunk;
        }
    }

    const Cell* get_cell(int x, int y)
    {
        const Point chunk_position = grid_to_chunk(x, y);
        const Point local_position = grid_to_chunk_local(x, y);

        if (Chunk* chunk = get_chunk_or_create(chunk_position))
        {
            return &chunk->get_cell(local_position);
        } 

        return nullptr;
    }    

    void set_cell(int x, int y, const Cell& cell)
    {
        const Point chunk_position = grid_to_chunk(x, y);
        const Point local_position = grid_to_chunk_local(x, y);

        if (Chunk* chunk = get_chunk_or_create(chunk_position))
        {  
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
    }

    void wake_up_chunk(int x, int y)
    {
        const Point chunk_position = grid_to_chunk(x, y);
        const Point local_position = grid_to_chunk_local(x, y);

        if (m_chunk_lookup.contains(chunk_position))
        {
            m_chunk_lookup.at(chunk_position)->wake_up(local_position);
        }
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
    template<typename ChunkWorker>
    void update()
    {
        PROFILE_FUNCTION();

        for (auto* chunk : m_chunks)
        {
            assert(chunk != nullptr);

            auto tmp = ChunkWorker(*this, chunk);
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
            assert(chunk != nullptr);

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
            assert(chunk != nullptr);

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
    bool in_world_bounds(const Point& chunk_position)
    {
        return (
            chunk_position.x >= c_min_chunk_pos.x && 
            chunk_position.x <= c_max_chunk_pos.x &&
            chunk_position.y >= c_min_chunk_pos.y && 
            chunk_position.y <= c_max_chunk_pos.y
        );
    }

    Chunk* create_chunk(Point chunk_position)
    {
        PROFILE_FUNCTION();

        if (in_world_bounds(chunk_position))
        {
            const Point position = {
                chunk_position.x * c_width * c_cell_size,
                chunk_position.y * c_height * c_cell_size,
            };

            auto* chunk = new Chunk(position);

            auto [it, inserted] = m_chunk_lookup.try_emplace(chunk_position, chunk);

            if (inserted)
            {
                return m_chunks.emplace_back(chunk);
            }

            delete chunk; // i cant imagine we'll get here but who knows
        }

        return nullptr;
    }

    Chunk* get_chunk_or_create(Point chunk_position)
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
            }
            else 
            {
                it++;
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
    static constexpr int c_width = ChunkContext::width;
    static constexpr int c_height = ChunkContext::height;
    static constexpr int c_cell_size = ChunkContext::cell_size;

    static constexpr Point c_min_chunk_pos = ChunkContext::min_chunk_pos;
    static constexpr Point c_max_chunk_pos = ChunkContext::max_chunk_pos;
    static constexpr int c_max_chunks = ChunkContext::max_chunks;

private:
    std::unordered_map<Point, Chunk*> m_chunk_lookup;
    boost::container::static_vector<Chunk*, c_max_chunks> m_chunks;
};