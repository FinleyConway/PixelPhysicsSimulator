#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>

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
    CellChunk(Point position) : 
        m_position(position)
    {
        m_render_texture = LoadRenderTexture(
			TWidth * TCellSize, 
			THeight * TCellSize
		);
        m_changes.reserve(TWidth * THeight);
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
    }

    bool is_empty(Point position) const
    {
        return m_grid[position.x + position.y * TWidth].type == CellType::Empty;
    }

    void keep_alive(Point position)
    {
        set_next_rect(position.x + position.y * TWidth);
    }

    const IntRect& get_current_rect() const
    {
        return m_current_rect;
    }

    void update()
    {
        PROFILE_FUNCTION();

        if (m_changes.empty()) return;

        int m_filled_cells = 0;

        update_rect();

        for (auto& [index, cell] : m_changes)
        {
            m_grid[index] = cell;
        }

        for (size_t i = 0; i < m_grid.size(); i++)
        {
            if (m_grid[i].type == CellType::Empty)
            {
                m_filled_cells++;
            }
        }

        if (m_filled_cells == TWidth * THeight)
        {
            m_should_delete = true;
        }

        m_changes.clear();
    }

    void pre_draw()
    {
        PROFILE_FUNCTION();

        if (m_drawn) return;

        BeginTextureMode(m_render_texture);
        ClearBackground(BLANK);

        for (int x = 0; x < TWidth; x++)
        {
            for (int y = 0; y < THeight; y++)
            {
                const Cell& current_cell = get_cell({ x, y });

                DrawRectangle(x * TCellSize, y * TCellSize, TCellSize, TCellSize, current_cell.colour);
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
                m_position.x + m_current_rect.min_x * TCellSize,
                m_position.y + m_current_rect.min_y * TCellSize,
                (m_current_rect.max_x - m_current_rect.min_x + 1) * TCellSize,
                (m_current_rect.max_y - m_current_rect.min_y + 1) * TCellSize,
                RED
            );
        }
    }

    bool should_remove() const
    {
        return m_should_delete;
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
    Point m_position;
    bool m_drawn = false;
    bool m_should_delete = false;

    IntRect m_next_rect;
    IntRect m_current_rect;
    std::vector<std::pair<int, Cell>> m_changes;
    std::array<Cell, TWidth * THeight> m_grid;
    RenderTexture2D m_render_texture;
};