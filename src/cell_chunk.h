#pragma once

#include <array>
#include <vector>

#include "raylib.h"

#include "point.h"
#include "int_rect.h"
#include "cell.h"

template<int TWidth, int THeight, int TCellSize>
class CellChunk
{
public:
    CellChunk(Point position);
    ~CellChunk();

    Point get_position() const;
    const IntRect& get_current_rect() const;

    const Cell& get_cell(Point position) const;
    void set_cell(Point position, const Cell& cell);
    bool is_empty(Point position) const;
    bool is_in_bounds(Point position) const;

    void wake_up(Point position);

    void apply_cells();
    void update_rect();
    void pre_draw();
    void draw(bool debug) const;
    bool should_remove() const;

private:
    void set_next_rect(int index);
    void generate_bounds();
    void reset_rect(IntRect& rect);

private:
    Point m_position;
    int m_filled_cells = 0;
    bool m_drawn = false;

    IntRect m_final_rect;
    IntRect m_intermediate_rect;
    IntRect m_dirty_rect;

    std::vector<std::pair<int, Cell>> m_changes; // look into a static_vector?
    std::array<Cell, TWidth * THeight> m_grid;
    RenderTexture2D m_render_texture;
};

#include "cell_chunk.inl"
