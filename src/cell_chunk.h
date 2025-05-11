#pragma once

#include <array>
#include <cstdint>
#include <cstring>

enum class CellType
{
    Empty = 0,
    Bob,
};

struct Cell
{
    CellType cell_type;
};

template<size_t Width, size_t Height>
class CellChunk 
{
public:
    CellChunk(int32_t position_x, int32_t position_y) :
        m_PositionX(position_x),
        m_PositionY(position_y)
    {
    }

    size_t get_area() const
    {
        return Width * Height;
    }

    size_t get_width() const
    {
        return Width;
    }

    size_t get_height() const
    {
        return Height;
    }

    int32_t get_position_x() const
    {
        return m_PositionX;
    }

    int32_t get_position_y() const
    {
        return m_PositionY;
    }

    bool in_bounds(size_t x, size_t y) const
    {
        return x < Width && y < Height;
    }

    bool is_empty(size_t x, size_t y) const
    {
        if (in_bounds(x, y))
        {
            bool current = m_CurrentGrid[x + y * Width].cell_type == CellType::Empty;
            bool next = m_NextGrid[x + y * Width].cell_type == CellType::Empty;

            return current && next;
        }

        return false;
    }

    const Cell* get_cell(size_t x, size_t y) const
    {
        if (in_bounds(x, y))
        {
            return &m_CurrentGrid[x + y * Width];
        }

        return nullptr;
    }

    bool set_cell(size_t x, size_t y, const Cell& cell)
    {
        if (in_bounds(x, y))
        {
            m_NextGrid[x + y * Width] = cell;

            return true;
        }

        return false;
    }

    void update()
    {
        for (size_t x = 0; x < Width; x++)
        {
            for (size_t y = 0; y < Height; y++)
            {
                // update grid
                const Cell* cell = get_cell(x, y);
            }
        }

        m_CurrentGrid = m_NextGrid;
        m_NextGrid.fill(Cell());
    }

    void draw() const
    {

    }

private:
    std::array<Cell, Width * Height> m_CurrentGrid;
    std::array<Cell, Width * Height> m_NextGrid;
    int32_t m_PositionX = 0;
    int32_t m_PositionY = 0;
};