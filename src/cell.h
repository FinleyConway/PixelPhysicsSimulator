#pragma once

#include <stddef.h>

#include "raylib.h"

typedef struct CellChunk CellChunk; // forward declaration 

typedef enum CellMovement {
    CELL_MOVEMENT_MOVE_NONE          = 0b00000000,
    CELL_MOVEMENT_STATIONARY         = 0b00000001,
    CELL_MOVEMENT_MOVE_UP            = 0b00000010,
    CELL_MOVEMENT_MOVE_DOWN          = 0b00000100,
    CELL_MOVEMENT_MOVE_SIDEWAYS      = 0b00001000,
    CELL_MOVEMENT_MOVE_UP_DIAGONAL   = 0b00010000,
    CELL_MOVEMENT_MOVE_DOWN_DIAGONAL = 0b00100000,
} CellMovement;

typedef enum CellType {
    CELL_TYPE_EMPTY = 0,
    CELL_TYPE_SAND,
    CELL_TYPE_WATER,
    CELL_TYPE_STONE,
    CELL_TYPE_STEAM,
} Element;

typedef struct Cell {
    Element type;
    CellMovement movement;
    unsigned int velocity_x;
    unsigned int velocity_y;
    Color colour;
} Cell;

const static Cell DEFAULT_CELL = {
    CELL_TYPE_EMPTY,
    CELL_MOVEMENT_MOVE_NONE,
    0, 0,
    BLACK
};

bool move_cell_up(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell);

bool move_cell_down(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell );

bool move_cell_sideways(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell);

bool move_cell_up_diagonal(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell);

bool move_cell_down_diagonal(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell);

void draw_cell(unsigned int x, unsigned int y, Color colour);