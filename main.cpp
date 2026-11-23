#include <raylib.h>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>

using namespace std;

// grid class 
class Grid {
    int rows;
    int cols;
    vector<uint8_t> cells;

public:
    //constructor
    Grid(int r, int c) : rows(r), cols(c), cells(r * c, 0) {}

    // getters
    int getRows() const { 
        return rows; 
    }

    int getCols() const {
         return cols; 
    }

    //get cell value
    uint8_t getValue(int r, int c) const {
        if (r < 0 || r >= rows || c < 0 || c >= cols) return 0;
        return cells[r * cols + c];
    }

    //set cell value
    void set(int r, int c, uint8_t value) {
        assert(r >= 0 && r < rows && c >= 0 && c < cols);
        cells[r * cols + c] = value;
    }

    //clear grid
    void clear(){ 
        fill(cells.begin(), cells.end(), 0); 
    }
};

//make the next generation
Grid nextGeneration(const Grid& current) {

    Grid next(current.getRows(), current.getCols());

    for (int r = 0; r < current.getRows(); ++r) {
        for (int c = 0; c < current.getCols(); ++c) {
            int live = 0;

            //check all 8 cells around the current cell
            for (int dr = -1; dr <= 1; ++dr)
                for (int dl = -1; dl <= 1; ++dl)
                    if (dr || dl) 
                        live += current.getValue(r + dr, c + dl);

            uint8_t alive = current.getValue(r, c);
            //apply rules
            if ((alive && (live == 2 || live == 3)) || (!alive && live == 3))
                next.set(r, c, 1);
        }
    }
    return next;
}

//bounding box struct
struct Box { int minR, maxR, minC, maxC; bool empty; };

Box boundingBox(const Grid& g) {
    Box b { g.getRows(), -1, g.getCols(), -1, true };
    for (int r = 0; r < g.getRows(); ++r) {
        for (int c = 0; c < g.getCols(); ++c) {
            if (g.getValue(r, c)) {
                b.empty = false;
                //update the bounding box coordinates
                if (r < b.minR) b.minR = r;
                if (r > b.maxR) b.maxR = r;
                if (c < b.minC) b.minC = c;
                if (c > b.maxC) b.maxC = c;
            }
        }
    }
    return b;
}

int main() {
    const int ROWS = 200;
    const int COLS = 200;
    const int WINDOW_W = 900;
    const int WINDOW_H = 700;
    const int MIN_CELL = 2;      
    const int MAX_CELL = 40;     
    const int PADDING = 4;       

    Grid grid(ROWS, COLS);

    
    grid.set(1, 2, 1);
    grid.set(2, 3, 1);
    grid.set(3, 1, 1); 
    grid.set(3, 2, 1); 
    grid.set(3, 3, 1);

    bool running = false;
    bool autoFit = true;           
    int stepEvery = 6;             
    int frameCounter = 0;

    //cell size and offsets for panning
    float cellSize = 8.0f;
    float offsetX = 0.0f;          
    float offsetY = 0.0f;

    InitWindow(WINDOW_W, WINDOW_H, "Game of Life");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        
        if (IsKeyPressed(KEY_SPACE)){
            running = !running;
        }

        if (IsKeyPressed(KEY_C)){ 
            grid.clear(); 
        }
        
        if (IsKeyPressed(KEY_F))     
            autoFit = true;

        
        if (IsKeyPressed(KEY_R)) {
            for (int r = 0; r < ROWS; ++r)
                for (int c = 0; c < COLS; ++c)
                    grid.set(r, c, GetRandomValue(0, 4) == 0 ? 1 : 0);
        }

        if (IsKeyDown(KEY_LEFT))  { autoFit = false; offsetX -= 5.0f / cellSize * 10; }
        if (IsKeyDown(KEY_RIGHT)) { autoFit = false; offsetX += 5.0f / cellSize * 10; }
        if (IsKeyDown(KEY_UP))    { autoFit = false; offsetY -= 5.0f / cellSize * 10; }
        if (IsKeyDown(KEY_DOWN))  { autoFit = false; offsetY += 5.0f / cellSize * 10; }

        
        if (autoFit) {
            Box b = boundingBox(grid);
            int minR, maxR, minC, maxC;
            if (b.empty) {
                minR = 0; maxR = ROWS - 1;
                minC = 0; maxC = COLS - 1;
            } else {
                minR = max(0, b.minR - PADDING);
                maxR = min(ROWS - 1, b.maxR + PADDING);
                minC = max(0, b.minC - PADDING);
                maxC = min(COLS - 1, b.maxC + PADDING);
            }
            int viewRows = maxR - minR + 1;
            int viewCols = maxC - minC + 1;

            
            float sx = (float)WINDOW_W / viewCols;
            float sy = (float)WINDOW_H / viewRows;
            cellSize = min(sx, sy);
            cellSize = max((float)MIN_CELL, min((float)MAX_CELL, cellSize));

            
            float worldCenterX = (minC + maxC + 1) / 2.0f;
            float worldCenterY = (minR + maxR + 1) / 2.0f;

            //calculate offsets to center the view
            offsetX = worldCenterX - (WINDOW_W / 2.0f) / cellSize;
            offsetY = worldCenterY - (WINDOW_H / 2.0f) / cellSize;
        }

        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int mx = GetMouseX();
            int my = GetMouseY();
            int c = (int)(offsetX + mx / cellSize);
            int r = (int)(offsetY + my / cellSize);
            if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
                grid.set(r, c, grid.getValue(r, c) ? 0 : 1);
        }

        
        if (running) {
            if (++frameCounter >= stepEvery) {
                grid = nextGeneration(grid);
                frameCounter = 0;
            }
        }

        
        BeginDrawing();
        ClearBackground(BLACK);

        
        int firstCol = max(0, (int)offsetX);
        int lastCol  = min(COLS - 1, (int)(offsetX + WINDOW_W / cellSize) + 1);
        int firstRow = max(0, (int)offsetY);
        int lastRow  = min(ROWS - 1, (int)(offsetY + WINDOW_H / cellSize) + 1);

        for (int r = firstRow; r <= lastRow; ++r) {
            for (int c = firstCol; c <= lastCol; ++c) {
                if (grid.getValue(r, c)) {
                    float x = (c - offsetX) * cellSize;
                    float y = (r - offsetY) * cellSize;
                    float s = cellSize > 3 ? cellSize - 1 : cellSize;  
                    DrawRectangle((int)x, (int)y, (int)s, (int)s, RAYWHITE);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}