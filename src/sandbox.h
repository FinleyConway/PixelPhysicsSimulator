#pragma once

#include <cmath>
#include <cstdint>
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
    void set_cell(int32_t x, int32_t y, const Cell& cell)
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        auto [local_x, local_y] = grid_to_chunk_local(x, y);
        auto& chunk = get_or_create_chunk(chunk_x, chunk_y);

        chunk.set_cell(local_x, local_y, cell);
    }

    bool has_cell(int32_t x, int32_t y) const
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        auto [local_x, local_y] = grid_to_chunk_local(x, y);

        if (has_chunk(chunk_x, chunk_y))
        {
            auto& chunk = get_chunk(chunk_x, chunk_y);

            return chunk.has_cell(local_x, local_y);
        }

        return false;
    }

    size_t get_chunk_count() const { return m_Chunks.size(); }

//private:
    CellChunk<Width, Height, CellSize>& get_or_create_chunk(int32_t chunk_x, int32_t chunk_y)
    {
        const auto& [it, inserted] = m_Chunks.try_emplace(
            { chunk_x, chunk_y }, 
            chunk_x, chunk_y
        );

        return it->second;
    }

    const CellChunk<Width, Height, CellSize>& get_chunk(int32_t chunk_x, int32_t chunk_y) const
    {
        return m_Chunks.at({ chunk_x, chunk_y });
    }

    bool has_chunk(int32_t chunk_x, int32_t chunk_y) const
    {
        return m_Chunks.contains({ chunk_x, chunk_y });
    }

    bool remove_chunk(int32_t chunk_x, int32_t chunk_y)
    {
        return m_Chunks.erase({ chunk_x, chunk_y }) > 0;
    }

//private:
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
    std::unordered_map<std::pair<int32_t, int32_t>, CellChunk<Width, Height, CellSize>, hash_pair<int32_t, int32_t>> m_Chunks;
};