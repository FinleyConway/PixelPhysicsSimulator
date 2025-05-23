#pragma once

#include "chunk_updater.h"

template<int TWidth, int THeight, int TCellSize>
class ChunkWorker : public ChunkUpdater<TWidth, THeight, TCellSize>
{
using Manager = ChunkUpdater<TWidth, THeight, TCellSize>;
using Chunk = CellChunk<TWidth, THeight, TCellSize>;

public:
    ChunkWorker(Manager& manager, Chunk* chunk)
        : ChunkUpdater<TWidth, THeight, TCellSize>(manager, chunk) {}

protected:
    void update(const Cell& cell, Point position) override
    {
        auto [x, y] = position;

        if (cell.type == CellType::Sand)
        {
            int dir = rand() % 2 ? -1 : 1;

            if (this->is_empty({ x, y + 1 }))
            {
                this->set_cell({ x, y }, { CellType::Empty, BLANK });
                this->set_cell({ x, y + 1 }, cell);
            }
            else if (this->is_empty({ x + dir, y + 1 }))
            {
                this->set_cell({ x, y }, { CellType::Empty, BLANK });
                this->set_cell({ x + dir, y + 1 }, cell);
            }
        }
    }
};
