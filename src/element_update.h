#pragma once

#include "grid.h"

int random_dir();

void update_sand(ElementGrid* grid, int x, int y);

void update_water(ElementGrid* grid, int x, int y);