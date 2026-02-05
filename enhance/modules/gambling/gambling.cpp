#include "gambling.hpp"
#include <cmath>
#include <algorithm>
#include <string>

namespace enhance::modules::gambling {

    MinesGame::MinesGame() : rng(std::random_device{}()) {
        grid.resize(grid_size, std::vector<int>(grid_size, 0));
        revealed.resize(grid_size, std::vector<bool>(grid_size, false));
    }

    void MinesGame::generateGrid() {
        for (int i = 0; i < grid_size; i++) {
            for (int j = 0; j < grid_size; j++) {
                grid[i][j] = 0;
                revealed[i][j] = false;
            }
        }

        std::vector<std::pair<int, int>> positions;
        for (int i = 0; i < grid_size; i++) {
            for (int j = 0; j < grid_size; j++) {
                positions.push_back({ i, j });
            }
        }

        std::shuffle(positions.begin(), positions.end(), rng);

        for (int i = 0; i < num_mines && i < (int)positions.size(); i++) {
            int x = positions[i].first;
            int y = positions[i].second;
            grid[x][y] = -1;
        }

        total_safe_tiles = grid_size * grid_size - num_mines;
        tiles_revealed = 0;
        current_multiplier = 1.0f;
    }

    int MinesGame::countAdjacentMines(int x, int y) {
        int count = 0;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && nx < grid_size && ny >= 0 && ny < grid_size) {
                    if (grid[nx][ny] == -1) count++;
                }
            }
        }
        return count;
    }

    float MinesGame::calculateMultiplier(int tiles) {
        if (tiles == 0) return 1.0f;
        float base = 1.0f + (float)num_mines * 0.1f;
        return base * (1.0f + tiles * 0.05f);
    }

    void MinesGame::revealTile(int x, int y) {
        if (x < 0 || x >= grid_size || y < 0 || y >= grid_size) return;
        if (revealed[x][y]) return;

        revealed[x][y] = true;

        if (grid[x][y] == -1) {
            game_lost = true;
            game_active = false;
            return;
        }

        tiles_revealed++;
        current_multiplier = calculateMultiplier(tiles_revealed);

        if (tiles_revealed >= total_safe_tiles) {
            game_won = true;
            game_active = false;
            balance += bet_amount * current_multiplier;
        }
    }

    void MinesGame::resetGame() {
        game_active = false;
        game_won = false;
        game_lost = false;
        tiles_revealed = 0;
        current_multiplier = 1.0f;
        generateGrid();
    }

    void MinesGame::render() {
    }

    MinesGame g_mines_game;

}