#pragma once

#include "core/cell.hpp"
#include "simulation/chunk.hpp"
#include "simulation/chunk_worker.hpp"
#include "simulation/chunk_manager.hpp"

class ChunkUpdater : public ChunkWorker
{
public:
    ChunkUpdater(ChunkManager& manager, Chunk* chunk) : ChunkWorker(manager, chunk) { }

protected:
    void update_cell(const Cell& cell, int x, int y)
    {
        int dest_x = 0;
        int dest_y = 0;

        if (cell.type == CellType::Sand)
        {
            bool can_down = is_empty(x, y + 1);
            bool has_type_down = has_types_at<2>(x, y + 1, {
                CellType::Water,
                CellType::Smoke
            });
            bool can_hor = try_random_dver(x, y, dest_x, dest_y);

            // if (can_down)
            // {
            //     move_cell(x, y, x, y + 1);
            // }
            // else
            {
                push_cell(x, y, 5, -5);
            }
            // else if (has_type_down)
            // {
            //     swap_cells(x, y, x, y + 1);
            // }
            // else if (can_hor) 
            // {
            //     move_cell(x, y, dest_x, dest_y);
            // }
        }

        if (cell.type == CellType::Water)
        {
            if (is_empty(x, y + 1))
            {
                move_cell(x, y, x, y + 1);
            }
            else if (try_random_dver(x, y, dest_x, dest_y))
            {
                move_cell(x, y, dest_x, dest_y);
            }
            else if (try_random_dhor(x, y, dest_x, dest_y)) 
            { 
                move_cell(x, y, dest_x, dest_y);
            }
        }

        if (cell.type == CellType::Smoke)
        {
            if (is_empty(x, y - 1))
            {
                move_cell(x, y, x, y - 1);
            }
            else if (try_random_uver(x, y, dest_x, dest_y))
            {
                move_cell(x, y, dest_x, dest_y);
            }
            else if (try_random_uhor(x, y, dest_x, dest_y)) 
            { 
                move_cell(x, y, dest_x, dest_y);
            }
        }
    }

private:
    template<int N>
    bool has_types_at(int x, int y, std::array<CellType, N>&& type)
    {
        for (int i = 0; i < N; i++)
        {
            const Cell* cell = get_cell(x, y);
            
            if (cell != nullptr && cell->type == type[i])
            {
                return true;
            }
        }

        return false;
    }

    bool try_random_dver(int x, int y, int& dest_x, int& dest_y)
    {
        Point dest = random_direction_movement({ x, y }, { x - 1, y + 1 }, { x + 1, y + 1 });

        if (dest != Point::zero())
        {
            dest_x = dest.x;
            dest_y = dest.y;

            return true;
        }

        return false;
    }

    bool try_random_uver(int x, int y, int& dest_x, int& dest_y)
    {
        Point dest = random_direction_movement({ x, y }, { x - 1, y - 1 }, { x + 1, y - 1 });

        if (dest != Point::zero())
        {
            dest_x = dest.x;
            dest_y = dest.y;

            return true;
        }

        return false;
    }

    bool try_random_dhor(int x, int y, int& dest_x, int& dest_y)
    {
        Point dest = random_direction_movement({ x, y }, { x - 1, y }, { x + 1, y });

        if (dest != Point::zero())
        {
            dest_x = dest.x;
            dest_y = dest.y;

            return true;
        }

        return false;
    }

    bool try_random_uhor(int x, int y, int& dest_x, int& dest_y)
    {
        Point dest = random_direction_movement({ x, y }, { x - 1, y }, { x + 1, y });

        if (dest != Point::zero())
        {
            dest_x = dest.x;
            dest_y = dest.y;

            return true;
        }

        return false;
    }

    Point random_direction_movement(Point from, Point to_a, Point to_b)
    {
        bool options[2] = { 
            is_empty(to_a.x, to_a.y), 
            is_empty(to_b.x, to_b.y) 
        };

        if (options[0] && options[1])
        {
            int r = rand() % 2;

            if (r == 0) return to_a;
            if (r == 1) return to_b;
        }
        else if (options[0]) return to_a;
        else if (options[1]) return to_b;

        return {};
    }
};