#include "cell_chunk.h"

#include <cassert>

#include "instrumentor.h"

template<int TWidth, int THeight, int TCellSize>
CellChunk<TWidth, THeight, TCellSize>::CellChunk(Point position) : m_position(position)
{
    reset_rect(m_final_rect);
    reset_rect(m_intermediate_rect);
    reset_rect(m_dirty_rect);

    m_render_texture = LoadRenderTexture(TWidth * TCellSize, THeight * TCellSize);
}

template<int TWidth, int THeight, int TCellSize>
CellChunk<TWidth, THeight, TCellSize>::~CellChunk()
{
    UnloadRenderTexture(m_render_texture);
}

template<int TWidth, int THeight, int TCellSize>
Point CellChunk<TWidth, THeight, TCellSize>::get_position() const
{
    return m_position;
}

template<int TWidth, int THeight, int TCellSize>
const IntRect& CellChunk<TWidth, THeight, TCellSize>::get_current_rect() const
{
    return m_dirty_rect;
}

template<int TWidth, int THeight, int TCellSize>
const Cell& CellChunk<TWidth, THeight, TCellSize>::get_cell(Point position) const
{
    assert(is_in_bounds(position) && "CellChunk::get_cell out of bounds");

    return m_grid[position.x + position.y * TWidth];
}

template<int TWidth, int THeight, int TCellSize>
void CellChunk<TWidth, THeight, TCellSize>::set_cell(Point position, const Cell& cell)
{
    assert(is_in_bounds(position) && "CellChunk::set_cell out of bounds");

    int index = position.x + position.y * TWidth;

    m_changes.emplace_back(index, cell);
    m_drawn = false;

    set_next_rect(index);
}

template<int TWidth, int THeight, int TCellSize>
bool CellChunk<TWidth, THeight, TCellSize>::is_empty(Point position) const
{
    assert(is_in_bounds(position) && "CellChunk::is_empty out of bounds");

    return m_grid[position.x + position.y * TWidth].type == CellType::Empty;
}

template<int TWidth, int THeight, int TCellSize>
bool CellChunk<TWidth, THeight, TCellSize>::is_in_bounds(Point position) const
{
    return position.x < TWidth && position.y < THeight;
}


template<int TWidth, int THeight, int TCellSize>
void CellChunk<TWidth, THeight, TCellSize>::wake_up(Point position)
{
    set_next_rect(position.x + position.y * TWidth);
}

template<int TWidth, int THeight, int TCellSize>
void CellChunk<TWidth, THeight, TCellSize>::apply_cells()
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

template<int TWidth, int THeight, int TCellSize>
void CellChunk<TWidth, THeight, TCellSize>::update_rect()
{
    m_dirty_rect = m_intermediate_rect;

    reset_rect(m_intermediate_rect);
}

template<int TWidth, int THeight, int TCellSize>
void CellChunk<TWidth, THeight, TCellSize>::pre_draw()
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
            const Cell& current_cell = get_cell({ x, y });

            if (current_cell.type != CellType::Empty)
            {
                DrawRectangle(x * TCellSize, y * TCellSize, TCellSize, TCellSize, current_cell.colour);
            }
        }
    }

    EndTextureMode();

    m_drawn = true;
}

template<int TWidth, int THeight, int TCellSize>
void CellChunk<TWidth, THeight, TCellSize>::draw(bool debug) const
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
        DrawRectangleLines(m_position.x, m_position.y, TWidth * TCellSize, THeight * TCellSize, GREEN);
        DrawRectangleLines(
            m_position.x + m_dirty_rect.min_x * TCellSize,
            m_position.y + m_dirty_rect.min_y * TCellSize,
            (m_dirty_rect.max_x - m_dirty_rect.min_x + 1) * TCellSize,
            (m_dirty_rect.max_y - m_dirty_rect.min_y + 1) * TCellSize,
            (m_dirty_rect.min_x > m_dirty_rect.max_x || m_dirty_rect.min_y > m_dirty_rect.max_y) ? BLUE : RED
        );
        DrawRectangleLines(
            m_position.x + m_final_rect.min_x * TCellSize,
            m_position.y + m_final_rect.min_y * TCellSize,
            (m_final_rect.max_x - m_final_rect.min_x + 1) * TCellSize,
            (m_final_rect.max_y - m_final_rect.min_y + 1) * TCellSize,
            WHITE
        );
        DrawText(TextFormat("%d", m_filled_cells), m_position.x, m_position.y, 20, YELLOW);
    }
}

template<int TWidth, int THeight, int TCellSize>
bool CellChunk<TWidth, THeight, TCellSize>::should_remove() const
{
    return m_filled_cells == 0;
}

template<int TWidth, int THeight, int TCellSize>
void CellChunk<TWidth, THeight, TCellSize>::set_next_rect(int index)
{
    int x = index % TWidth;
    int y = index / TWidth;

    int min_x = std::max(x - 2, 0);
    int min_y = std::max(y - 2, 0);
    int max_x = std::min(x + 2, TWidth - 1);
    int max_y = std::min(y + 2, THeight - 1);

    m_intermediate_rect.min_x = std::min(m_intermediate_rect.min_x, min_x);
    m_intermediate_rect.min_y = std::min(m_intermediate_rect.min_y, min_y);
    m_intermediate_rect.max_x = std::max(m_intermediate_rect.max_x, max_x);
    m_intermediate_rect.max_y = std::max(m_intermediate_rect.max_y, max_y);
}

template<int TWidth, int THeight, int TCellSize>
void CellChunk<TWidth, THeight, TCellSize>::generate_bounds()
{
    reset_rect(m_final_rect);

    for (int y = 0; y < THeight; y++)
    {
        for (int x = 0; x < TWidth; x++)
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

template<int TWidth, int THeight, int TCellSize>
void CellChunk<TWidth, THeight, TCellSize>::reset_rect(IntRect& rect)
{
    rect.min_x = TWidth;
    rect.min_y = THeight;
    rect.max_x = -1;
    rect.max_y = -1;
}
