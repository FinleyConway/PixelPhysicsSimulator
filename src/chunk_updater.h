#pragma once

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
        bool up = is_empty(x, y - 1);
        bool up_left = is_empty(x - 1, y - 1);
        bool up_right = is_empty(x + 1, y - 1);

        bool left = is_empty(x - 1, y);
        bool right = is_empty(x + 1, y);

        bool down = is_empty(x, y + 1);
        bool down_left = is_empty(x - 1, y + 1);
        bool down_right = is_empty(x + 1, y + 1);

        if (cell.type == CellType::Sand)
        {
            if (down)
            {
                move_cell(x, y, x, y + 1, cell);
            }
            else if (down_left || down_right)
            {
                bool options[2] = { false, false };

                if (down_left) options[0] = true;
                if (down_right) options[1] = true;

                if (options[0] && options[1])
                {
                    int r = rand() % 2;

                    if (r == 0) move_cell(x, y, x - 1, y + 1, cell); 
                    if (r == 1) move_cell(x, y, x + 1, y + 1, cell); 
                }
                else if (options[0])
                {
                    move_cell(x, y, x - 1, y + 1, cell); 
                }
                else if (options[1])
                {
                    move_cell(x, y, x + 1, y + 1, cell); 
                }
            }
        }

        if (cell.type == CellType::Water)
        {
            if (down)
            {
                move_cell(x, y, x, y + 1, cell);
            }
            else if (down_left || down_right)
            {
                bool options[2] = { false, false };

                if (down_left) options[0] = true;
                if (down_right) options[1] = true;

                if (options[0] && options[1])
                {
                    int r = rand() % 2;

                    if (r == 0) move_cell(x, y, x - 1, y + 1, cell); 
                    if (r == 1) move_cell(x, y, x + 1, y + 1, cell); 
                }
                else if (options[0])
                {
                    move_cell(x, y, x - 1, y + 1, cell); 
                }
                else if (options[1])
                {
                    move_cell(x, y, x + 1, y + 1, cell); 
                }
            }
            else if (left || right)
            {
                bool options[2] = { false, false };

                if (left) options[0] = true;
                if (right) options[1] = true;

                if (options[0] && options[1])
                {
                    int r = rand() % 2;

                    if (r == 0) move_cell(x, y, x - 1, y, cell); 
                    if (r == 1) move_cell(x, y, x + 1, y, cell); 
                }
                else if (options[0])
                {
                    move_cell(x, y, x - 1, y, cell); 
                }
                else if (options[1])
                {
                    move_cell(x, y, x + 1, y, cell); 
                }
            }
        }
    }
};