#include "grid.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"
#include "raylib.h"

CellChunk* create_chunk()
{
    CellChunk* chunk = malloc(sizeof(CellChunk));

	if (chunk != NULL)
	{
		// init grids
		memset(chunk->current_grid, 0, CHUNK_SIZE * sizeof(Cell));
		memset(chunk->next_grid, 0, CHUNK_SIZE * sizeof(Cell));

		// set up render texture and dirty flag
		chunk->render_texture = LoadRenderTexture(
			CHUNK_WIDTH * CHUNK_CELL_SIZE, 
			CHUNK_HEIGHT * CHUNK_CELL_SIZE
		);
        chunk->position_x = 0;
        chunk->position_y = 0;
		chunk->dirty = false;
	}

    return chunk;
}

void destroy_chunk(CellChunk* chunk)
{
    assert(chunk != NULL);

    UnloadRenderTexture(chunk->render_texture);
    free(chunk);
    chunk = NULL;
}

bool in_bounds_of_chunk(unsigned int x, unsigned int y)
{
    return x < CHUNK_WIDTH && y < CHUNK_HEIGHT;
}

const Cell* get_cell_in_chunk(const CellChunk* chunk, unsigned int x, unsigned int y)
{
    assert(chunk != NULL);
    assert(in_bounds_of_chunk(x, y));

    return &chunk->current_grid[x + y * CHUNK_WIDTH];
}

bool overwrite_cell_in_chunk(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell)
{
    assert(chunk != NULL);

    if (cell == NULL) return false;

    if (in_bounds_of_chunk(x, y))
    {
        chunk->current_grid[x + y * CHUNK_WIDTH] = *cell;
        chunk->dirty = true;

        return true;
    }

    return false;
}

bool set_cell_in_chunk(CellChunk* chunk, unsigned int x, unsigned int y, const Cell* cell)
{
    assert(chunk != NULL);

    if (cell == NULL) return false;

    if (in_bounds_of_chunk(x, y))
    {
        chunk->next_grid[x + y * CHUNK_WIDTH] = *cell;
        chunk->dirty = true;

        return true;
    }

    return false;
}

bool is_empty_in_chunk(const CellChunk* chunk, unsigned int x, unsigned int y)
{
    assert(chunk != NULL);

    if (in_bounds_of_chunk(x, y))
    {
        bool current = chunk->current_grid[x + y * CHUNK_WIDTH].type == CELL_TYPE_EMPTY;
        bool next = chunk->next_grid[x + y * CHUNK_WIDTH].type == CELL_TYPE_EMPTY;

        return current && next;
    }

    return false;
}

void update_chunk(CellChunk* chunk)
{
    assert(chunk != NULL);

    // check all pixels
    for (unsigned int x = 0; x < CHUNK_WIDTH; x++)
    {
        for (unsigned int y = 0; y < CHUNK_HEIGHT; y++)
        {   
            const Cell* current_cell = get_cell_in_chunk(chunk, x, y);
            CellMovement movement = current_cell->movement;

            if      (movement & CELL_MOVEMENT_MOVE_UP            && move_cell_up(chunk, x, y, current_cell)) {}
            else if (movement & CELL_MOVEMENT_MOVE_UP_DIAGONAL   && move_cell_up_diagonal(chunk, x, y, current_cell)) {}
            else if (movement & CELL_MOVEMENT_MOVE_DOWN          && move_cell_down(chunk, x, y, current_cell)) {}
            else if (movement & CELL_MOVEMENT_MOVE_DOWN_DIAGONAL && move_cell_down_diagonal(chunk, x, y, current_cell)) {}
            else if (movement & CELL_MOVEMENT_MOVE_SIDEWAYS      && move_cell_sideways(chunk, x, y, current_cell)) {}
            else if (movement != CELL_MOVEMENT_MOVE_NONE)
            {
                set_cell_in_chunk(chunk, x, y, current_cell); // TEMP: will keep cells inside window
            }
        }
    }

    // copy next state to the new grid state
    memcpy(chunk->current_grid, chunk->next_grid, CHUNK_SIZE * sizeof(Cell));

    // reset next_grid
    memset(chunk->next_grid, 0, CHUNK_SIZE * sizeof(Cell));  
}

void draw_chunk(CellChunk* chunk)
{
    assert(chunk != NULL);

    // re-render chunk if dirty
    if (chunk->dirty)
    {
        BeginTextureMode(chunk->render_texture);
        ClearBackground(BLANK);

        for (unsigned int x = 0; x < CHUNK_WIDTH; x++)
        {
            for (unsigned int y = 0; y < CHUNK_HEIGHT; y++)
            {
                const Cell* current_cell = get_cell_in_chunk(chunk, x, y);

                draw_cell(x, y, current_cell->colour);
            }
        }

        EndTextureMode();
	    chunk->dirty = false; // reset flag
    }

    // draw the chunk texture
    Rectangle sourceRec = {
        0.0f,                        
        0.0f,                        
        (float)chunk->render_texture.texture.width,
        -(float)chunk->render_texture.texture.height // flip texture
    };

    Vector2 position = {
        chunk->position_x * CHUNK_WIDTH * CHUNK_CELL_SIZE,
        chunk->position_y * CHUNK_HEIGHT * CHUNK_CELL_SIZE
    };

    DrawTextureRec(chunk->render_texture.texture, sourceRec, position, WHITE);
}

int mouse_to_grid_x(int mouse_x)
{
    return mouse_x / CHUNK_CELL_SIZE;
}

int mouse_to_grid_y(int mouse_y)
{
    return mouse_y / CHUNK_CELL_SIZE;
}