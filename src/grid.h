#pragma once

#include "cell.h"

typedef struct CellChunk {
    Cell* current_grid;
    Cell* next_grid;
    size_t width;
    size_t height;
    unsigned int cell_size;
} CellChunk;


CellChunk create_chunk(size_t width, size_t height, unsigned int cell_size);

void destroy_chunk(CellChunk* chunk);

bool in_bounds_of_chunk(const CellChunk* chunk, size_t x, size_t y);

Cell* get_cell_in_chunk(const CellChunk* chunk, size_t x, size_t y);

bool overwrite_cell_in_chunk(CellChunk* chunk, size_t x, size_t y, const Cell* cell);

bool set_cell_in_chunk(CellChunk* chunk, size_t x, size_t y, const Cell* cell);

bool is_empty_in_chunk(const CellChunk* chunk, size_t x, size_t y);

void update_chunk(CellChunk* chunk);

void draw_chunk(const CellChunk* chunk);