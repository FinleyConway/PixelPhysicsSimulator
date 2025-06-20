#pragma once

#include "simulation/chunk.hpp"
#include "simulation/chunk_manager.hpp"

class ChunkWorker
{
public:
    ChunkWorker(ChunkManager& manager, Chunk* chunk);
    virtual ~ChunkWorker() = default;

    void update_chunk(float time_step);

protected:
    virtual void update_cell(const Cell& cell, int x, int y) = 0;

    const Cell* get_cell(int x, int y);
    void set_cell(int x, int y, const Cell& cell);
    void move_cell(int from_x, int from_y, int to_x, int to_y);
    void push_cell(int from_x, int from_y, int dir_x, int dir_y);
    void swap_cells(int from_x, int from_y, int to_x, int to_y);
    bool is_empty(int x, int y) const;

private:
    void handle_life_time(Cell& cell, int x, int y, float time_step);

private:
    ChunkManager& m_manager;
    Chunk* m_chunk = nullptr;
};