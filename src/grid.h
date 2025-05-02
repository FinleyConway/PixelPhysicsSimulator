#pragma once

#include "raylib.h"

#include "cell.h"

#define CHUNK_WIDTH 128
#define CHUNK_HEIGHT 128
#define CHUNK_SIZE CHUNK_WIDTH * CHUNK_HEIGHT
#define CHUNK_CELL_SIZE 4

typedef struct CellChunk {
    RenderTexture2D render_texture;
    Cell current_grid[CHUNK_SIZE];
    Cell next_grid[CHUNK_SIZE];
    int position_x;
    int position_y;
    bool dirty;
} CellChunk;


CellChunk* create_chunk();

void destroy_chunk(CellChunk* chunk);

bool in_bounds_of_chunk(unsigned int x, unsigned int y);

const Cell* get_cell_in_chunk(const CellChunk* chunk, unsigned int x, unsigned int y);

bool overwrite_cell_in_chunk(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell);

bool set_cell_in_chunk(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell);

bool is_empty_in_chunk(const CellChunk* chunk, unsigned int x, unsigned int y);

void update_chunk(CellChunk* chunk);

void draw_chunk(CellChunk* chunk);


int mouse_to_grid_x(int mouse_x);

int mouse_to_grid_y(int mouse_y);