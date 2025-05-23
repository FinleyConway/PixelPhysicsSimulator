#include "simulator/chunk_updater.h"

#include "simulator/context.h"
#include "utils/instrumentor.h"

ChunkUpdater::ChunkUpdater(ChunkManager& manager, CellChunk* chunk)
    : m_manager(manager), m_chunk(chunk)
{
}

void ChunkUpdater::update_chunk()
{
    PROFILE_FUNCTION();

    const Point position = m_chunk->get_position();
    const IntRect& rect = m_chunk->get_current_rect();

    for (int x = rect.min_x; x <= rect.max_x; x++)
    {
        for (int y = rect.min_y; y <= rect.max_y; y++)
        {
            const Cell& cell = m_chunk->get_cell({ x, y });
            const Point world_position = {
                x + (position.x / Context::cell_size),
                y + (position.y / Context::cell_size)
            };

            update(cell, world_position);
        }
    }
}

const Cell& ChunkUpdater::get_cell(Point position)
{
    if (m_chunk->is_in_bounds(position))
    {
        return m_chunk->get_cell(position);
    }

    return m_manager.get_cell(position);
}

void ChunkUpdater::set_cell(Point position, const Cell& cell)
{
    if (m_chunk->is_in_bounds(position))
    {
        m_chunk->set_cell(position, cell);
    }

    m_manager.set_cell(position, cell);
}

bool ChunkUpdater::is_empty(Point position) const
{
    if (m_chunk->is_in_bounds(position))
    {
        return m_chunk->is_empty(position);
    }

    return m_manager.is_empty(position);
}
