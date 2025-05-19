#pragma once

#include <cmath>
#include <vector>
#include <utility>
#include <functional>
#include <unordered_map>

#include "raylib.h"

#include "cell_chunk.h"

template<typename T1, typename T2>
struct std::hash<std::pair<T1, T2>>
{
    size_t operator()(std::pair<T1, T2> p) const noexcept
    {
        // https://stackoverflow.com/a/55083395
        size_t hash = std::hash<T1>()(p.first);
        hash <<= sizeof(size_t) * 4;
        hash ^= std::hash<T2>()(p.second);

        return std::hash<size_t>()(hash);
    }
};

#define TWidth 64
#define THeight 64
#define TCellSize  4

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
        auto [local_x, local_y] = grid_to_chunk_local(x, y);

        return get_chunk(chunk_x, chunk_y)->get_cell(local_x, local_y);
    }    

    void set_cell(int x, int y, const Cell& cell)
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        auto [local_x, local_y] = grid_to_chunk_local(x, y);

        Chunk* chunk = get_chunk(chunk_x, chunk_y);
        
        chunk->set_cell(local_x, local_y, cell);
        chunk->keep_alive(local_x, local_y);
    }

    void stay_cell(int x, int y, const Cell& cell)
    {
        auto [chunk_x, chunk_y] = grid_to_chunk(x, y);
        auto [local_x, local_y] = grid_to_chunk_local(x, y);

        Chunk* chunk = get_chunk(chunk_x, chunk_y);
        
        chunk->set_cell(local_x, local_y, cell);
    }

public:
    template<typename Func>
    void update(Func update)
    {
        for (auto* chunk : m_chunks)
        {
            update_chunk(chunk, update);
        }

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

    void pre_draw()
    {
        for (auto* chunk : m_chunks)
        {
            chunk->pre_draw();
        }
    }

    void draw(const Camera2D& camera, bool debug = false)
    {
        for (auto* chunk : m_chunks)
        {
            chunk->draw(debug);
        }
    }

public:
    std::pair<int, int> pos_to_grid(float x, float y) const
    {
        return { 
            static_cast<int>(std::floor(x / TCellSize)), 
            static_cast<int>(std::floor(y / TCellSize)) 
        };
    }    

    std::pair<int, int> grid_to_chunk(int x, int y) const
    {
        return { 
            x >= 0 ? x / TWidth : (x - TWidth + 1) / TWidth,
            y >= 0 ? y / THeight : (y - THeight + 1) / THeight,
        };
    }

    std::pair<int, int> grid_to_chunk_local(int x, int y) const
    {
        return {
            ((x % TWidth + TWidth) % TWidth),
            ((y % THeight + THeight) % THeight)
        };
    }

    std::pair<int, int> world_to_chunk(float x, float y) const
    {
        return {
            static_cast<int>(std::floor(x / (TWidth * TCellSize))),
            static_cast<int>(std::floor(y / (THeight * TCellSize)))
        };
    }

private:
    Chunk* create_chunk(int chunk_x, int chunk_y)
    {
        int position_x = chunk_x * TWidth * TCellSize;
        int position_y = chunk_y * THeight * TCellSize;

        auto* chunk = new Chunk(position_x, position_y);

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

    template<typename Func>
    void update_chunk(Chunk* chunk, Func update)
    {
        const auto [position_x, position_y] = chunk->get_position();
        const IntRect& rect = chunk->get_current_rect();

        for (int x = rect.min_x; x <= rect.max_x; x++)
        {
            for (int y = rect.min_y; y <= rect.max_y; y++)
            {
                const Cell& cell = chunk->get_cell(x, y);

                int world_x = x + (position_x / TCellSize);
                int world_y = y + (position_y / TCellSize);

                update(cell, world_x, world_y);
            }
        }

        chunk->update();
    }

private:
    std::unordered_map<std::pair<int, int>, Chunk*> m_chunk_lookup;
    std::vector<Chunk*> m_chunks;
};