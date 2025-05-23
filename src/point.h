#pragma once

#include <functional>
#include <string>

struct Point
{
    int x = 0;
    int y = 0;

    bool operator==(Point other) const
    {
        return x == other.x && y == other.y;
    }

    std::string to_str() const
    {
        return "Point(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

template<>
struct std::hash<Point>
{
    size_t operator()(Point p) const noexcept
    {
        // https://stackoverflow.com/a/55083395
        size_t hash = std::hash<int>()(p.x);
        hash <<= sizeof(size_t) * 4;
        hash ^= std::hash<int>()(p.y);

        return std::hash<size_t>()(hash);
    }
};