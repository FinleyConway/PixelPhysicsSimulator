#pragma once

#include <vector>
#include <unordered_map>
#include "raylib.h"

#include "simulator/cell_chunk.h"
#include "utils/instrumentor.h"

class ChunkManager
{
public:
    ChunkManager() = default;
    ~ChunkManager();

    template<typename ChunkHandler>
    void update()
    {
        PROFILE_FUNCTION();

        for (auto* chunk : m_chunks)
        {
            auto handle = ChunkHandler(*this, chunk);
            handle.update_chunk();
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

    void pre_draw(const Rectangle& view);
    void draw(const Rectangle& view, bool debug = false);

    const Cell& get_cell(Point grid_position);
    void set_cell(Point grid_position, const Cell& cell);
    bool is_empty(Point grid_position) const;
    void wake_up_chunk(Point grid_position);
    
    size_t get_total_chunks() const;

    Point pos_to_grid(float x, float y) const;
    Point grid_to_chunk(Point grid_position) const;
    Point grid_to_chunk_local(Point grid_position) const;
    Point world_to_chunk(Point world_position) const;

private:
    CellChunk* create_chunk(Point chunk_position);
    CellChunk* get_chunk(Point chunk_position);
    void remove_empty_chunks();
    bool is_chunk_in_view(const CellChunk* chunk, const Rectangle& view);

private:
    std::unordered_map<Point, CellChunk*> m_chunk_lookup;
    std::vector<CellChunk*> m_chunks;
};