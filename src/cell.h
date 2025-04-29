#pragma once

#include <stddef.h>

#include "raylib.h"

typedef struct CellChunk CellChunk; // forward declaration 

typedef enum ElementProperty {
    ELEMENT_PROPERTY_MOVE_NONE          = 0b00000000,
    ELEMENT_PROPERTY_STATIONARY         = 0b00000001,
    ELEMENT_PROPERTY_MOVE_UP            = 0b00000010,
    ELEMENT_PROPERTY_MOVE_DOWN          = 0b00000100,
    ELEMENT_PROPERTY_MOVE_SIDEWAYS      = 0b00001000,
    ELEMENT_PROPERTY_MOVE_UP_DIAGONAL   = 0b00010000,
    ELEMENT_PROPERTY_MOVE_DOWN_DIAGONAL = 0b00100000,
    ELEMENT_PROPERTY_INSIDE_CHUNK       = 0b01000000,
} ElementProperty;

typedef enum CellType {
    CELL_TYPE_EMPTY = 0,
    CELL_TYPE_SAND,
    CELL_TYPE_WATER,
    CELL_TYPE_STONE,
    CELL_TYPE_STEAM,
} Element;

typedef struct Cell {
    Element type;
    ElementProperty property;
    Color colour;
} Cell;

bool move_cell_up(CellChunk* chunk, size_t x, size_t y, const Cell* cell);

bool move_cell_down(CellChunk* chunk, size_t x, size_t y, const Cell* cell );

bool move_cell_sideways(CellChunk* chunk, size_t x, size_t y, const Cell* cell);

bool move_cell_up_diagonal(CellChunk* chunk, size_t x, size_t y, const Cell* cell);

bool move_cell_down_diagonal(CellChunk* chunk, size_t x, size_t y, const Cell* cell);

void draw_cell(const CellChunk* chunk, size_t x, size_t y, Color colour);