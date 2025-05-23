#include "simulator/cell_chunk.h"

#include <cassert>

#include "utils/instrumentor.h"

CellChunk::CellChunk(Point position) : m_position(position)
{
    const int width = Context::width;
    const int height = Context::height;
    const int cell_size = Context::cell_size;

    reset_rect(m_final_rect);
    reset_rect(m_intermediate_rect);
    reset_rect(m_dirty_rect);

    m_render_texture = LoadRenderTexture(width * cell_size, height * cell_size);
}

CellChunk::~CellChunk()
{
    UnloadRenderTexture(m_render_texture);
}

Point CellChunk::get_position() const
{
    return m_position;
}

const IntRect& CellChunk::get_current_rect() const
{
    return m_dirty_rect;
}

const Cell& CellChunk::get_cell(Point position) const
{
    assert(is_in_bounds(position) && "CellChunk::get_cell out of bounds");

    return m_grid[get_index(position)];
}

void CellChunk::set_cell(Point position, const Cell& cell)
{
    assert(is_in_bounds(position) && "CellChunk::set_cell out of bounds");

    int index = get_index(position);

    m_changes.emplace_back(index, cell);
    m_drawn = false;

    set_next_rect(index);
}

bool CellChunk::is_empty(Point position) const
{
    assert(is_in_bounds(position) && "CellChunk::is_empty out of bounds");

    return m_grid[get_index(position)].type == CellType::Empty;
}

bool CellChunk::is_in_bounds(Point position) const
{
    return position.x < Context::width && position.y < Context::height;
}


void CellChunk::wake_up(Point position)
{
    set_next_rect(get_index(position));
}

void CellChunk::apply_cells()
{
    PROFILE_FUNCTION();

    if (m_changes.empty()) return;

    for (auto& [index, cell] : m_changes)
    {
        const Cell& dest = m_grid[index];

        if (dest.type == CellType::Empty && cell.type != CellType::Empty)
        {
            m_filled_cells++;
        }
        else if (dest.type != CellType::Empty && cell.type == CellType::Empty)
        {
            m_filled_cells--;
        }

        m_grid[index] = cell;
    }

    m_changes.clear();
}

void CellChunk::update_rect()
{
    m_dirty_rect = m_intermediate_rect;

    reset_rect(m_intermediate_rect);
}

void CellChunk::pre_draw()
{
    PROFILE_FUNCTION();

    if (m_drawn) return;

    generate_bounds();

    BeginTextureMode(m_render_texture);
    ClearBackground(BLANK);

    for (int x = m_final_rect.min_x; x <= m_final_rect.max_x; x++)
    {
        for (int y = m_final_rect.min_y; y <= m_final_rect.max_y; y++)
        {
            const int cell_size = Context::cell_size;
            const Cell& current_cell = get_cell({ x, y });

            if (current_cell.type != CellType::Empty)
            {
                DrawRectangle(x * cell_size, y * cell_size, cell_size, cell_size, current_cell.colour);
            }
        }
    }

    EndTextureMode();

    m_drawn = true;
}

void CellChunk::draw(bool debug) const
{
    PROFILE_FUNCTION();

    Rectangle sourceRec = {
        0.0f,
        0.0f,
        static_cast<float>(m_render_texture.texture.width),
        static_cast<float>(-m_render_texture.texture.height)
    };

    Vector2 position = {
        static_cast<float>(m_position.x),
        static_cast<float>(m_position.y)
    };

    DrawTextureRec(m_render_texture.texture, sourceRec, position, WHITE);

    if (debug)
    {
        const int width = Context::width;
        const int height = Context::height;
        const int cell_size = Context::cell_size;

        DrawRectangleLines(m_position.x, m_position.y, width * cell_size, height * cell_size, GREEN);
        DrawRectangleLines(
            m_position.x + m_dirty_rect.min_x * cell_size,
            m_position.y + m_dirty_rect.min_y * cell_size,
            (m_dirty_rect.max_x - m_dirty_rect.min_x + 1) * cell_size,
            (m_dirty_rect.max_y - m_dirty_rect.min_y + 1) * cell_size,
            (m_dirty_rect.min_x > m_dirty_rect.max_x || m_dirty_rect.min_y > m_dirty_rect.max_y) ? BLUE : RED
        );
        DrawRectangleLines(
            m_position.x + m_final_rect.min_x * cell_size,
            m_position.y + m_final_rect.min_y * cell_size,
            (m_final_rect.max_x - m_final_rect.min_x + 1) * cell_size,
            (m_final_rect.max_y - m_final_rect.min_y + 1) * cell_size,
            WHITE
        );
        DrawText(TextFormat("%d", m_filled_cells), m_position.x, m_position.y, 20, YELLOW);
    }
}

bool CellChunk::should_remove() const
{
    return m_filled_cells == 0;
}

int CellChunk::get_index(Point position) const
{
    return position.x + position.y * Context::width;
}

void CellChunk::set_next_rect(int index)
{
    int width = Context::width;
    int hieght = Context::height;
    int x = index % width;
    int y = index / width;

    int min_x = std::max(x - 2, 0);
    int min_y = std::max(y - 2, 0);
    int max_x = std::min(x + 2, width - 1);
    int max_y = std::min(y + 2, hieght - 1);

    m_intermediate_rect.min_x = std::min(m_intermediate_rect.min_x, min_x);
    m_intermediate_rect.min_y = std::min(m_intermediate_rect.min_y, min_y);
    m_intermediate_rect.max_x = std::max(m_intermediate_rect.max_x, max_x);
    m_intermediate_rect.max_y = std::max(m_intermediate_rect.max_y, max_y);
}

void CellChunk::generate_bounds()
{
    reset_rect(m_final_rect);

    for (int x = 0; x < Context::width; x++)
    {
        for (int y = 0; y < Context::height; y++)
        {
            if (get_cell({ x, y }).type != CellType::Empty)
            {
                m_final_rect.min_x = std::min(m_final_rect.min_x, x);
                m_final_rect.min_y = std::min(m_final_rect.min_y, y);
                m_final_rect.max_x = std::max(m_final_rect.max_x, x);
                m_final_rect.max_y = std::max(m_final_rect.max_y, y);
            }
        }
    }
}

void CellChunk::reset_rect(IntRect& rect)
{
    rect.min_x = Context::width;
    rect.min_y = Context::height;
    rect.max_x = -1;
    rect.max_y = -1;
}
