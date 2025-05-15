#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <unordered_map>

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

        if (has_chunk(chunk_x, chunk_y))
        {
            return get_chunk(chunk_x, chunk_y).has_cell(local_x, local_y);
        }

        return false;
    }

    bool has_empty_cell(int32_t x, int32_t y) const
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        auto [local_x, local_y] = grid_to_chunk_local(x, y);

        if (has_chunk(chunk_x, chunk_y))
        {
            return get_chunk(chunk_x, chunk_y).is_empty_cell(local_x, local_y);
        }

        return true;
    }

    template<typename Func>
    void update(Func&& update)
    {
        // slow iterating map
        for (auto& [pos, chunk] : m_chunks)
        {
            for (size_t x = 0; x < Width; x++)
            {
                for (size_t y = 0; y < Height; y++)
                {
                    const Cell& cell = chunk.get_cell(x, y);

                    size_t world_x = x + chunk.m_position_x;
                    size_t world_y = y + chunk.m_position_y;

                    std::forward<Func>(update)(cell, world_x, world_y);
                }
            }

            chunk.flip();
        }
    }

    void pre_draw()
    {
        // slow iterating map
        // update all dirty chuks
        for (auto& [pos, chunk] : m_chunks)
        {
            chunk.pre_draw();
        }
    }

    void draw()
    {
        // slow iterating map
        for (auto& [pos, chunk] : m_chunks)
        {
            chunk.draw();
        }
    }

    size_t get_chunk_count() const { return m_chunks.size(); }

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

    const CellChunk<Width, Height, CellSize>& get_chunk(int32_t chunk_x, int32_t chunk_y) const
    {
        return m_chunks.at({ chunk_x, chunk_y });
    }

    bool has_chunk(int32_t chunk_x, int32_t chunk_y) const
    {
        return m_chunks.contains({ chunk_x, chunk_y });
    }

    bool remove_chunk(int32_t chunk_x, int32_t chunk_y)
    {
        return m_chunks.erase({ chunk_x, chunk_y }) > 0;
    }

private:
    std::unordered_map<std::pair<int32_t, int32_t>, CellChunk<Width, Height, CellSize>, hash_pair<int32_t, int32_t>> m_chunks;
};