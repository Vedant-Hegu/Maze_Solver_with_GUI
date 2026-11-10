// MazeSolver — raylib graphical maze visualizer
// Stage 5: Race mode, random/perfect maze generation with solvability check,
//          execution-time stat, pause/step/speed controls, g(n)/h(n)/f(n) explainer
//
// Build:
//   g++ -std=c++17 -O2 maze_gui.cpp -o maze_gui -lraylib -lopengl32 -lgdi32 -lwinmm
// Run:
//   ./maze_gui.exe
//
// Controls:
//   Row 1 - New Maze / Clear / BFS / DFS / A* / Race / Fullscreen
//   Row 2 - Maze type (Perfect/Random) / Difficulty / Pause / Step / Speed -+
//   F          - toggle fullscreen
//   Left-drag  - draw wall
//   Right-drag - erase wall

#include "raylib.h"
#include <vector>
#include <deque>
#include <queue>
#include <random>
#include <algorithm>
#include <cmath>
#include <string>
#include <functional>

int SIZE = 21;
int CELL = 26;
const int MARGIN = 20;
const int TOPBAR = 318;
int stepsPerFrame = 3;
const int SIDEBAR_W = 230;
bool paused = false;

const int mazeSizes[3] = {15, 21, 31};
int sizeIdx = 1; // index into mazeSizes, default 21

enum MazeType { PERFECT, RANDOM };
MazeType mazeType = PERFECT;
int difficultyIdx = 1; // 0=Easy 1=Medium 2=Hard
const float difficulties[3] = {0.20f, 0.30f, 0.40f};
const char* difficultyNames[3] = {"Easy", "Medium", "Hard"};

enum CellType { OPEN = 0, WALL = 1 };
enum VisState { NONE_, VISITED, FRONTIER, PATH };
enum AppMode { MODE_NORMAL, MODE_RACE };

struct Point { int x, y; bool operator==(const Point& o) const { return x==o.x && y==o.y; } };

std::vector<std::vector<int>> grid(SIZE, std::vector<int>(SIZE, WALL));
std::vector<std::vector<VisState>> state(SIZE, std::vector<VisState>(SIZE, NONE_));
Point start{1,1}, goal{SIZE-2, SIZE-2};

Color colBg       = {234, 238, 241, 255};
Color colRoad     = {255, 255, 255, 255};
Color colWall     = {200, 210, 216, 255};
Color colStart    = { 46, 158,  91, 255};
Color colEnd      = {214,  69,  69, 255};
Color colText     = { 26,  29,  33, 255};
Color colVisited  = {207, 227, 245, 255};
Color colFrontier = {243, 201, 105, 255};
Color colPath     = { 46, 158,  91, 255};
Color colBtn      = {255, 255, 255, 255};
Color colBtnHover = {230, 236, 240, 255};
Color colBtnBorder= {200, 210, 216, 255};
Color colAccent   = { 66, 133, 201, 255};

bool isDark = false;

void applyTheme(bool dark) {
    isDark = dark;
    if (dark) {
        colBg        = { 20,  24,  27, 255};
        colRoad      = { 38,  43,  47, 255};
        colWall      = { 58,  66,  70, 255};
        colStart     = { 52, 180, 103, 255};
        colEnd       = {224,  84,  84, 255};
        colText      = {236, 239, 241, 255};
        colVisited   = { 31,  58,  82, 255};
        colFrontier  = {138, 106,  31, 255};
        colPath      = { 52, 180, 103, 255};
        colBtn       = { 30,  35,  39, 255};
        colBtnHover  = { 42,  48,  52, 255};
        colBtnBorder = { 60,  68,  72, 255};
        colAccent    = { 66, 133, 201, 255};
    } else {
        colBg        = {234, 238, 241, 255};
        colRoad      = {255, 255, 255, 255};
        colWall      = {200, 210, 216, 255};
        colStart     = { 46, 158,  91, 255};
        colEnd       = {214,  69,  69, 255};
        colText      = { 26,  29,  33, 255};
        colVisited   = {207, 227, 245, 255};
        colFrontier  = {243, 201, 105, 255};
        colPath      = { 46, 158,  91, 255};
        colBtn       = {255, 255, 255, 255};
        colBtnHover  = {230, 236, 240, 255};
        colBtnBorder = {200, 210, 216, 255};
        colAccent    = { 66, 133, 201, 255};
    }
}

