#pragma once

#include "utils/point.hpp"

struct ChunkContext
{
    static constexpr int width = 64;
    static constexpr int height = 64;
    static constexpr int cell_size = 4;

    static constexpr Point min_chunk_pos = { -2, -2 };
    static constexpr Point max_chunk_pos = { +2, +2 };

    static constexpr int max_chunks = (max_chunk_pos.x - min_chunk_pos.x + 1) * (max_chunk_pos.y - min_chunk_pos.y + 1);
};