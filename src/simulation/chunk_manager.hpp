#pragma once

#include <unordered_map>
#include <boost/container/static_vector.hpp>

#include <raylib.h>

#include "utils/point.hpp"
#include "simulation/chunk.hpp"
#include "core/chunk_context.hpp"

class ChunkManager
{
public:
    ChunkManager();
    ~ChunkManager();

    const Cell* get_cell(int x, int y);  
    void set_cell(int x, int y, const Cell& cell);
    void move_cell(int from_x, int from_y, int to_x, int to_y, bool swap = false);
    bool is_empty(int x, int y) const;

    size_t get_total_chunks() const;

public:
    template<typename ChunkWorker>
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

                auto tmp = ChunkWorker(*this, chunk);
                tmp.update_chunk();
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
    bool in_world_bounds(const Point& chunk_position);
    bool is_chunk_in_view(const Chunk* chunk, const Rectangle& view) const;

    Chunk* create_chunk(Point chunk_position);
    Chunk* get_chunk_or_create(Point chunk_position);
    void remove_empty_chunks();
    void wake_up_chunk(int x, int y);

private:
    static constexpr int c_width = ChunkContext::width;
    static constexpr int c_height = ChunkContext::height;
    static constexpr int c_cell_size = ChunkContext::cell_size;

    static constexpr Point c_min_chunk_pos = ChunkContext::min_chunk_pos;
    static constexpr Point c_max_chunk_pos = ChunkContext::max_chunk_pos;
    static constexpr int c_max_chunks = ChunkContext::max_chunks;

private:
    const float c_time_step = 1.0f / 60.0f;
    float m_accumulator = 0;

    std::unordered_map<Point, Chunk*> m_chunk_lookup;
    boost::container::static_vector<Chunk*, c_max_chunks> m_chunks;
};