void carve(int x, int y, std::mt19937& rng) {
    grid[y][x] = OPEN;
    std::vector<std::pair<int,int>> dirs = {{0,-2},{0,2},{-2,0},{2,0}};
    std::shuffle(dirs.begin(), dirs.end(), rng);
    for (auto& [dx, dy] : dirs) {
        int nx = x+dx, ny = y+dy;
        if (nx>0 && nx<SIZE-1 && ny>0 && ny<SIZE-1 && grid[ny][nx]==WALL) {
            grid[y+dy/2][x+dx/2] = OPEN;
            carve(nx, ny, rng);
        }
    }
}

void resetState() { for (auto& row : state) std::fill(row.begin(), row.end(), NONE_); }

bool isSolvable() {
    std::vector<std::vector<bool>> seen(SIZE, std::vector<bool>(SIZE, false));
    std::deque<Point> q; q.push_back(start); seen[start.y][start.x] = true;
    while (!q.empty()) {
        Point c = q.front(); q.pop_front();
        if (c == goal) return true;
        const int dx[4]={1,-1,0,0}, dy[4]={0,0,1,-1};
        for (int i=0;i<4;i++) {
            int nx=c.x+dx[i], ny=c.y+dy[i];
            if (nx>=0&&ny>=0&&nx<SIZE&&ny<SIZE&&grid[ny][nx]==OPEN&&!seen[ny][nx]) {
                seen[ny][nx]=true; q.push_back({nx,ny});
            }
        }
    }
    return false;
}

void generatePerfectMaze(std::mt19937& rng) {
    for (auto& row : grid) std::fill(row.begin(), row.end(), WALL);
    carve(1, 1, rng);
}

// random walls by density, regenerated (up to 40 tries) until a path exists —
// never leaves the maze in an unverified/possibly-unsolvable state
void generateRandomMaze(std::mt19937& rng, float density) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    int tries = 0;
    do {
        for (auto& row : grid) std::fill(row.begin(), row.end(), WALL);
        for (int y=1;y<SIZE-1;y++) for (int x=1;x<SIZE-1;x++) grid[y][x] = dist(rng) < density ? WALL : OPEN;
        grid[start.y][start.x] = OPEN;
        grid[goal.y][goal.x] = OPEN;
        tries++;
    } while (!isSolvable() && tries < 40);
}

void resizeMaze(int newSize) {
    SIZE = newSize;
    grid.assign(SIZE, std::vector<int>(SIZE, WALL));
    state.assign(SIZE, std::vector<VisState>(SIZE, NONE_));
    start = {1, 1};
    goal = {SIZE-2, SIZE-2};
}

void generateMaze() {
    std::mt19937 rng{std::random_device{}()};
    if (mazeType == PERFECT) generatePerfectMaze(rng);
    else generateRandomMaze(rng, difficulties[difficultyIdx]);
    grid[start.y][start.x] = OPEN;
    grid[goal.y][goal.x] = OPEN;
    resetState();
}

std::vector<Point> neighborsOf(Point p) {
    std::vector<Point> result;
    const int dx[4] = {1,-1,0,0};
    const int dy[4] = {0,0,1,-1};
    for (int i=0;i<4;i++) {
        int nx=p.x+dx[i], ny=p.y+dy[i];
        if (nx>=0 && ny>=0 && nx<SIZE && ny<SIZE && grid[ny][nx]==OPEN)
            result.push_back({nx,ny});
    }
    return result;
}

int heuristic(Point a, Point b) { return std::abs(a.x-b.x) + std::abs(a.y-b.y); }

