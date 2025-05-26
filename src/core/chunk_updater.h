#pragma once

#include "cell.h"
#include "simulation/chunk.h"
#include "simulation/chunk_worker.h"
#include "simulation/chunk_manager.h"

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
            if (is_empty(x, y + 1) || has_type_at(x, y + 1, CellType::Water))
            {
                move_cell(x, y, x, y + 1);
            }
            else if (try_random_ver(x, y, dest_x, dest_y)) 
            {
                move_cell(x, y, dest_x, dest_y);
            }
        }

        if (cell.type == CellType::Water)
        {
            if (is_empty(x, y + 1))
            {
                move_cell(x, y, x, y + 1);
            }
            else if (try_random_ver(x, y, dest_x, dest_y))
            {
                move_cell(x, y, dest_x, dest_y);
            }
            else if (try_random_hor(x, y, dest_x, dest_y)) 
            { 
                move_cell(x, y, dest_x, dest_y);
            }
        }
    }

private:
    bool has_type_at(int x, int y, CellType type)
    {
        const Cell* cell = get_cell(x, y);

        if (cell != nullptr)
        {
            return cell->type == type;
        }

        return false;
    }

    bool try_random_ver(int x, int y, int& dest_x, int& dest_y)
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

    bool try_random_hor(int x, int y, int& dest_x, int& dest_y)
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