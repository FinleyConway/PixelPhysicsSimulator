#include "simulation/chunk_worker.hpp"

#include "core/chunk_context.hpp"

ChunkWorker::ChunkWorker(ChunkManager& manager, Chunk* chunk) : m_manager(manager), m_chunk(chunk)
{
}

void ChunkWorker::update_chunk()
{
    const Point position = m_chunk->get_position();
    const IntRect& rect = m_chunk->get_current_rect();

    for (int x = rect.min_x; x <= rect.max_x; x++)
    {
        for (int y = rect.max_y; y >= rect.min_y; y--)
        {
            const Cell& cell = m_chunk->get_cell({ x, y });
            const Point world_position = {
                x + (position.x / ChunkContext::cell_size),
                y + (position.y / ChunkContext::cell_size)
            };

            update_cell(cell, world_position.x, world_position.y);
        }
    }
}

const Cell* ChunkWorker::get_cell(int x, int y)
{
    return m_manager.get_cell(x, y);
}

void ChunkWorker::set_cell(int x, int y, const Cell& cell)
{
    m_manager.set_cell(x, y, cell);
}

void ChunkWorker::move_cell(int from_x, int from_y, int to_x, int to_y)
{
    m_manager.move_cell(from_x, from_y, to_x, to_y);
}

bool ChunkWorker::is_empty(int x, int y) const
{
    return m_manager.is_empty(x, y);
}