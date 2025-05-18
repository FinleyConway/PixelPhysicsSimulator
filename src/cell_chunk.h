#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>

#include "instrumentor.h"
#include "raylib.h"

#include "cell.h"

struct Rect
{
    int32_t min_x = 0;
    int32_t min_y = 0;
    int32_t max_x = 0;
    int32_t max_y = 0;
};

template<size_t Width, size_t Height, size_t CellSize>
class CellChunk 
{
public:
    CellChunk(int32_t position_x, int32_t position_y) :
        m_position_x(position_x),
        m_position_y(position_y)
    {
        m_render_texture = LoadRenderTexture(
			Width * CellSize, 
			Height * CellSize
		);
    }

    ~CellChunk()
    {
        UnloadRenderTexture(m_render_texture);
    }

    bool in_bounds(size_t x, size_t y) const
    {
        return x < Width && y < Height;
    }

    const Cell& get_cell(size_t x, size_t y) const
    {
        return m_current_grid[x + y * Width];
    }

    const Cell& get_temp_cell(size_t x, size_t y) const
    {
        return m_next_grid[x + y * Width];
    }

    bool set_cell(size_t x, size_t y, const Cell& cell)
    {
        if (in_bounds(x, y))
        {
            m_next_grid[x + y * Width] = cell;
            set_next_rect(x + y * Width);

            return true;
        }

        return false;
    }

    bool has_cell(size_t x, size_t y) const
    {
        if (in_bounds(x, y))
        {
            return m_current_grid[x + y * Width].cell_type != CellType::Empty;
        }

        return false;
    }

    bool is_empty_cell(size_t x, size_t y) const
    {
        if (in_bounds(x, y))
        {
            bool current = m_current_grid[x + y * Width].cell_type == CellType::Empty;
            bool next = m_next_grid[x + y * Width].cell_type == CellType::Empty;

            return current && next;
        }

        return false;
    }

private:
    void set_next_rect(size_t index)
    {
        int32_t x = index % Width;
        int32_t y = index / Width;

        int32_t min_x = std::max(x - 2, 0);
        int32_t min_y = std::max(y - 2, 0);
        int32_t max_x = std::min<int32_t>(x + 2, Width - 1);
        int32_t max_y = std::min<int32_t>(y + 2, Height - 1);

        m_working_rect.min_x = std::min(m_working_rect.min_x, min_x);
        m_working_rect.min_y = std::min(m_working_rect.min_y, min_y);
        m_working_rect.max_x = std::max(m_working_rect.max_x, max_x);
        m_working_rect.max_y = std::max(m_working_rect.max_y, max_y);
    }


    void flip_rect()
    {
        m_dirty_rect.min_x = m_working_rect.min_x;
        m_dirty_rect.min_y = m_working_rect.min_y;
        m_dirty_rect.max_x = m_working_rect.max_x;
        m_dirty_rect.max_y = m_working_rect.max_y;

        m_working_rect.min_x = Width;
        m_working_rect.min_y = Height;
        m_working_rect.max_x = -1;
        m_working_rect.max_y = -1;
    }

public:
    void flip()
    {
        PROFILE_FUNCTION();

        size_t empty_cells = Width * Height;
        size_t current_empty_cells = 0;

        m_current_grid = m_next_grid;

        for (size_t i = 0; i < m_next_grid.size(); i++)
        {
            if (m_next_grid[i].cell_type == CellType::Empty)
            {
                current_empty_cells++;
            }

            m_next_grid[i] = Cell();
        }

        if (empty_cells == current_empty_cells)
        {
            m_should_die = true;
        }

        flip_rect();
    }

    bool should_die() const
    {
        return m_should_die;
    }

    void pre_draw()
    {
        if (m_dirty_rect.min_x > m_dirty_rect.max_x || m_dirty_rect.min_y > m_dirty_rect.max_y) return;

        BeginTextureMode(m_render_texture);
        ClearBackground(BLANK);

        // only draw the changed sections of the chunk
        int scissor_x = m_dirty_rect.min_x * CellSize;
        int scissor_y = m_dirty_rect.min_y * CellSize;
        int scissor_width = (m_dirty_rect.max_x - m_dirty_rect.min_x + 1) * CellSize;
        int scissor_height = (m_dirty_rect.max_y - m_dirty_rect.min_y + 1) * CellSize;

        BeginScissorMode(scissor_x, scissor_y, scissor_width, scissor_height);

        for (int x = m_dirty_rect.min_x; x <= m_dirty_rect.max_x; x++)
        {
            for (int y = m_dirty_rect.min_y; y <= m_dirty_rect.max_y; y++)
            {
                const Cell& current_cell = get_cell(x, y);

                DrawRectangle(x * CellSize, y * CellSize, CellSize, CellSize, current_cell.colour);
            }
        }

        EndScissorMode();
        EndTextureMode();
    }

    void draw()
    {
        // draw the chunk texture
        Rectangle sourceRec = {
            0.0f,                        
            0.0f,                        
            (float)m_render_texture.texture.width,
            -(float)m_render_texture.texture.height // flip texture
        };

        Vector2 position = {
            (float)m_position_x,
            (float)m_position_y
        };

        DrawTextureRec(m_render_texture.texture, sourceRec, position, WHITE);
    }

    void debug_draw()
    {
        DrawRectangleLines(m_position_x, m_position_y, Width * CellSize, Height * CellSize, GREEN);
        DrawRectangleLines(
        m_position_x + m_dirty_rect.min_x * CellSize,
        m_position_y + m_dirty_rect.min_y * CellSize,
        (m_dirty_rect.max_x - m_dirty_rect.min_x) * CellSize,
        (m_dirty_rect.max_y - m_dirty_rect.min_y) * CellSize,
        RED
    );
    }

//private:
    bool m_should_die = false;
    int32_t m_position_x = 0;
    int32_t m_position_y = 0;
    Rect m_dirty_rect;
    Rect m_working_rect;

    RenderTexture2D m_render_texture;
    std::array<Cell, Width * Height> m_current_grid;
    std::array<Cell, Width * Height> m_next_grid; 
};