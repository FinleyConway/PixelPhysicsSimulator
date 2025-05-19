#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

#include "raylib.h"

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

struct IntRect
{
    int min_x = 0; 
    int min_y = 0;
    int max_x = 0;
    int max_y = 0;
};

template<int TWidth, int THeight, int TCellSize>
class CellChunk
{
public:
    CellChunk(int position_x, int position_y) : 
        m_position_x(position_x),
        m_position_y(position_y)
    {
        m_render_texture = LoadRenderTexture(
			TWidth * TCellSize, 
			THeight * TCellSize
		);
    }

    std::pair<int, int> get_position() const
    {
        return {
            m_position_x,
            m_position_y
        };
    }

    const Cell& get_cell(int x, int y) const
    {
        return m_current_grid[x + y * TWidth];
    }

    void set_cell(int x, int y, const Cell& cell)
    {
        m_next_grid[x + y * TWidth] = cell;
    }

    void keep_alive(int x, int y)
    {
        m_should_update = true;
        set_next_rect(x + y * TWidth);
    }

    const IntRect& get_current_rect() const
    {
        return m_current_rect;
    }

    void update()
    {
        if (m_should_update)
        {
            int empty_cell_amount = TWidth * THeight;
            int empty_cells = 0;

            m_current_grid = m_next_grid;

            // search and reset next grid
            for (auto& cell : m_next_grid)
            {
                if (cell.type == CellType::Empty)
                {
                    empty_cells++;
                }

                cell = Cell();
            }

            // is chunk empty?
            if (empty_cells == empty_cell_amount)
            {
                // flag chunk to be removed
                m_should_remove = true;
            }

            update_rect();
            m_should_update = false;
        }
    }

    void pre_draw()
    {
        if (m_current_rect.min_x > m_current_rect.max_x || m_current_rect.min_y > m_current_rect.max_y) return;

        BeginTextureMode(m_render_texture);
        ClearBackground(BLANK);

        // only draw the changed sections of the chunk
        int scissor_x = m_current_rect.min_x * TCellSize;
        int scissor_y = m_current_rect.min_y * TCellSize;
        int scissor_width = (m_current_rect.max_x - m_current_rect.min_x + 1) * TCellSize;
        int scissor_height = (m_current_rect.max_y - m_current_rect.min_y + 1) * TCellSize;

        BeginScissorMode(scissor_x, scissor_y, scissor_width, scissor_height);

        for (int x = m_current_rect.min_x; x <= m_current_rect.max_x; x++)
        {
            for (int y = m_current_rect.min_y; y <= m_current_rect.max_y; y++)
            {
                const Cell& current_cell = get_cell(x, y);

                DrawRectangle(x * TCellSize, y * TCellSize, TCellSize, TCellSize, current_cell.colour);
            }
        }

        EndScissorMode();
        EndTextureMode();
    }

    void draw(bool debug = false)
    {
        // draw the chunk texture
        Rectangle sourceRec = {
            0.0f,                        
            0.0f,                        
            static_cast<float>(m_render_texture.texture.width),
            static_cast<float>(-m_render_texture.texture.height) // flip texture
        };

        Vector2 position = {
            static_cast<float>(m_position_x),
            static_cast<float>(m_position_y)
        };

        DrawTextureRec(m_render_texture.texture, sourceRec, position, WHITE);

        if (debug)
        {
            DrawRectangleLines(m_position_x, m_position_y, TWidth * TCellSize, THeight * TCellSize, GREEN);
            DrawRectangleLines(
                m_position_x + m_current_rect.min_x * TCellSize,
                m_position_y + m_current_rect.min_y * TCellSize,
                (m_current_rect.max_x - m_current_rect.min_x + 1) * TCellSize,
                (m_current_rect.max_y - m_current_rect.min_y + 1) * TCellSize,
                RED
            );
        }
    }

    bool should_remove() const
    {
        return m_should_remove;
    }

private:
    void set_next_rect(size_t index)
    {
        int x = index % TWidth;
        int y = index / TWidth;

        int min_x = std::max(x - 2, 0);
        int min_y = std::max(y - 2, 0);
        int max_x = std::min(x + 2, TWidth - 1);
        int max_y = std::min(y + 2, THeight - 1);

        m_next_rect.min_x = std::min(m_next_rect.min_x, min_x);
        m_next_rect.min_y = std::min(m_next_rect.min_y, min_y);
        m_next_rect.max_x = std::max(m_next_rect.max_x, max_x);
        m_next_rect.max_y = std::max(m_next_rect.max_y, max_y);
    }

    void update_rect()
    {
        m_current_rect.min_x = m_next_rect.min_x;
        m_current_rect.min_y = m_next_rect.min_y;
        m_current_rect.max_x = m_next_rect.max_x;
        m_current_rect.max_y = m_next_rect.max_y;

        m_next_rect.min_x = TWidth;
        m_next_rect.min_y = THeight;
        m_next_rect.max_x = -1;
        m_next_rect.max_y = -1;
    }

private:
    int m_position_x = 0;
    int m_position_y = 0;
    bool m_should_remove = false;
    bool m_should_update = false;

    IntRect m_next_rect;
    IntRect m_current_rect;
    std::array<Cell, TWidth * THeight> m_next_grid;
    std::array<Cell, TWidth * THeight> m_current_grid;
    RenderTexture2D m_render_texture;
};