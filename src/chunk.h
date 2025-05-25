#pragma once

#include <array>
#include <cassert>
#include <algorithm>
#include <boost/container/static_vector.hpp>

#include "raylib.h"

#include "cell.h"
#include "point.h"
#include "int_rect.h"
#include "chunk_context.h"

class Chunk
{
private:
    struct CellChange
    {
        int src_index = 0;
        int dst_index = 0;
        Chunk* chunk = nullptr;
    };

public:
    Chunk(Point position) : m_position(position)
    {
        m_render_texture = LoadRenderTexture(c_width * c_cell_size, c_height * c_cell_size);

        reset_rect(m_final_rect);
        reset_rect(m_intermediate_rect);
    }

    ~Chunk()
    {
        UnloadRenderTexture(m_render_texture);
    }

    Point get_position() const
    {
        return m_position;
    }

    const Cell& get_cell(int index) const
    {
        assert(in_bounds(index) && "Chunk::get_cell out of bounds!");

        return m_grid[index];
    }

    const Cell& get_cell(Point position) const
    {
        assert(in_bounds(position) && "Chunk::get_cell out of bounds!");

        return get_cell(get_index(position));
    }

    void move_cell(Point from_position, Point to_position, Chunk* chunk)
    {
        assert(chunk != nullptr && "Chunk::move_cell chunk is nullptr!");

        // keep track of the changes
        m_changes.emplace_back(
            get_index(from_position),
            get_index(to_position),
            chunk
        );
    }

    void set_cell(int index, const Cell& cell) 
    {
        assert(in_bounds(index) && "Chunk::set_cell out of bounds!");

        // allows to overwrite the grid
        Cell& dest = m_grid[index];

        // checks if im filling or removing a cell
        if (dest.type == CellType::Empty && cell.type != CellType::Empty)
        {
            m_filled_cells++;
        }
        else if (dest.type != CellType::Empty && cell.type == CellType::Empty)
        {
            m_filled_cells--;
        }

        // set and flag grid
        dest = cell;
        m_drawn = false;
        
        // wake up chunk to apply changes
        set_next_rect(index);
    }

    void set_cell(Point position, const Cell& cell) 
    {
        set_cell(get_index(position), cell);
    }

    bool in_bounds(Point position) const
    {
        return position.x >= 0 && position.y >= 0 && position.x < c_width && position.y < c_height;
    }

    bool in_bounds(int index) const
    {
        return index >= 0 && index < m_grid.size();
    }

    void wake_up(Point position)
    {
        assert(in_bounds(position) && "Chunk::wake_up out of bounds!");

        // wake up change by modifying to iteration bounds
        set_next_rect(get_index(position));
    }

    bool is_empty(Point position) const
    {
        assert(in_bounds(position) && "Chunk::is_empty out of bounds!");

        return m_grid[get_index(position)].type == CellType::Empty;
    }

    const IntRect& get_current_rect() const
    {
        return m_dirty_rect;
    }

    void apply_moved_cells()
    {
        if (m_changes.empty()) return;
        
        // sort changes by destination
        std::sort(m_changes.begin(), m_changes.end(), 
            [](const auto& a, const auto& b) 
        {
            return a.dst_index < b.dst_index;
        });

        int prev_iter = 0;
        m_changes.emplace_back(-1, -1, nullptr);
        
        for (int i = 0; i < m_changes.size() - 1; i++)
        {
            if (m_changes[i].dst_index != m_changes[i + 1].dst_index)
            {
                int chosen = GetRandomValue(i, prev_iter);
                auto& change = m_changes[chosen];

                // swap cells from the source to destination
                set_cell(change.dst_index, change.chunk->get_cell(change.src_index));
                change.chunk->set_cell(change.src_index, Cell());

                prev_iter = i + 1;
            }
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
                    DrawRectangle(x * c_cell_size, y * c_cell_size, c_cell_size, c_cell_size, current_cell.colour);
                }
            }
        }

        EndTextureMode();

        m_drawn = true;
    }

    void draw(bool debug) const
    {
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
            DrawRectangleLines(m_position.x, m_position.y, c_width * c_cell_size, c_height * c_cell_size, GREEN);
            DrawRectangleLines(
                m_position.x + m_dirty_rect.min_x * c_cell_size,
                m_position.y + m_dirty_rect.min_y * c_cell_size,
                (m_dirty_rect.max_x - m_dirty_rect.min_x + 1) * c_cell_size,
                (m_dirty_rect.max_y - m_dirty_rect.min_y + 1) * c_cell_size,
                    (m_dirty_rect.min_x > m_dirty_rect.max_x || m_dirty_rect.min_y > m_dirty_rect.max_y) ? BLUE : RED
            );
            DrawRectangleLines(
                m_position.x + m_final_rect.min_x * c_cell_size,
                m_position.y + m_final_rect.min_y * c_cell_size,
                (m_final_rect.max_x - m_final_rect.min_x + 1) * c_cell_size,
                (m_final_rect.max_y - m_final_rect.min_y + 1) * c_cell_size,
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
    int get_index(Point position) const
    {
        return position.x + position.y * c_width;
    }

    void set_next_rect(int index)
    {
        int x = index % c_width;
        int y = index / c_width;

        int min_x = std::max(x - 2, 0);
        int min_y = std::max(y - 2, 0);
        int max_x = std::min(x + 2, c_width - 1);
        int max_y = std::min(y + 2, c_height - 1);

        m_intermediate_rect.min_x = std::min(m_intermediate_rect.min_x, min_x);
        m_intermediate_rect.min_y = std::min(m_intermediate_rect.min_y, min_y);
        m_intermediate_rect.max_x = std::max(m_intermediate_rect.max_x, max_x);
        m_intermediate_rect.max_y = std::max(m_intermediate_rect.max_y, max_y);
    }

    void generate_bounds() 
    {
        reset_rect(m_final_rect);

        for (int x = 0; x < c_width; x++)
        {
            for (int y = 0; y < c_height; y++)
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
        rect.min_x = c_width;
        rect.min_y = c_height;
        rect.max_x = -1;
        rect.max_y = -1;
    }

private:
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