enum SolverStatus { IDLE, RUNNING, DONE_FOUND, DONE_NOTFOUND };

// Solver now owns its own visualization grid (myState) so several can run
// at once in race mode without overwriting each other.
struct Solver {
    std::string algo = "astar";
    SolverStatus status = IDLE;
    std::vector<std::vector<bool>> visited;
    std::vector<std::vector<Point>> cameFrom;
    std::vector<std::vector<int>> gScore;
    std::vector<std::vector<VisState>> myState =
        std::vector<std::vector<VisState>>(SIZE, std::vector<VisState>(SIZE, NONE_));
    std::deque<Point> dq;
    std::priority_queue<std::pair<int,Point>,
        std::vector<std::pair<int,Point>>,
        std::function<bool(const std::pair<int,Point>&,const std::pair<int,Point>&)>> pq{
            [](const std::pair<int,Point>& a, const std::pair<int,Point>& b){ return a.first > b.first; }
        };
    std::vector<Point> path;
    int pathIndex = 0;
    int exploredCount = 0;
    double startTime = 0, elapsedSec = -1;

    void start_(const std::string& which) {
        algo = which;
        status = RUNNING;
        exploredCount = 0;
        startTime = GetTime();
        elapsedSec = -1;
        myState.assign(SIZE, std::vector<VisState>(SIZE, NONE_));
        visited.assign(SIZE, std::vector<bool>(SIZE, false));
        cameFrom.assign(SIZE, std::vector<Point>(SIZE, {-1,-1}));
        gScore.assign(SIZE, std::vector<int>(SIZE, 1<<29));
        dq.clear();
        while (!pq.empty()) pq.pop();
        path.clear();
        pathIndex = 0;

        visited[start.y][start.x] = true;
        if (algo == "astar") {
            gScore[start.y][start.x] = 0;
            pq.push({heuristic(start, goal), start});
        } else {
            dq.push_back(start);
        }
    }

    void reconstruct() {
        path.clear();
        Point p = goal;
        while (!(p == start)) {
            path.push_back(p);
            p = cameFrom[p.y][p.x];
        }
        std::reverse(path.begin(), path.end());
    }

    void step() {
        if (status == DONE_FOUND) {
            // just advance the reveal counter; drawMaze() draws the line using
            // path[0..pathIndex) in the correct start->goal order, not grid scan order
            if (pathIndex < (int)path.size()) pathIndex++;
            return;
        }
        if (status != RUNNING) return;

        Point cur; bool haveCur = false;
        if (algo == "bfs" || algo == "dfs") {
            if (dq.empty()) { status = DONE_NOTFOUND; elapsedSec = GetTime() - startTime; return; }
            if (algo == "bfs") { cur = dq.front(); dq.pop_front(); }
            else { cur = dq.back(); dq.pop_back(); }
            haveCur = true;
        } else {
            if (pq.empty()) { status = DONE_NOTFOUND; elapsedSec = GetTime() - startTime; return; }
            cur = pq.top().second; pq.pop();
            haveCur = true;
        }
        if (!haveCur) return;

        exploredCount++;
        if (!(cur == start) && !(cur == goal)) myState[cur.y][cur.x] = VISITED;
        if (cur == goal) { status = DONE_FOUND; elapsedSec = GetTime() - startTime; reconstruct(); return; }

        for (Point np : neighborsOf(cur)) {
            if (algo == "astar") {
                int tentativeG = gScore[cur.y][cur.x] + 1;
                if (tentativeG < gScore[np.y][np.x]) {
                    gScore[np.y][np.x] = tentativeG;
                    cameFrom[np.y][np.x] = cur;
                    pq.push({tentativeG + heuristic(np, goal), np});
                    if (!visited[np.y][np.x]) {
                        visited[np.y][np.x] = true;
                        if (!(np == goal)) myState[np.y][np.x] = FRONTIER;
                    }
                }
            } else {
                if (!visited[np.y][np.x]) {
                    visited[np.y][np.x] = true;
                    cameFrom[np.y][np.x] = cur;
                    dq.push_back(np);
                    if (!(np == goal)) myState[np.y][np.x] = FRONTIER;
                }
            }
        }
    }
};

