#pragma once

#include <array>
#include <cstdint>
#include <cstring>

#include "raylib.h"

#include "cell.h"

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

    bool set_cell(size_t x, size_t y, const Cell& cell)
    {
        if (in_bounds(x, y))
        {
            m_next_grid[x + y * Width] = cell;
            m_is_dirty = true;

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

public:
    void flip()
    {
        m_current_grid = m_next_grid;
        m_next_grid.fill(Cell());
    }

    void pre_draw()
    {
        // re-render chunk if dirty
        if (!m_is_dirty) return;

        BeginTextureMode(m_render_texture);
        ClearBackground(BLANK);

        for (size_t x = 0; x < Width; x++)
        {
            for (size_t y = 0; y < Height; y++)
            {
                const Cell& current_cell = get_cell(x, y);

                DrawRectangle(x * CellSize, y * CellSize, CellSize, CellSize, current_cell.colour);
            }
        }

        EndTextureMode();

        m_is_dirty = false; // reset flag
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

//private:
    int32_t m_position_x = 0;
    int32_t m_position_y = 0;
    bool m_is_dirty = true;

    RenderTexture2D m_render_texture;
    std::array<Cell, Width * Height> m_current_grid;
    std::array<Cell, Width * Height> m_next_grid; 
};