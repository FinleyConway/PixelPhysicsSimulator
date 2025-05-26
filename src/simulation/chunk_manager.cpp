#include "simulation/chunk_manager.hpp"

#include <cmath>

ChunkManager::ChunkManager()
{
    m_chunk_lookup.reserve(c_max_chunks);
}

ChunkManager::~ChunkManager()
{
    for (auto* chunk : m_chunks)
    {
        delete chunk;
    }
}

const Cell* ChunkManager::get_cell(int x, int y)
{
    const Point chunk_position = grid_to_chunk(x, y);
    const Point local_position = grid_to_chunk_local(x, y);

    if (Chunk* chunk = get_chunk_or_create(chunk_position))
    {
        return &chunk->get_cell(local_position);
    } 

    // not in bounds of world
    return nullptr;
}    

void ChunkManager::set_cell(int x, int y, const Cell& cell)
{
    const Point chunk_position = grid_to_chunk(x, y);
    const Point local_position = grid_to_chunk_local(x, y);

    if (Chunk* chunk = get_chunk_or_create(chunk_position))
    {  
        chunk->set_cell(local_position, cell);
    }
}

void ChunkManager::move_cell(int from_x, int from_y, int to_x, int to_y)
{
    const Point from_chunk_pos = grid_to_chunk(from_x, from_y);
    const Point to_chunk_pos = grid_to_chunk(to_x, to_y);

    Chunk* from_chunk = get_chunk_or_create(from_chunk_pos);
    Chunk* to_chunk = get_chunk_or_create(to_chunk_pos);

    if (from_chunk != nullptr && to_chunk != nullptr)
    {
        const Point from_local = grid_to_chunk_local(from_x, from_y);
        const Point to_local = grid_to_chunk_local(to_x, to_y);
        Point notify;

        // get chunk offset if local pos is at the edges
        if (from_local.x == 0)            notify.x = -1;
        if (from_local.x == c_width - 1)  notify.x = +1;
        if (from_local.y == 0)            notify.y = -1;
        if (from_local.y == c_height - 1) notify.y = +1;

        // notify neighour chunks
        if (notify.x != 0)                  wake_up_chunk(from_x + notify.x, from_y);
        if (notify.y != 0)                  wake_up_chunk(from_x, from_y + notify.y);
        if (notify.x != 0 && notify.y != 0) wake_up_chunk(from_x + notify.x, from_y + notify.y);

        // move cell
        to_chunk->move_cell(from_local, to_local, from_chunk);
    }
}

bool ChunkManager::is_empty(int x, int y) const
{
    const Point chunk_position = grid_to_chunk(x, y);
    const Point local_position = grid_to_chunk_local(x, y);

    if (m_chunk_lookup.contains(chunk_position))
    {
        return m_chunk_lookup.at(chunk_position)->is_empty(local_position);
    }

    return true;
}

size_t ChunkManager::get_total_chunks() const
{
    return m_chunks.size();
}

void ChunkManager::pre_draw(const Rectangle& view)
{
    // prepare all active chunks in view
    for (auto* chunk : m_chunks)
    {
        assert(chunk != nullptr);

        if (is_chunk_in_view(chunk, view))
        {
            chunk->pre_draw();
        }
    }
}

void ChunkManager::draw(const Rectangle& view, bool debug)
{
    // draw all active chunks in view
    for (const auto* chunk : m_chunks)
    {
        assert(chunk != nullptr);

        if (is_chunk_in_view(chunk, view))
        {
            chunk->draw(debug);
        }
    }
}

Point ChunkManager::pos_to_grid(float x, float y) const
{
    return { 
        static_cast<int>(std::floor(x / c_cell_size)), 
        static_cast<int>(std::floor(y / c_cell_size)) 
    };
}    

Point ChunkManager::grid_to_chunk(int x, int y) const
{
    return { 
        x >= 0 ? x / c_width : (x - c_width + 1) / c_width,
        y >= 0 ? y / c_height : (y - c_height + 1) / c_height,
    };
}

Point ChunkManager::grid_to_chunk_local(int x, int y) const
{
    return {
        ((x % c_width + c_width) % c_width),
        ((y % c_height + c_height) % c_height)
    };
}

Point ChunkManager::world_to_chunk(float x, float y) const
{
    return {
        static_cast<int>(std::floor(x / (c_width * c_cell_size))),
        static_cast<int>(std::floor(y / (c_height * c_cell_size)))
    };
}

bool ChunkManager::in_world_bounds(const Point& chunk_position)
{
    return (
        chunk_position.x >= c_min_chunk_pos.x && 
        chunk_position.x <= c_max_chunk_pos.x &&
        chunk_position.y >= c_min_chunk_pos.y && 
        chunk_position.y <= c_max_chunk_pos.y
    );
}

bool ChunkManager::is_chunk_in_view(const Chunk* chunk, const Rectangle& view) const
{
    const Point position = chunk->get_position();
    const Point size = { c_width * c_cell_size, c_height * c_cell_size };
    const Rectangle chunkRect = {
        static_cast<float>(position.x),
        static_cast<float>(position.y),
        static_cast<float>(size.x),
        static_cast<float>(size.y)
    };

    return CheckCollisionRecs(view, chunkRect);
}

Chunk* ChunkManager::create_chunk(Point chunk_position)
{
    // only create a chunk in the world bounds
    // prevent static_vector from overflowing
    if (in_world_bounds(chunk_position))
    {
        // create chunk at world position
        const Point position = {
            chunk_position.x * c_width * c_cell_size,
            chunk_position.y * c_height * c_cell_size,
        };

        auto* chunk = new Chunk(position);

        // attempt to create chunk and return a reference
        auto [it, inserted] = m_chunk_lookup.try_emplace(chunk_position, chunk);

        if (inserted)
        {
            return m_chunks.emplace_back(chunk);
        }

        delete chunk; // i cant imagine we'll get here but who knows
    }

    // chunk cant be created
    return nullptr;
}

Chunk* ChunkManager::get_chunk_or_create(Point chunk_position)
{
    // return an existing chunk
    if (m_chunk_lookup.contains(chunk_position))
    {
        return m_chunk_lookup.at(chunk_position);
    }

    // or create a new one, this should be fine as the array wont move
    // and wont invalid any pointers
    return create_chunk(chunk_position); 
}

void ChunkManager::remove_empty_chunks()
{
    // go through each chunk and check if its empty
    for (auto it = m_chunks.begin(); it != m_chunks.end();)
    {
        Chunk* chunk = *it;

        if (chunk->should_remove())
        {
            // remove chunk from the world
            const Point position = chunk->get_position();
            const Point chunk_position = world_to_chunk(position.x, position.y);

            m_chunk_lookup.erase(chunk_position);
            it = m_chunks.erase(it);

            delete chunk;
        }
        else 
        {
            it++;
        }
    }
}

void ChunkManager::wake_up_chunk(int x, int y)
{
    const Point chunk_position = grid_to_chunk(x, y);
    const Point local_position = grid_to_chunk_local(x, y);

    // only wake up chunk if it exists
    if (m_chunk_lookup.contains(chunk_position))
    {
        m_chunk_lookup.at(chunk_position)->wake_up(local_position);
    }
}