Solver solver;                 // used in normal mode
Solver raceSolvers[3];         // bfs, dfs, astar — used in race mode
const char* raceNames[3] = {"BFS", "DFS", "A*"};
const char* raceAlgos[3] = {"bfs", "dfs", "astar"};

void startRace() {
    for (int i = 0; i < 3; i++) raceSolvers[i].start_(raceAlgos[i]);
}

// ---------------- crisp font ----------------
Font appFont;
bool customFontLoaded = false;

void drawText(const std::string& text, int x, int y, int size, Color c) {
    DrawTextEx(appFont, text.c_str(), {(float)x, (float)y}, (float)size, 1.0f, c);
}
int textWidth(const std::string& text, int size) {
    return (int)MeasureTextEx(appFont, text.c_str(), (float)size, 1.0f).x;
}

// ---------------- simple button helper ----------------
struct Button {
    Rectangle rect;
    std::string label;
    bool highlighted = false;
};

bool drawButton(Button& b) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, b.rect);
    Color bg = b.highlighted ? colAccent : (hover ? colBtnHover : colBtn);
    Color fg = b.highlighted ? WHITE : colText;
    DrawRectangleRec(b.rect, bg);
    DrawRectangleLinesEx(b.rect, 1, colBtnBorder);
    int tw = textWidth(b.label, 22);
    drawText(b.label, b.rect.x + (b.rect.width - tw) / 2, b.rect.y + (b.rect.height - 24) / 2, 22, fg);
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// draws one maze grid (walls + optional vis state) at the given pixel offset/cell size.
// Visited/frontier cells are drawn as soft rounded tiles; the final PATH is instead drawn
// as a single smooth line threaded through the cell centers, on top of everything else.
// orderedPath / revealCount: the solver's path in actual start->goal travel order,
// and how many of those points to currently show (for the step-by-step reveal animation).
void drawMaze(int offsetX, int offsetY, int cellSize, const std::vector<std::vector<VisState>>* vis,
              const std::vector<Point>* orderedPath = nullptr, int revealCount = 0) {
    float radius = 0.28f;
    int gap = std::max(1, cellSize / 14);

    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            int px = offsetX + x * cellSize;
            int py = offsetY + y * cellSize;
            VisState v = vis ? (*vis)[y][x] : NONE_;

            Color c = (grid[y][x] == WALL) ? colWall : colRoad;
            if (v == VISITED) c = colVisited;
            else if (v == FRONTIER) c = colFrontier;

            Rectangle r = { (float)(px + gap), (float)(py + gap),
                             (float)(cellSize - gap*2), (float)(cellSize - gap*2) };
            DrawRectangleRounded(r, radius, 6, c);

            if (x == start.x && y == start.y) DrawRectangleRounded(r, 0.5f, 10, colStart);
            if (x == goal.x && y == goal.y)  DrawRectangleRounded(r, 0.5f, 10, colEnd);
        }
    }

    // draw the solved route as one smooth thick line, following the real path order
    if (orderedPath && revealCount > 0) {
        Vector2 startPt = { offsetX + start.x*cellSize + cellSize/2.0f, offsetY + start.y*cellSize + cellSize/2.0f };

        std::vector<Vector2> line;
        line.push_back(startPt);
        int n = std::min(revealCount, (int)orderedPath->size());
        for (int i = 0; i < n; i++) {
            const Point& p = (*orderedPath)[i];
            line.push_back({ offsetX + p.x*cellSize + cellSize/2.0f, offsetY + p.y*cellSize + cellSize/2.0f });
        }

        float thickness = std::max(3.0f, cellSize * 0.28f);
        for (size_t i = 0; i + 1 < line.size(); i++)
            DrawLineEx(line[i], line[i+1], thickness, colPath);
        for (auto& p : line) DrawCircleV(p, thickness/2.0f, colPath);
    }
}

