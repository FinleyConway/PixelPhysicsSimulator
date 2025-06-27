#pragma once

#include <vector>

#include <raylib.h>

#include "core/cell.hpp"
#include "utils/point.hpp"
#include "utils/int_rect.hpp"

class Chunk
{
public:
    Chunk(Point position, Point size, int cell_size);
    ~Chunk();

    Point get_position() const;
    const IntRect& get_current_rect() const;

    Cell& get_cell(int index);
    Cell& get_cell(Point position);

    void set_cell(int index, const Cell& cell);
    void set_cell(Point position, const Cell& cell);

    void move_cell(Point from_position, Point to_position, bool swap, Chunk* chunk);

    bool in_bounds(int index) const;
    bool in_bounds(Point position) const;
    
    void wake_up(Point position);

    bool is_empty(int index) const;
    bool is_empty(Point position) const;

    void apply_moved_cells();
    void update_rect();

    void pre_draw();
    void draw(bool debug) const;

    bool should_remove() const;

private:
    int get_index(Point position) const;

    void set_next_rect(int index);
    void reset_rect(IntRect& rect);

private:
    struct CellChange
    {
        int src_index = 0;
        int dst_index = 0;
        bool swap = false;
        Chunk* chunk = nullptr;
    };

private:
    Point m_position;
    Point m_size;
    int m_cell_size = 0;

    int m_filled_cells = 0;
    bool m_drawn = false;

    IntRect m_intermediate_rect;
    IntRect m_dirty_rect;

    std::vector<CellChange> m_changes;
    std::vector<Cell> m_grid;
    RenderTexture2D m_render_texture;
};