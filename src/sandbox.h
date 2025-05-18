#pragma once

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <unordered_map>

#include "cell.h"
#include "cell_chunk.h"
#include "raylib.h"

// temp, just to get intellisense
#define Width 64
#define Height 64
#define CellSize 8

template<typename T1, typename T2>
struct hash_pair
{
    size_t operator()(std::pair<T1, T2> v) const noexcept
    {
        // https://stackoverflow.com/a/55083395
        size_t hash = std::hash<int32_t>()(v.first);
        hash <<= sizeof(size_t) * 4;
        hash ^= std::hash<int32_t>()(v.first);
        return std::hash<size_t>()(hash);
    }
};

//template<size_t Width, size_t Height, size_t CellSize>
class Sandbox
{
public:
    const Cell& get_cell(int32_t x, int32_t y)
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        auto [local_x, local_y] = grid_to_chunk_local(x, y);

        return get_or_create_chunk(chunk_x, chunk_y).get_cell(local_x, local_y);
    }

    void set_cell(int32_t x, int32_t y, const Cell& cell)
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        auto [local_x, local_y] = grid_to_chunk_local(x, y);

        get_or_create_chunk(chunk_x, chunk_y).set_cell(local_x, local_y, cell);
    }

    bool has_cell(int32_t x, int32_t y) const
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        auto [local_x, local_y] = grid_to_chunk_local(x, y);

        if (m_chunks.contains({ chunk_x, chunk_y }))
        {
            return m_chunks.at({ chunk_x, chunk_y }).has_cell(local_x, local_y);
        }

        return false;
    }

    bool has_empty_cell(int32_t x, int32_t y) const
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        auto [local_x, local_y] = grid_to_chunk_local(x, y);

        if (m_chunks.contains({ chunk_x, chunk_y }))
        {
            return m_chunks.at({ chunk_x, chunk_y }).is_empty_cell(local_x, local_y);
        }

        return true;
    }

public:
    template<typename Func>
    void update(Func update)
    {
        PROFILE_FUNCTION();

        // slow iterating map
        for (auto it = m_chunks.begin(); it != m_chunks.end(); )
        {
            auto& chunk = it->second;

            // update the current chunk
            for (int x = chunk.m_dirty_rect.min_x; x <= chunk.m_dirty_rect.max_x; x++)
            {
                for (int y = chunk.m_dirty_rect.min_y; y <= chunk.m_dirty_rect.max_y; y++)
                {
                    const Cell& cell = chunk.get_cell(x, y);

                    int32_t world_x = x + (chunk.m_position_x / CellSize);
                    int32_t world_y = y + (chunk.m_position_y / CellSize);

                    update(cell, world_x, world_y);
                }
            }

            chunk.flip();

            if (chunk.should_die())
            {
                it = m_chunks.erase(it);
            }
            else 
            {
                it++;
            }
        }
    }

    void pre_draw(const Camera2D& camera)
    {
        PROFILE_FUNCTION();

        handle_chunk_culling(camera, [&](int x, int y)
        {
            if (m_chunks.contains({ x, y }))
            {
                auto& chunk = m_chunks.at({ x, y });

                chunk.pre_draw();
            }
        });
    }

    void draw(const Camera2D& camera, bool debug = false)
    {
        PROFILE_FUNCTION();

        handle_chunk_culling(camera, [&](int x, int y)
        {
            if (m_chunks.contains({ x, y }))
            {
                auto& chunk = m_chunks.at({ x, y });

                chunk.draw();

                if (debug) chunk.debug_draw();
            }
        });
    }

public:
    std::pair<int32_t, int32_t> pos_to_grid(float x, float y) const
    {
        return { 
            std::floor(x / CellSize), 
            std::floor(y / CellSize) 
        };
    }    

    std::pair<int32_t, int32_t> grid_to_chunk(int32_t x, int32_t y) const
    {
        return { 
            x >= 0 ? x / Width : (x - Width + 1) / Width,
            y >= 0 ? y / Height : (y - Height + 1) / Height,
        };
    }

    std::pair<int32_t, int32_t> grid_to_chunk_local(int32_t x, int32_t y) const
    {
        return {
            ((x % Width + Width) % Width),
            ((y % Height + Height) % Height)
        };
    }

    std::pair<int32_t, int32_t> world_to_chunk(float x, float y) const
    {
        return {
            static_cast<int32_t>(std::floor(x / (Width * CellSize))),
            static_cast<int32_t>(std::floor(y / (Height * CellSize)))
        };
    }

private:
    CellChunk<Width, Height, CellSize>& get_or_create_chunk(int32_t chunk_x, int32_t chunk_y)
    {
        const auto& [it, inserted] = m_chunks.try_emplace(
            { chunk_x, chunk_y }, 
            chunk_x * Width * CellSize, chunk_y * Height * CellSize
        );

        return it->second;
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

    template<typename Func>
    void handle_chunk_culling(const Camera2D& camera, Func draw)
    {
        if (m_chunks.empty()) return;

        auto view = handle_camera_view(camera);
        auto [min_chunk_x, min_chunk_y] = world_to_chunk(view.x, view.y);
        auto [max_chunk_x, max_chunk_y] = world_to_chunk(view.x + view.width, view.y + view.height);

        for (int x = min_chunk_x; x <= max_chunk_x; x++)
        {
            for (int y = min_chunk_y; y <= max_chunk_y; y++)
            {
                draw(x, y);
            }
        }
    }

private:
    std::unordered_map<std::pair<int32_t, int32_t>, CellChunk<Width, Height, CellSize>, hash_pair<int32_t, int32_t>> m_chunks;
};