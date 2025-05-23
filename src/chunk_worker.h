#pragma once

#include "simulator/cell_chunk.h"
#include "simulator/chunk_manager.h"
#include "simulator/chunk_updater.h"

class ChunkWorker : public ChunkUpdater
{
public:
    ChunkWorker(ChunkManager& manager, CellChunk* chunk)
        : ChunkUpdater(manager, chunk) {}

protected:
    void update(const Cell& cell, Point position) override
    {
        auto [x, y] = position;

        if (cell.type == CellType::Sand)
        {
            int dir = rand() % 2 ? -1 : 1;

            if (is_empty({ x, y + 1 }))
            {
                set_cell({ x, y }, { CellType::Empty, BLANK });
                set_cell({ x, y + 1 }, cell);
            }
            else if (this->is_empty({ x + dir, y + 1 }))
            {
                set_cell({ x, y }, { CellType::Empty, BLANK });
                set_cell({ x + dir, y + 1 }, cell);
            }
        }
    }
};
