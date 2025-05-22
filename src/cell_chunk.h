#pragma once

#include <algorithm>
#include <array>

#include "raylib.h"

#include "instrumentor.h"
#include "point.h"
#include "int_rect.h"

enum class CellType
{
    Empty = 0,
    Sand,
    Stone,
};

struct Cell
{
    CellType type = CellType::Empty;
    Color colour = BLANK;
};

template<int TWidth, int THeight, int TCellSize>
class CellChunk
{
public:
    CellChunk(Point position) : m_position(position)
    {
        m_render_texture = LoadRenderTexture(TWidth * TCellSize, THeight * TCellSize);
        m_changes.reserve(TWidth * THeight);

        reset_rect(m_final_rect);
        reset_rect(m_intermediate_rect);
    }

    ~CellChunk()
    {
        UnloadRenderTexture(m_render_texture);
    }

    Point get_position() const
    {
        return m_position;
    }

    const Cell& get_cell(Point position) const
    {
        return m_grid[position.x + position.y * TWidth];
    }

    void set_cell(Point position, const Cell& cell)
    {
        int index = position.x + position.y * TWidth;

        m_changes.emplace_back(index, cell);
        m_drawn = false;

        set_next_rect(index);
    }

    void wake_up(Point position)
    {
        set_next_rect(position.x + position.y * TWidth);
    }

    bool is_empty(Point position) const
    {
        return m_grid[position.x + position.y * TWidth].type == CellType::Empty;
    }

    const IntRect& get_current_rect() const
    {
        return m_dirty_rect;
    }

    void apply_cells()
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

    void update_rect()
    {
        m_dirty_rect = m_intermediate_rect;

        reset_rect(m_intermediate_rect);
    }

    void pre_draw()
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

    void draw(bool debug = false)
    {
        PROFILE_FUNCTION();

        // draw the chunk texture
        Rectangle sourceRec = {
            0.0f,                        
            0.0f,                        
            static_cast<float>(m_render_texture.texture.width),
            static_cast<float>(-m_render_texture.texture.height) // flip texture
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

    bool should_remove() const
    {
        return m_filled_cells == 0;
    }

private:
    void set_next_rect(int index)
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

    void generate_bounds() 
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

    void reset_rect(IntRect& rect)
    {
        rect.min_x = TWidth;
        rect.min_y = THeight;
        rect.max_x = -1;
        rect.max_y = -1;
    }

private:
    Point m_position;
    int m_filled_cells = 0;

    bool m_drawn = false;
    IntRect m_final_rect;

    IntRect m_intermediate_rect;
    IntRect m_dirty_rect;
    std::vector<std::pair<int, Cell>> m_changes;
    std::array<Cell, TWidth * THeight> m_grid;
    RenderTexture2D m_render_texture;
};