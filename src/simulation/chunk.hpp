#pragma once

#include <array>
#include <boost/container/static_vector.hpp>

#include <raylib.h>

#include "core/cell.hpp"
#include "core/chunk_context.hpp"

#include "utils/point.hpp"
#include "utils/int_rect.hpp"

class Chunk
{
public:
    Chunk(Point position);
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
    void generate_bounds();
    void reset_rect(IntRect& rect);

private:
    struct CellChange
    {
        int src_index = 0;
        int dst_index = 0;
        bool swap = false;
        Chunk* chunk = nullptr;
    };

    static constexpr int c_width = ChunkContext::width;
    static constexpr int c_height = ChunkContext::height;
    static constexpr int c_cell_size = ChunkContext::cell_size;

private:
    Point m_position;
    int m_filled_cells = 0;
    bool m_drawn = false;

    IntRect m_final_rect;
    IntRect m_intermediate_rect;
    IntRect m_dirty_rect;

    boost::container::static_vector<CellChange, c_width * c_height> m_changes;
    std::array<Cell, c_width * c_height> m_grid;
    RenderTexture2D m_render_texture;
};