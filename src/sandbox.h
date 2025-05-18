#pragma once

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <unordered_map>

#include "cell.h"
#include "cell_chunk.h"

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

            // update chunk
            chunk.flip();
            it++;
        }
    }

    void pre_draw()
    {
        PROFILE_FUNCTION();

        // slow iterating map
        // update all dirty chuks
        for (auto& [pos, chunk] : m_chunks)
        {
            chunk.pre_draw();
        }
    }

    void draw()
    {
        PROFILE_FUNCTION();

        // slow iterating map
        for (auto& [pos, chunk] : m_chunks)
        {
            chunk.draw();
        }
    }

    void debug_draw()
    {
        // slow iterating map
        for (auto& [pos, chunk] : m_chunks)
        {
            chunk.debug_draw();
        }
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

private:
    CellChunk<Width, Height, CellSize>& get_or_create_chunk(int32_t chunk_x, int32_t chunk_y)
    {
        const auto& [it, inserted] = m_chunks.try_emplace(
            { chunk_x, chunk_y }, 
            chunk_x * Width * CellSize, chunk_y * Height * CellSize
        );

        return it->second;
    }

private:
    std::unordered_map<std::pair<int32_t, int32_t>, CellChunk<Width, Height, CellSize>, hash_pair<int32_t, int32_t>> m_chunks;
};