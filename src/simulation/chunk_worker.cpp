#include "simulation/chunk_worker.hpp"

#include "core/chunk_context.hpp"

ChunkWorker::ChunkWorker(ChunkManager& manager, Chunk* chunk) : m_manager(manager), m_chunk(chunk)
{
}

void ChunkWorker::update_chunk(float time_step)
{
    const Point position = m_chunk->get_position();
    const IntRect& rect = m_chunk->get_current_rect();

    for (int x = rect.min_x; x <= rect.max_x; x++)
    {
        for (int y = rect.max_y; y >= rect.min_y; y--)
        {
            Cell& cell = m_chunk->get_cell({ x, y });
            const Point world_position = {
                x + (position.x / ChunkContext::cell_size),
                y + (position.y / ChunkContext::cell_size)
            };

            update_cell(cell, world_position.x, world_position.y);
            handle_life_time(cell, x, y, time_step);
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
    m_manager.move_cell(from_x, from_y, to_x, to_y, false);
}

void ChunkWorker::push_cell(int from_x, int from_y, int dir_x, int dir_y)
{
    int final_dx = 0;
    int final_dy = 0;

    int step_x = (dir_x == 0) ? 0 : (dir_x > 0 ? 1 : -1);
    int step_y = (dir_y == 0) ? 0 : (dir_y > 0 ? 1 : -1);

    int max_steps = std::max(std::abs(dir_x), std::abs(dir_y));

    for (int i = 1; i <= max_steps; i++)
    {
        int target_x = from_x + step_x * i;
        int target_y = from_y + step_y * i;

        if (m_manager.is_empty(target_x, target_y))
        {
            final_dx = step_x * i;
            final_dy = step_y * i;
        }
        else break;
    }

    if (final_dx != 0 || final_dy != 0)
    {
        move_cell(from_x, from_y, from_x + final_dx, from_y + final_dy);
    }
}

void ChunkWorker::swap_cells(int from_x, int from_y, int to_x, int to_y)
{
    m_manager.move_cell(from_x, from_y, to_x, to_y, true);
}

bool ChunkWorker::is_empty(int x, int y) const
{
    return m_manager.is_empty(x, y);
}

void ChunkWorker::handle_life_time(Cell& cell, int x, int y, float time_step)
{
    if (cell.life_time >= 0)
    {
        cell.life_time -= time_step;

        if (cell.life_time <= 0)
        {
            m_chunk->set_cell({ x, y }, Cell());
        }
        else
        {
            m_chunk->wake_up({ x, y });
        }
    }
}