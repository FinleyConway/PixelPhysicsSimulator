#pragma once

#include "chunk.h"
#include "chunk_manager.h"

class ChunkWorker
{
public:
    ChunkWorker(ChunkManager& manager, Chunk* chunk);
    virtual ~ChunkWorker() = default;

    void update_chunk();

protected:
    virtual void update_cell(const Cell& cell, int x, int y) = 0;

    const Cell* get_cell(int x, int y);
    void set_cell(int x, int y, const Cell& cell);
    void move_cell(int from_x, int from_y, int to_x, int to_y, const Cell& cell);
    bool is_empty(int x, int y) const;

private:
    ChunkManager& m_manager;
    Chunk* m_chunk = nullptr;
};