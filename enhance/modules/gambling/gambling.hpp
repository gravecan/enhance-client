#ifndef GAMBLING_H_
#define GAMBLING_H_

#include <utils/imgui/imgui.h>
#include <vector>
#include <random>

namespace enhance::modules::gambling {
    class MinesGame {
    private:
        std::mt19937 rng;
        
        int countAdjacentMines(int x, int y);
        float calculateMultiplier(int tiles);
        
    public:
        bool window_open = false;
        float balance = 50.0f;
        float bet_amount = 1.0f;
        int num_mines = 3;
        int grid_size = 5;
        std::vector<std::vector<int>> grid;
        std::vector<std::vector<bool>> revealed;
        bool game_active = false;
        bool game_won = false;
        bool game_lost = false;
        int tiles_revealed = 0;
        int total_safe_tiles = 0;
        float current_multiplier = 1.0f;
        
        MinesGame();
        void render();
        bool isWindowOpen() const { return window_open; }
        void openWindow() { window_open = true; }
        void closeWindow() { window_open = false; resetGame(); }
        void resetGame();
        void generateGrid();
        void revealTile(int x, int y);
    };
    
    extern MinesGame g_mines_game;
}

#endif

