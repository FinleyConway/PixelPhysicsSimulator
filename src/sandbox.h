#pragma once

#include <cmath>
#include <cstdint>
#include <unordered_map>

#include "cell_chunk.h"

// temp, just to get intellisense
#define Width 10
#define Height 10

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

//template<size_t Width, size_t Height>
class Sandbox
{
public:
    size_t get_chunk_count() const { return m_Chunks.size(); }

    CellChunk<Width, Height>& create_chunk(int32_t chunk_x, int32_t chunk_y)
    {
        if (has_chunk(chunk_x, chunk_y))
        {
            return get_chunk(chunk_x, chunk_y);
        }

        m_Chunks.try_emplace({ chunk_x, chunk_y }, chunk_x, chunk_y);

        return get_chunk(chunk_x, chunk_y);
    }

    bool has_chunk(int32_t chunk_x, int32_t chunk_y) const
    {
        return m_Chunks.contains({ chunk_x, chunk_y });
    }

    const CellChunk<Width, Height>& get_chunk(int32_t chunk_x, int32_t chunk_y) const
    {
        return m_Chunks.at({ chunk_x, chunk_y });
    }

    CellChunk<Width, Height>& get_chunk(int32_t chunk_x, int32_t chunk_y)
    {
        return m_Chunks.at({ chunk_x, chunk_y });
    }

    bool remove_chunk(int32_t chunk_x, int32_t chunk_y)
    {
        if (has_chunk(chunk_x, chunk_y))
        {
            m_Chunks.erase({ chunk_x, chunk_y });

            return true;
        }

        return false;
    }

    void set_cell(int32_t x, int32_t y, const Cell& cell)
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        auto [local_x, local_y] = grid_to_chunk_local(x, y);

        if (has_chunk(chunk_x, chunk_y))
        {
            auto& chunk = get_chunk(chunk_x, chunk_y);

            chunk.set_cell(local_x, local_y, cell);
        }
        else 
        {
            auto& chunk = create_chunk(chunk_x, chunk_y);

            chunk.set_cell(local_x, local_y, cell);
        }
    }

    std::pair<int32_t, int32_t> pos_to_grid(float x, float y)
    {
        return { 
            std::floor(x / Width), 
            std::floor(y / Height) 
        };
    }    

    std::pair<int32_t, int32_t> grid_to_chunk(int32_t x, int32_t y)
    {
        return { 
            x >= 0 ? x / Width : (x - Width + 1) / Width,
            y >= 0 ? y / Height : (y - Height + 1) / Height,
        };
    }

    std::pair<int32_t, int32_t> grid_to_chunk_local(int32_t x, int32_t y)
    {
        return {
            ((x % Width + Width) % Width),
            ((y % Height + Height) % Height)
        };
    }

private:
    std::unordered_map<std::pair<int32_t, int32_t>, CellChunk<Width, Height>, hash_pair<int32_t, int32_t>> m_Chunks;
};