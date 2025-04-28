#pragma once

#define GRID_WIDTH 128
#define GRID_HEIGHT 128
#define GRID_SIZE GRID_WIDTH * GRID_HEIGHT 
#define CELL_SIZE 4

typedef struct Color Color;

typedef enum Element {
    ELEMENT_EMPTY = 0,
    ELEMENT_SAND,
    ELEMENT_WATER,
    ELEMENT_STONE
} Element;

typedef struct ElementGrid {
    Element current_grid[GRID_SIZE];
    Element next_grid[GRID_SIZE];
} ElementGrid;


ElementGrid create_grid();

bool in_bounds_of_grid(int x, int y);

Element get_element_in_grid(const ElementGrid* grid, int x, int y);

bool overwrite_element_in_grid(ElementGrid* grid, int x, int y, Element element);

bool set_element_in_grid(ElementGrid* grid, int x, int y, Element element);

bool is_empty_in_grid(const ElementGrid* grid, int x, int y);

void update_grid(ElementGrid* grid);

void draw_element(int x, int y, Color colour);

void draw_grid(const ElementGrid* grid);