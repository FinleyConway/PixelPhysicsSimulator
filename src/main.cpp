#include <iostream>

#include "cell_chunk.h"
#include "sandbox.h"

int main()
{
    //auto sandbox = Sandbox<10, 10>();
    auto sandbox = Sandbox();

    sandbox.set_cell(0, 0, Cell(
        CellType::Bob
    ));
    sandbox.set_cell(10, 0, Cell(
        CellType::Bob
    ));

    std::cout << sandbox.has_chunk(0, 0) << std::endl;
    std::cout << sandbox.has_chunk(1, 0) << std::endl;

    std::cout << "Total: " << sandbox.get_chunk_count() << std::endl;
}