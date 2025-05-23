#include "simulator/chunk_manager.h"

#include <cmath>

ChunkManager::~ChunkManager()
{
    for (auto* chunk : m_chunks)
    {
        delete chunk;
    }
}

void ChunkManager::pre_draw(const Rectangle& view)
{
    PROFILE_FUNCTION();

    for (auto* chunk : m_chunks)
    {
        if (is_chunk_in_view(chunk, view))
        {
            chunk->pre_draw();
        }
    }
}

void ChunkManager::draw(const Rectangle& view, bool debug)
{
    PROFILE_FUNCTION();

    for (auto* chunk : m_chunks)
    {
        if (is_chunk_in_view(chunk, view))
        {
            chunk->draw(debug);
        }
    }
}

const Cell& ChunkManager::get_cell(Point grid_position)
{
    const Point chunk_position = grid_to_chunk(grid_position);
    const Point local_position = grid_to_chunk_local(grid_position);

    return get_chunk(chunk_position)->get_cell(local_position);
}

void ChunkManager::set_cell(Point grid_position, const Cell& cell)
{
    const Point chunk_position = grid_to_chunk(grid_position);
    const Point local_position = grid_to_chunk_local(grid_position);

    auto* chunk = get_chunk(chunk_position);
    Point notify;

    if (local_position.x == 0)                   notify.x = -1;
    if (local_position.x == Context::width - 1)  notify.x = +1;
    if (local_position.y == 0)                   notify.y = -1;
    if (local_position.y == Context::height - 1) notify.y = +1;

    Point notify_offset = grid_position + notify;

    if (notify.x != 0)                  wake_up_chunk(notify_offset);
    if (notify.y != 0)                  wake_up_chunk(notify_offset);
    if (notify.x != 0 && notify.y != 0) wake_up_chunk(notify_offset);

    chunk->set_cell(local_position, cell);
}

bool ChunkManager::is_empty(Point grid_position) const
{
    const Point chunk_position = grid_to_chunk(grid_position);
    const Point local_position = grid_to_chunk_local(grid_position);

    if (m_chunk_lookup.contains(chunk_position))
    {
        return m_chunk_lookup.at(chunk_position)->is_empty(local_position);
    }

    return true;
}

void ChunkManager::wake_up_chunk(Point grid_position)
{
    const Point chunk_position = grid_to_chunk(grid_position);
    const Point local_position = grid_to_chunk_local(grid_position);

    get_chunk(chunk_position)->wake_up(local_position);
}

size_t ChunkManager::get_total_chunks() const
{
    return m_chunks.size();
}

Point ChunkManager::pos_to_grid(float x, float y) const
{
    const int cell_size = Context::cell_size;

    return {
        static_cast<int>(std::floor(x / cell_size)),
        static_cast<int>(std::floor(y / cell_size))
    };
}

Point ChunkManager::grid_to_chunk(Point grid_position) const
{
    const int width = Context::width;
    const int height = Context::height;

    return {
        grid_position.x >= 0 ? grid_position.x / width : (grid_position.x - width + 1) / width,
        grid_position.y >= 0 ? grid_position.y / height : (grid_position.y - height + 1) / height,
    };
}

Point ChunkManager::grid_to_chunk_local(Point grid_position) const
{
    const int width = Context::width;
    const int height = Context::height;

    return {
        ((grid_position.x % width + width) % width),
        ((grid_position.y % height + height) % height)
    };
}

Point ChunkManager::world_to_chunk(Point world_position) const
{
    const int width = Context::width;
    const int height = Context::height;
    const int cell_size = Context::cell_size;

    return {
        static_cast<int>(std::floor(world_position.x / (width * cell_size))),
        static_cast<int>(std::floor(world_position.y / (height * cell_size)))
    };
}

CellChunk* ChunkManager::create_chunk(Point chunk_position)
{
    PROFILE_FUNCTION();

    const int width = Context::width;
    const int height = Context::height;
    const int cell_size = Context::cell_size;

    Point position = {
        chunk_position.x * width * cell_size,
        chunk_position.y * height * cell_size
    };

    auto* chunk = new CellChunk(position);

    m_chunk_lookup.try_emplace(chunk_position, chunk);
    m_chunks.emplace_back(chunk);

    return chunk;
}

CellChunk* ChunkManager::get_chunk(Point chunk_position)
{
    PROFILE_FUNCTION();

    if (m_chunk_lookup.contains(chunk_position))
    {
        return m_chunk_lookup.at(chunk_position);
    }

    return create_chunk(chunk_position);
}

void ChunkManager::remove_empty_chunks()
{
    PROFILE_FUNCTION();

    for (auto it = m_chunks.begin(); it != m_chunks.end();)
    {
        CellChunk* chunk = *it;

        if (chunk->should_remove())
        {
            const Point chunk_world_position = chunk->get_position();
            const Point chunk_position = world_to_chunk(chunk_world_position);

            m_chunk_lookup.erase(chunk_position);
            it = m_chunks.erase(it);

            delete chunk;
        }
        else
        {
            ++it;
        }
    }
}

bool ChunkManager::is_chunk_in_view(const CellChunk* chunk, const Rectangle& view)
{
    const int width = Context::width;
    const int height = Context::height;
    const int cell_size = Context::cell_size;
    const Point position = chunk->get_position();
    const Point size = { width * cell_size, height * cell_size };
    const Rectangle chunkRect = {
        static_cast<float>(position.x),
        static_cast<float>(position.y),
        static_cast<float>(size.x),
        static_cast<float>(size.y)
    };

    return CheckCollisionRecs(view, chunkRect);
}
