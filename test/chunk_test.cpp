#include <catch2/catch_test_macros.hpp>
#include <raylib.h>


#include "core/cell.hpp"
#include "utils/int_rect.hpp"
#include "simulation/chunk.hpp"

void CustomLog2(int msgType, const char *text, va_list args)
{ 
    return;
}

TEST_CASE("Chunk Class Test", "[Chunk]")
{
    SetTraceLogCallback(CustomLog2);
    InitWindow(1280, 720, "Pixel Physics");

    Chunk chunk({ 0, 0 });

    SECTION("Chunk position")
    {
        REQUIRE(chunk.get_position() == Point(0, 0));
    }

    SECTION("Initially all cells are empty") 
    {
        for (int x = 0; x < ChunkContext::width; x++) 
        {
            for (int y = 0; y < ChunkContext::height; y++) 
            {
                REQUIRE(chunk.is_empty({x, y}));
            }
        }

        REQUIRE(chunk.should_remove() == true);
    }

    SECTION("Set and get cell") 
    {
        Point pos = { 2, 3 };
        Cell cell = { CellType::Sand, 0 };

        chunk.set_cell(pos, cell);
        const Cell& result = chunk.get_cell(pos);

        REQUIRE(result.type == CellType::Sand);
        REQUIRE_FALSE(chunk.should_remove());
    }

    SECTION("Out of bounds") 
    {
        Point outOfBounds = { -1, -1 };

        REQUIRE_FALSE(chunk.in_bounds(outOfBounds));
        REQUIRE_FALSE(chunk.in_bounds(ChunkContext::width * ChunkContext::height));
    }

    SECTION("Move cell in chunk")
    {
        chunk.set_cell(0, { CellType::Sand, 0 });

        chunk.move_cell({ 0, 0 }, { 1, 1 }, &chunk);

        REQUIRE(chunk.get_cell(0).type == CellType::Sand);
        REQUIRE(chunk.get_cell({ 1, 1 }).type == CellType::Empty);

        chunk.apply_moved_cells();

        REQUIRE(chunk.get_cell(0).type == CellType::Empty);
        REQUIRE(chunk.get_cell({ 1, 1 }).type == CellType::Sand);
    }

    SECTION("Wake up a region updates intermediate rect") 
    {
        Point pos = { 5, 5 };
        chunk.wake_up(pos);
        chunk.update_rect();

        IntRect rect = chunk.get_current_rect();

        REQUIRE(rect.min_x <= 5);
        REQUIRE(rect.max_x >= 5);
        REQUIRE(rect.min_y <= 5);
        REQUIRE(rect.max_y >= 5);
    }

    CloseWindow();
}