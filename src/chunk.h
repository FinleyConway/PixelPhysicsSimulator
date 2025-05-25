    #pragma once

    #include <array>
    #include <cassert>
    #include <algorithm>
    #include <boost/container/static_vector.hpp>

    #include "raylib.h"

    #include "point.h"
    #include "int_rect.h"
    #include "chunk_context.h"

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

    class Chunk
    {
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

        const Cell& get_cell(Point position) const
        {
            assert(in_bounds(position) && "Chunk::get_cell out of bounds!");

            return m_grid[get_index(position)];
        }

        void set_cell(Point position, const Cell& cell)
        {
            assert(in_bounds(position) && "Chunk::set_cell out of bounds!");

            int index = get_index(position);

            m_changes.emplace_back(index, cell);
            m_drawn = false;

            set_next_rect(index);
        }

        bool in_bounds(Point position) const
        {
            return position.x >= 0 && position.y >= 0 && position.x < c_width && position.y < c_height;
        }

        void wake_up(Point position)
        {
            assert(in_bounds(position) && "Chunk::wake_up out of bounds!");

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

        void apply_cells()
        {
            if (m_changes.empty()) return;

            for (const auto& [index, cell] : m_changes)
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

        boost::container::static_vector<std::pair<int, Cell>, c_width * c_height> m_changes;
        std::array<Cell, c_width * c_height> m_grid;
        RenderTexture2D m_render_texture;
    };