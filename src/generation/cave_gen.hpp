#pragma once

#include <array>
#include <random>

#include <raylib.h>

#include "core/chunk_context.hpp"

class CaveGen
{
public:
    CaveGen()
    {
        m_texture = LoadRenderTexture(c_width * 8, c_height * 8);
    }

    ~CaveGen()
    {
        UnloadRenderTexture(m_texture);
    }

    void reset()
    {
        m_current.fill(0);
        //m_next.fill(0);
    }

    void generate_map()
    {
        add_noise();

        for (int i = 0; i < m_smooth_iter; i++)
        {
            smooth_map();
        }

        BeginTextureMode(m_texture);
        ClearBackground(BLANK);

        for (int x = 0; x < c_width; x++)
        {
            for (int y = 0; y < c_height; y++)
            {
                if (m_current[x + y * c_width] == 1)
                {
                    DrawRectangle(x * 8, y * 8, 8, 8, WHITE);
                }
            }
        }

        EndTextureMode();
    }

    void draw()
    {
        DrawTexture(m_texture.texture, 0, 0, WHITE);
    }

private:
    void add_noise()
    {
        static std::random_device seed;
        static std::mt19937 gen(seed());
        std::uniform_int_distribution<> dist{0, 100};
        std::uniform_int_distribution<> fill_percent{30, 60};

        for (int x = 0; x < c_width; x++)
        {
            for (int y = 0; y < c_height; y++)
            {
                m_current[x + y * c_width] = dist(gen) > fill_percent(gen) ? 1 : 0;
            }
        }
    }

    int get_neighour_count(int x, int y)
    {
        int count = 0;

        for (int n_x = x - 1; n_x <= x + 1; n_x++)
        {
            for (int n_y = y - 1; n_y <= y + 1; n_y++)
            {
                if (n_x >= 0 && n_x < c_width && n_y >= 0 && n_y < c_height)
                {
                    if (n_x != x || n_y != y )
                    {
                        count += m_current[n_x + n_y * c_width];
                    }
                }
            }
        }

        return count;
    }

    void smooth_map()
    {
        for (int x = 0; x < c_width; x++)
        {
            for (int y = 0; y < c_height; y++)
            {
                int neighourTiles = get_neighour_count(x, y);

                if (neighourTiles > 4)
                {
                    m_current[x + y * c_width] = 1;
                }
                else if (neighourTiles < 4)
                {
                    m_current[x + y * c_width] = 0;
                }
                else
                {
                    //m_next[x + y * c_width] = m_current[x + y * c_width];
                }
            }
        }

        //m_current = m_next;
        //m_next.fill(0);
    }

private:
    static constexpr int c_width = ChunkContext::width;
    static constexpr int c_height = ChunkContext::height;

private:
    int m_smooth_iter = 3; // 4
    //int m_random_fill_percent = 34; // 48
    //std::array<int, c_width * c_height> m_next;
    std::array<int, c_width * c_height> m_current;
    RenderTexture2D m_texture;
};