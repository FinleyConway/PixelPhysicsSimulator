#pragma once

#include <unordered_map>
#include <cassert>

#include <raylib.h>

#include "utils/point.hpp"
#include "simulation/chunk.hpp"

struct ChunkManagerSpec
{
    Point chunk_size = { 16, 16 };
    int chunk_cell_size = 4;

    Point min_chunk_pos = { -5, -5 };
    Point max_chunk_pos = { +5, +5 }; 
};

class PixelWorld
{
public:
    PixelWorld(const ChunkManagerSpec& spec = ChunkManagerSpec());
    ~PixelWorld();

    const Cell* get_cell(int x, int y);  
    void set_cell(int x, int y, const Cell& cell);
    void move_cell(int from_x, int from_y, int to_x, int to_y, bool swap = false);
    bool is_empty(int x, int y) const;

    Point get_chunk_size() const;
    int get_chunk_cell_size() const;
    size_t get_total_chunks() const;

public:
    template<typename TChunkWorker>
    void update(float delta_time)
    {
        m_accumulator += delta_time;

        // update world at a fixed rate 
        while (m_accumulator > c_time_step)
        {
            // apply cell logic
            for (auto* chunk : m_chunks)
            {
                assert(chunk != nullptr);

                auto tmp = TChunkWorker(*this, chunk);
                tmp.update_chunk(c_time_step);
            }

            // apply moved cells to grid
            for (auto* chunk : m_chunks)
            {
                chunk->apply_moved_cells();
            }

            // update the bounds
            for (auto* chunk : m_chunks)
            {
                chunk->update_rect();
            }

            // remove any empty chunks
            remove_empty_chunks();

            m_accumulator -= c_time_step;
        }
    }

    void pre_draw(const Rectangle& view);
    void draw(const Rectangle& view, bool debug = false);

public:
    Point pos_to_grid(float x, float y) const;
    Point grid_to_chunk(int x, int y) const;
    Point grid_to_chunk_local(int x, int y) const;
    Point world_to_chunk(float x, float y) const;

private:
    int get_max_chunks() const;
    bool in_world_bounds(const Point& chunk_position);
    bool is_chunk_in_view(const Chunk* chunk, const Rectangle& view) const;

    Chunk* create_chunk(Point chunk_position);
    Chunk* get_chunk_or_create(Point chunk_position);
    void remove_empty_chunks();
    void wake_up_chunk(int x, int y);

private:
    const float c_time_step = 1.0f / 60.0f;
    float m_accumulator = 0;

    Point m_chunk_size;
    int m_chunk_cell_size = 0;
    Point m_min_chunk_pos;
    Point m_max_chunk_pos;

    std::unordered_map<Point, Chunk*> m_chunk_lookup;
    std::vector<Chunk*> m_chunks;
};