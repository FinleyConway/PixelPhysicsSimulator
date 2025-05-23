#pragma once

#include "simulator/cell.h"
#include "simulator/cell_chunk.h"
#include "simulator/chunk_manager.h"
#include "utils/point.h"

class ChunkUpdater
{
public:
    ChunkUpdater(ChunkManager& manager, CellChunk* chunk);
    virtual ~ChunkUpdater() = default;

    void update_chunk();

protected:
    virtual void update(const Cell& cell, Point position) = 0;

    const Cell& get_cell(Point position);
    void set_cell(Point position, const Cell& cell);
    bool is_empty(Point position) const;

private:
    ChunkManager& m_manager;
    CellChunk* m_chunk = nullptr;
};
