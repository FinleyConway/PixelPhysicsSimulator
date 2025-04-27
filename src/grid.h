#pragma once

#include <assert.h>

typedef enum Element {
    ELEMENT_EMPTY = 0,
    ELEMENT_SAND,
    ELEMENT_WATER,
    ELEMENT_STONE
} Element;

typedef struct {
    Element* data;
    int width;
    int height;
} ElementGrid;

bool in_bounds(const ElementGrid* grid, int x, int y)
{
    return x >= 0 && x < grid->width && y >= 0 && y < grid->height;
}

Element get_element(const ElementGrid* grid, int x, int y)
{
    assert(in_bounds(grid, x, y));

    return grid->data[x + y * grid->width];
}

bool set_element(ElementGrid* grid, int x, int y, Element element)
{
    if (in_bounds(grid, x, y))
    {
        grid->data[x + y * grid->width] = element;

        return true;
    }

    return false;
}

bool has_element(const ElementGrid* grid, int x, int y, Element element)
{
    if (in_bounds(grid, x, y))
    {
        return get_element(grid, x, y) == element;
    }

    return false;
}

bool is_empty(const ElementGrid* grid, int x, int y)
{
    if (in_bounds(grid, x, y))
    {
        return get_element(grid, x, y) == ELEMENT_EMPTY;
    }

    return false;
}