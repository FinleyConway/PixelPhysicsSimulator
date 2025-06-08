#include <catch2/catch_test_macros.hpp>
#include <raylib.h>

#include "simulation/chunk_manager.hpp"
#include "simulation/chunk_worker.hpp"
#include "core/cell.hpp"
#include "utils/colour.hpp"

class ChunkUpdater : public ChunkWorker
{
public:
    ChunkUpdater(ChunkManager& manager, Chunk* chunk) : ChunkWorker(manager, chunk) { }

protected:
    void update_cell(const Cell& cell, int x, int y)
    {
    }
};

void CustomLog(int msgType, const char *text, va_list args)
{ 
  return;
}

TEST_CASE("Chunk Manager Class Test", "[ChunkManager]")
{
    SetTraceLogCallback(CustomLog);
    InitWindow(1280, 720, "Pixel Physics");

    ChunkManager manager;

    SECTION("Get cell from chunk")
    {
        // need a unique ChunkContext for testing

        const Cell* cell = manager.get_cell(0, 0);

        REQUIRE(cell != nullptr);
        REQUIRE(cell->type == CellType::Empty);
        REQUIRE(manager.get_total_chunks() == 1);

        // add tests for out of bounds when unique context is a thing
    }

    SECTION("Set cell in chunk")
    {
        manager.set_cell(0, 0, Cell::Sand);

        const Cell* cell = manager.get_cell(0, 0);

        REQUIRE(cell != nullptr);
        REQUIRE(cell->type == CellType::Sand);
        REQUIRE(manager.get_total_chunks() == 1);

        // add tests for out of bounds when unique context is a thing
    }

    SECTION("Move cell in chunk")
    {
        manager.set_cell(0, 0, Cell::Sand);

        manager.move_cell(0, 0, 20, 20);

        const Cell* b_from = manager.get_cell(0, 0);
        const Cell* b_to = manager.get_cell(20, 20);

        REQUIRE(b_from != nullptr);
        REQUIRE(b_from->type == CellType::Sand);
        REQUIRE(b_to != nullptr);
        REQUIRE(b_to->type == CellType::Empty); // hasnt moved yet
        REQUIRE(manager.get_total_chunks() == 1);

        for (int i = 0; i < 10; i++)
            manager.update<ChunkUpdater>(1.0f / 60.0f); // moved

        const Cell* from = manager.get_cell(0, 0);
        const Cell* to = manager.get_cell(20, 20);

        REQUIRE(from != nullptr);
        REQUIRE(from->type == CellType::Empty);
        REQUIRE(to != nullptr);
        REQUIRE(to->type == CellType::Sand);
        REQUIRE(manager.get_total_chunks() == 1);
    }

    SECTION("Move within same chunk")
    {
        manager.set_cell(2, 2, Cell::Sand);

        manager.move_cell(2, 2, 3, 3); // within same chunk

        for (int i = 0; i < 10; i++)
            manager.update<ChunkUpdater>(1.0f / 60.0f); // moved

        REQUIRE(manager.get_cell(2, 2)->type == CellType::Empty);
        REQUIRE(manager.get_cell(3, 3)->type == CellType::Sand);
    }

    SECTION("Move from empty cell should not affect destination")
    {
        manager.set_cell(10, 10, Cell::Sand);

        manager.move_cell(10, 10, 12, 12);

        for (int i = 0; i < 10; i++)
            manager.update<ChunkUpdater>(1.0f / 60.0f); // moved

        REQUIRE(manager.get_cell(10, 10)->type == CellType::Empty);
        REQUIRE(manager.get_cell(12, 12)->type == CellType::Empty);
    }

    SECTION("Move to occupied cell")
    {
        manager.set_cell(5, 5, Cell::Sand);
        manager.set_cell(6, 6, Cell::Water);

        manager.move_cell(5, 5, 6, 6);
        for (int i = 0; i < 10; i++)
            manager.update<ChunkUpdater>(1.0f / 60.0f); // moved

        // Depending on logic:
        REQUIRE(manager.get_cell(5, 5)->type == CellType::Empty);
        REQUIRE(manager.get_cell(6, 6)->type == CellType::Sand);
    }

    SECTION("Cell empty in chunk")
    {
        REQUIRE(manager.is_empty(0, 0) == true);
    }

    SECTION("Cell not empty in chunk")
    {
        manager.set_cell(0, 0, Cell::Sand);

        REQUIRE(manager.is_empty(0, 0) == false);
    }

    // test grid conversions when context is unique

    CloseWindow();
}