#include "simulation/pixel_world.hpp"
#include "simulation/chunk.hpp"

#include <cmath>

PixelWorld::PixelWorld(const ChunkManagerSpec& spec) :
    m_chunk_size(spec.chunk_size),
    m_chunk_cell_size(spec.chunk_cell_size),
    m_min_chunk_pos(spec.min_chunk_pos),
    m_max_chunk_pos(spec.max_chunk_pos)
{
    m_chunk_lookup.reserve(get_max_chunks());
}

PixelWorld::~PixelWorld()   
{
    for (auto* chunk : m_chunks)
    {
        delete chunk;
    }
}

const Cell* PixelWorld::get_cell(int x, int y)
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

void PixelWorld::set_cell(int x, int y, const Cell& cell)
{
    const Point chunk_position = grid_to_chunk(x, y);
    const Point local_position = grid_to_chunk_local(x, y);

    if (Chunk* chunk = get_chunk_or_create(chunk_position))
    {  
        chunk->set_cell(local_position, cell);
    }
}

void PixelWorld::move_cell(int from_x, int from_y, int to_x, int to_y, bool swap)
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
        if (from_local.x == m_chunk_size.x - 1)  notify.x = +1;
        if (from_local.y == 0)            notify.y = -1;
        if (from_local.y == m_chunk_size.y - 1) notify.y = +1;

        // notify neighour chunks
        if (notify.x != 0)                  wake_up_chunk(from_x + notify.x, from_y);
        if (notify.y != 0)                  wake_up_chunk(from_x, from_y + notify.y);
        if (notify.x != 0 && notify.y != 0) wake_up_chunk(from_x + notify.x, from_y + notify.y);

        // move cell
        to_chunk->move_cell(from_local, to_local, swap, from_chunk);
    }
}

bool PixelWorld::is_empty(int x, int y) const
{
    const Point chunk_position = grid_to_chunk(x, y);
    const Point local_position = grid_to_chunk_local(x, y);

    if (m_chunk_lookup.contains(chunk_position))
    {
        return m_chunk_lookup.at(chunk_position)->is_empty(local_position);
    }

    return true;
}

Point PixelWorld::get_chunk_size() const
{
    return m_chunk_size;
}

int PixelWorld::get_chunk_cell_size() const
{
    return m_chunk_cell_size;
}

size_t PixelWorld::get_total_chunks() const
{
    return m_chunks.size();
}

void PixelWorld::pre_draw(const Rectangle& view)
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

void PixelWorld::draw(const Rectangle& view, bool debug)
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

Point PixelWorld::pos_to_grid(float x, float y) const
{
    return { 
        static_cast<int>(std::floor(x / m_chunk_cell_size)), 
        static_cast<int>(std::floor(y / m_chunk_cell_size)) 
    };
}    

Point PixelWorld::grid_to_chunk(int x, int y) const
{
    return { 
        x >= 0 ? x / m_chunk_size.x : (x - m_chunk_size.x + 1) / m_chunk_size.x,
        y >= 0 ? y / m_chunk_size.y : (y - m_chunk_size.y + 1) / m_chunk_size.y,
    };
}

Point PixelWorld::grid_to_chunk_local(int x, int y) const
{
    return {
        ((x % m_chunk_size.x + m_chunk_size.x) % m_chunk_size.x),
        ((y % m_chunk_size.y + m_chunk_size.y) % m_chunk_size.y)
    };
}

Point PixelWorld::world_to_chunk(float x, float y) const
{
    return {
        static_cast<int>(std::floor(x / (m_chunk_size.x * m_chunk_cell_size))),
        static_cast<int>(std::floor(y / (m_chunk_size.y * m_chunk_cell_size)))
    };
}

int PixelWorld::get_max_chunks() const
{
    return (m_max_chunk_pos.x - m_min_chunk_pos.x + 1) * (m_max_chunk_pos.y - m_min_chunk_pos.y + 1);
}

bool PixelWorld::in_world_bounds(const Point& chunk_position)
{
    return (
        chunk_position.x >= m_min_chunk_pos.x && 
        chunk_position.x <= m_max_chunk_pos.x &&
        chunk_position.y >= m_min_chunk_pos.y && 
        chunk_position.y <= m_max_chunk_pos.y
    );
}

bool PixelWorld::is_chunk_in_view(const Chunk* chunk, const Rectangle& view) const
{
    const Point position = chunk->get_position();
    const Point size = { m_chunk_size.x * m_chunk_cell_size, m_chunk_size.y * m_chunk_cell_size };
    const Rectangle chunkRect = {
        static_cast<float>(position.x),
        static_cast<float>(position.y),
        static_cast<float>(size.x),
        static_cast<float>(size.y)
    };

    return CheckCollisionRecs(view, chunkRect);
}

Chunk* PixelWorld::create_chunk(Point chunk_position)
{
    // only create a chunk in the world bounds
    // prevent static_vector from overflowing
    if (in_world_bounds(chunk_position))
    {
        // create chunk at world position
        const Point position = {
            chunk_position.x * m_chunk_size.x * m_chunk_cell_size,
            chunk_position.y * m_chunk_size.y * m_chunk_cell_size,
        };

        auto* chunk = new Chunk(position, m_chunk_size, m_chunk_cell_size);

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

Chunk* PixelWorld::get_chunk_or_create(Point chunk_position)
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

void PixelWorld::remove_empty_chunks()
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

void PixelWorld::wake_up_chunk(int x, int y)
{
    const Point chunk_position = grid_to_chunk(x, y);
    const Point local_position = grid_to_chunk_local(x, y);

    // only wake up chunk if it exists
    if (m_chunk_lookup.contains(chunk_position))
    {
        m_chunk_lookup.at(chunk_position)->wake_up(local_position);
    }
}