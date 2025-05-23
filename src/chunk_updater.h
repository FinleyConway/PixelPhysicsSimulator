#pragma once

#include "cell.h"
#include "cell_chunk.h"
#include "int_rect.h"
#include "point.h"

template<int TWidth, int THeight, int TCellSize>
class ChunkUpdater
{
using Manager = ChunkUpdater<TWidth, THeight, TCellSize>;
using Chunk = CellChunk<TWidth, THeight, TCellSize>;

public:
    ChunkUpdater(Manager& manager, Chunk* chunk) : m_manager(manager), m_chunk(chunk) 
    {
    }

    virtual ~ChunkUpdater() = default;

    void update_chunk()
    {
        const Point position = m_chunk->get_position();
        const IntRect& rect = m_chunk->get_current_rect();

        for (int x = rect.min_x; x <= rect.max_x; x++)
        {
            for (int y = rect.min_y; y <= rect.max_y; y++)
            {
                const Cell& cell = m_chunk->get_cell({ x, y });
                const Point world_position = {
                    x + (position.x / m_chunk->get_cell_size()),
                    y + (position.y / m_chunk->get_cell_size())
                };

                update(cell, world_position);
            }
        }
    }   

protected:
    virtual void update(const Cell& cell, Point position) = 0;

    Cell& get_cell(Point position)
    {
        if (m_chunk->is_in_bounds(position))
        {
            return m_chunk->get_cell(position);
        }

        return m_manager.get_cell(position);
    }

    void set_cell(Point position, const Cell& cell)
    {
        if (m_chunk->is_in_bounds(position))
        {
            m_chunk->set_cell(position, cell);
        }

        m_manager.set_cell(position, cell);
    }

    bool is_empty(Point position) const
    {
        if (m_chunk->is_in_bounds(position))
        {
            return m_chunk->is_empty(position);
        }

        return m_manager.is_empty(position);
    }

private:
    Manager& m_manager;
    Chunk* m_chunk = nullptr;
};