int main() {
    int screenW = std::max(SIZE * CELL + MARGIN * 2, 1000); // wide enough for the full toolbar
    int screenH = SIZE * CELL + MARGIN * 2 + TOPBAR;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenW, screenH, "MazeSolver — Maze Navigator");
    SetTargetFPS(60);

    appFont = LoadFontEx("C:/Windows/Fonts/segoeui.ttf", 64, NULL, 0);
    customFontLoaded = (appFont.texture.id != 0 && appFont.glyphCount > 0);
    if (!customFontLoaded) appFont = GetFontDefault();
    SetTextureFilter(appFont.texture, TEXTURE_FILTER_BILINEAR);

    generateMaze();
    applyTheme(true); // permanent dark theme

    bool isFullscreen = false;
    AppMode mode = MODE_NORMAL;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_F)) {
            isFullscreen = !isFullscreen;
            if (isFullscreen) {
                int monitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
                ToggleFullscreen();
            } else {
                ToggleFullscreen();
                SetWindowSize(screenW, screenH);
            }
        }

        int winW = GetScreenWidth();
        int winH = GetScreenHeight();

        // ---- top bar buttons ----
        float bx = MARGIN, by = 100, bh = 52, gap = 12;
        Button btnNew    {{bx, by, 130, bh}, "New Maze"}; bx += 130+gap;
        Button btnClear  {{bx, by, 105, bh}, "Clear"};    bx += 105+gap;
        Button btnBFS    {{bx, by, 85, bh}, "BFS", mode==MODE_NORMAL && solver.algo=="bfs" && solver.status!=IDLE}; bx += 85+gap;
        Button btnDFS    {{bx, by, 85, bh}, "DFS", mode==MODE_NORMAL && solver.algo=="dfs" && solver.status!=IDLE}; bx += 85+gap;
        Button btnAstar  {{bx, by, 75, bh}, "A*",  mode==MODE_NORMAL && solver.algo=="astar" && solver.status!=IDLE}; bx += 75+gap;
        Button btnRace   {{bx, by, 95, bh}, "Race", mode==MODE_RACE}; bx += 95+gap;
        Button btnFull   {{bx, by, 150, bh}, isFullscreen ? "Exit Fullscreen" : "Fullscreen"};

        // ---- second row: maze generation + playback controls ----
        float by2 = by + bh + gap;
        float bx2 = MARGIN;
        Button btnMazeType {{bx2, by2, 150, bh}, mazeType==PERFECT ? "Maze: Perfect" : "Maze: Random", mazeType==RANDOM}; bx2 += 150+gap;
        Button btnDifficulty {{bx2, by2, 150, bh}, std::string("Difficulty: ") + difficultyNames[difficultyIdx]}; bx2 += 150+gap;
        Button btnPause  {{bx2, by2, 95, bh}, paused ? "Resume" : "Pause", paused}; bx2 += 95+gap;
        Button btnStep   {{bx2, by2, 75, bh}, "Step"}; bx2 += 75+gap;
        Button btnSlower {{bx2, by2, 50, bh}, "Speed -"}; bx2 += 50+gap;
        Button btnFaster {{bx2, by2, 50, bh}, "Speed +"}; bx2 += 50+gap;
        Button btnSize   {{bx2, by2, 110, bh}, "Size: " + std::to_string(mazeSizes[sizeIdx])};

        bool busyNormal = (mode==MODE_NORMAL && solver.status==RUNNING);
        bool busyRace = (mode==MODE_RACE && (raceSolvers[0].status==RUNNING || raceSolvers[1].status==RUNNING || raceSolvers[2].status==RUNNING));

        // ---- grid offset (normal mode) ----
        // dynamically size the maze to fill whatever space is available —
        // this is what makes fullscreen/large windows not look empty
        int availW = winW - MARGIN * 2 - SIDEBAR_W - MARGIN;
        int availH = winH - TOPBAR - MARGIN * 2;
        int dynCell = std::max(14, std::min(availW / SIZE, availH / SIZE));
        int gridPxW = SIZE * dynCell;
        int gridPxH = SIZE * dynCell;
        int offsetX = MARGIN + std::max(0, (availW - gridPxW) / 2);
        int offsetY = TOPBAR + MARGIN + std::max(0, (availH - gridPxH) / 2);

        Vector2 mouse = GetMousePosition();
        bool mouseOverUI = mouse.y < TOPBAR;

        if (mode == MODE_NORMAL && !busyNormal && !mouseOverUI) {
            int gx = (int)((mouse.x - offsetX) / dynCell);
            int gy = (int)((mouse.y - offsetY) / dynCell);
            bool inGrid = gx>=0 && gy>=0 && gx<SIZE && gy<SIZE;
            bool isPin = inGrid && ((gx==start.x && gy==start.y) || (gx==goal.x && gy==goal.y));
            if (inGrid && !isPin) {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) grid[gy][gx] = WALL;
                if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) grid[gy][gx] = OPEN;
            }
        }

        BeginDrawing();
        ClearBackground(colBg);

        drawText("MazeSolver", MARGIN, 14, 48, colText);
        drawText(mode==MODE_RACE ? "Race mode: all 3 algorithms run on the same maze at once"
                                  : "F: fullscreen   Left-drag: wall   Right-drag: erase   Pause then Step to go cell-by-cell",
                  MARGIN, 66, 18, {170,178,183,255});

        bool clickNew=false, clickClear=false, clickBFS=false, clickDFS=false, clickAstar=false,
             clickRace=false, clickFull=false, clickMazeType=false, clickDifficulty=false,
             clickPause=false, clickStep=false, clickSlower=false, clickFaster=false,
             clickSize=false;

        bool uiBusy = busyNormal || busyRace;
        if (!uiBusy) {
            clickNew   = drawButton(btnNew);
            clickClear = drawButton(btnClear);
            clickBFS   = drawButton(btnBFS);
            clickDFS   = drawButton(btnDFS);
            clickAstar = drawButton(btnAstar);
            clickRace  = drawButton(btnRace);
            clickMazeType   = drawButton(btnMazeType);
            clickDifficulty = drawButton(btnDifficulty);
        } else {
            drawButton(btnNew); drawButton(btnClear);
            drawButton(btnBFS); drawButton(btnDFS); drawButton(btnAstar);
            drawButton(btnRace);
            drawButton(btnMazeType); drawButton(btnDifficulty);
        }
        // Pause/Step/Speed are only meaningful while a solve exists (running or revealing path)
        bool solveActive = (mode==MODE_NORMAL && (solver.status==RUNNING || solver.status==DONE_FOUND));
        if (solveActive) {
            clickPause  = drawButton(btnPause);
            if (paused) clickStep = drawButton(btnStep);
            else drawButton(btnStep);
        } else {
            drawButton(btnPause); drawButton(btnStep);
        }
        clickSlower = drawButton(btnSlower);
        clickFaster = drawButton(btnFaster);
        if (!uiBusy) clickSize = drawButton(btnSize); else drawButton(btnSize);
        clickFull = drawButton(btnFull);

        auto clearAllSolverState = [&]() {
            solver.status = IDLE;
            solver.myState.assign(SIZE, std::vector<VisState>(SIZE, NONE_));
            solver.path.clear();
            solver.pathIndex = 0;
            for (int i = 0; i < 3; i++) {
                raceSolvers[i].status = IDLE;
                raceSolvers[i].myState.assign(SIZE, std::vector<VisState>(SIZE, NONE_));
                raceSolvers[i].path.clear();
                raceSolvers[i].pathIndex = 0;
            }
        };

        if (clickNew)   { generateMaze(); mode = MODE_NORMAL; clearAllSolverState(); paused = false; }
        if (clickClear) { for (auto& row : grid) std::fill(row.begin(), row.end(), OPEN);
                           resetState();
                           clearAllSolverState(); paused = false; }
        if (clickBFS)   { mode = MODE_NORMAL; solver.start_("bfs"); paused = false; }
        if (clickDFS)   { mode = MODE_NORMAL; solver.start_("dfs"); paused = false; }
        if (clickAstar) { mode = MODE_NORMAL; solver.start_("astar"); paused = false; }
        if (clickRace)  { mode = MODE_RACE; startRace(); paused = false; }
        if (clickMazeType)   mazeType = (mazeType==PERFECT) ? RANDOM : PERFECT;
        if (clickDifficulty) difficultyIdx = (difficultyIdx + 1) % 3;
        if (clickPause) paused = !paused;
        if (clickStep && paused && mode==MODE_NORMAL) solver.step();
        if (clickSlower) stepsPerFrame = std::max(1, stepsPerFrame - 1);
        if (clickFaster) stepsPerFrame = std::min(15, stepsPerFrame + 1);
        if (clickSize) {
            sizeIdx = (sizeIdx + 1) % 3;
            resizeMaze(mazeSizes[sizeIdx]);
            generateMaze();
            mode = MODE_NORMAL;
            clearAllSolverState();
            paused = false;
        }
        if (clickFull) {
            isFullscreen = !isFullscreen;
            if (isFullscreen) {
                int monitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
                ToggleFullscreen();
            } else {
                ToggleFullscreen();
                SetWindowSize(screenW, screenH);
            }
        }

        if (mode == MODE_NORMAL) {
            if (!paused && (solver.status == RUNNING || solver.status == DONE_FOUND))
                for (int i = 0; i < stepsPerFrame; i++) solver.step();

            // ---- big, legible stat cards instead of a small plain status line ----
            {
                std::string stateLabel = "Idle";
                Color stateColor = {170,178,183,255};
                if (solver.status == RUNNING) { stateLabel = paused ? "Paused" : "Solving"; stateColor = {243,201,105,255}; }
                else if (solver.status == DONE_FOUND) { stateLabel = "Found!"; stateColor = {82,200,140,255}; }
                else if (solver.status == DONE_NOTFOUND) { stateLabel = "No path"; stateColor = {224,84,84,255}; }

                std::string stepsStr = (solver.status==DONE_FOUND) ? std::to_string((int)solver.path.size()) : "-";
                std::string exploredStr = std::to_string(solver.exploredCount);
                std::string timeStr = "-";
                if (solver.elapsedSec >= 0) { char buf[32]; snprintf(buf, sizeof(buf), "%.0f ms", solver.elapsedSec*1000.0); timeStr = buf; }

                int cardY = 220, cardH = 80, cardW = 170, cardGap = 14;
                int cx = MARGIN;
                auto card = [&](const std::string& label, const std::string& value, Color valColor) {
                    Rectangle r = {(float)cx, (float)cardY, (float)cardW, (float)cardH};
                    DrawRectangleRounded(r, 0.18f, 8, colBtn);
                    DrawRectangleRoundedLines(r, 0.18f, 8, colBtnBorder);
                    drawText(label, cx+14, cardY+10, 15, {150,158,163,255});
                    drawText(value, cx+14, cardY+34, 32, valColor);
                    cx += cardW + cardGap;
                };
                card("STATE", stateLabel, stateColor);
                card("STEPS", stepsStr, colText);
                card("EXPLORED", exploredStr, colText);
                card("TIME", timeStr, colText);
            }

            drawMaze(offsetX, offsetY, dynCell, &solver.myState, &solver.path, solver.pathIndex);
            int pinFont = std::max(14, dynCell / 2);
            for (int y=0;y<SIZE;y++) for (int x=0;x<SIZE;x++) {
                if (x==start.x && y==start.y) drawText("S", offsetX+x*dynCell+dynCell/2-pinFont/3, offsetY+y*dynCell+dynCell/2-pinFont/2, pinFont, WHITE);
                if (x==goal.x && y==goal.y)  drawText("E", offsetX+x*dynCell+dynCell/2-pinFont/3, offsetY+y*dynCell+dynCell/2-pinFont/2, pinFont, WHITE);
            }

            // ---- legend panel, fills the empty space to the right of the maze ----
            int legendX = offsetX + gridPxW + MARGIN;
            int legendY = offsetY;
            drawText("Legend", legendX, legendY, 22, colText);
            struct LegendItem { Color c; std::string label; };
            std::vector<LegendItem> items = {
                {colStart,    "Start"},
                {colEnd,      "End"},
                {colWall,     "Wall"},
                {colVisited,  "Visited"},
                {colFrontier, "Frontier (queued)"},
                {colPath,     "Shortest route"},
            };
            int ly = legendY + 44;
            for (auto& item : items) {
                DrawRectangleRounded({(float)legendX, (float)ly, 26, 26}, 0.4f, 6, item.c);
                drawText(item.label, legendX + 38, ly + 3, 18, {200,206,210,255});
                ly += 40;
            }

            ly += 20;
            drawText("Algorithm", legendX, ly, 22, colText); ly += 38;
            std::vector<std::string> descLines;
            if (solver.algo == "bfs") {
                descLines = {
                    "BFS explores in widening",
                    "layers, one step at a time.",
                    "Guarantees the shortest",
                    "path on this unweighted grid."
                };
            } else if (solver.algo == "dfs") {
                descLines = {
                    "DFS dives down one path as",
                    "deep as possible before",
                    "backtracking. Finds *a* path,",
                    "not necessarily the shortest."
                };
            } else {
                descLines = {
                    "A* is guided by a heuristic:",
                    "h(n) = Manhattan distance",
                    "to the goal. It combines",
                    "g(n) = cost so far with h(n)",
                    "into f(n) = g(n) + h(n), and",
                    "expands the most promising",
                    "cell first - usually exploring",
                    "fewer cells than BFS."
                };
            }
            for (auto& line : descLines) { drawText(line, legendX, ly, 16, {160,168,173,255}); ly += 22; }
        } else {
            // ---- RACE MODE: three mini panels side by side ----
            for (int i = 0; i < 3; i++)
                if (raceSolvers[i].status == RUNNING || raceSolvers[i].status == DONE_FOUND)
                    for (int s = 0; s < stepsPerFrame; s++) raceSolvers[i].step();

            int panelGap = 24;
            int panelW = (winW - MARGIN*2 - panelGap*2) / 3;
            int availPanelH = winH - TOPBAR - MARGIN*2 - 44; // leave room for the label above each panel
            int miniCell = std::max(8, std::min(panelW / SIZE, availPanelH / SIZE));
            int mazePxW = miniCell * SIZE;

            for (int i = 0; i < 3; i++) {
                int panelX = MARGIN + i * (panelW + panelGap);
                int labelY = TOPBAR + MARGIN - 4;
                std::string label = raceNames[i];
                Solver& s = raceSolvers[i];
                if (s.status == RUNNING) label += "  (running, explored " + std::to_string(s.exploredCount) + ")";
                else if (s.status == DONE_FOUND) {
                    label += "  FOUND  path=" + std::to_string((int)s.path.size()) +
                             " explored=" + std::to_string(s.exploredCount);
                }
                else if (s.status == DONE_NOTFOUND) label += "  no path";

                drawText(label, panelX, labelY - 24, 19,
                         s.status==DONE_FOUND ? (Color){82,200,140,255} : colText);

                int mazeOffsetX = panelX + std::max(0, (panelW - mazePxW) / 2);
                int mazeOffsetY = labelY + 8;
                drawMaze(mazeOffsetX, mazeOffsetY, miniCell, &s.myState, &s.path, s.pathIndex);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}