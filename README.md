# MazeSolver — Visual Maze Generation & Pathfinding

An interactive desktop application built with **C++ and raylib** that generates mazes and visualizes how different pathfinding algorithms explore and solve them. MazeSolver provides a visual comparison of Breadth-First Search (BFS), Depth-First Search (DFS), and A* through animated execution and a simultaneous Race Mode.

## Overview

Pathfinding algorithms are fundamental to computer science and are widely used in robotics, game development, navigation systems, and artificial intelligence.

MazeSolver makes these algorithms easier to understand by presenting their execution on a dynamically generated maze. Instead of relying on static diagrams, users can observe how each algorithm explores the grid, maintains its frontier, and discovers a route from the starting point to the destination.

The application also provides performance statistics and interactive controls to experiment with different maze configurations.

## Features

### Maze Generation
- **Perfect Maze Generation:** Uses Recursive Backtracking to generate a maze with connected passages and no cycles.
- **Random Maze Generation:** Creates randomized wall layouts using configurable wall density.
- **Solvability Checking:** Checks generated random mazes using BFS and retries generation up to 40 times.
- **Adjustable Difficulty:** Three random-maze wall-density settings: Easy (20%), Medium (30%), and Hard (40%).
- **Adjustable Grid Size:** Supports 15×15, 21×21, and 31×31 mazes.

### Pathfinding Algorithms

| Algorithm | Approach | Characteristics |
|---|---|---|
| BFS | Queue-based exploration | Finds a shortest path on an unweighted grid |
| DFS | Stack-based exploration | Explores deeply before backtracking |
| A* | Priority-queue-based search | Uses a heuristic to guide exploration |

### Race Mode

Race Mode runs BFS, DFS, and A* simultaneously on the same maze.

Each algorithm maintains its own exploration state, allowing users to compare their behavior independently.

The interface displays:
- Individual maze visualizations.
- Number of explored cells.
- Final path length.
- Simultaneous execution of all three algorithms.

### Visualization & Controls

- Animated cell-by-cell exploration.
- Separate visual states for visited cells and frontier cells.
- Animated final route drawing.
- Pause and resume functionality.
- Step-by-step execution in normal mode.
- Adjustable animation speed.
- Interactive wall placement and removal.
- Fullscreen support.
- Dark-themed interface.

### Performance Statistics

In normal solving mode, the application displays:

- **State:** Current algorithm execution status.
- **Steps:** Final path length.
- **Explored:** Number of explored cells.
- **Time:** Execution time in milliseconds.

## Algorithms Explained

### 1. Breadth-First Search (BFS)

BFS explores the maze level by level, starting from the initial position.

It uses a **queue (FIFO)** to determine which cell to explore next.

Working process:
1. Insert the starting cell into the queue.
2. Remove the front cell.
3. Examine its valid neighboring cells.
4. Add unvisited neighbors to the queue.
5. Record their parent cells.
6. Continue until the destination is reached or the queue becomes empty.
7. Reconstruct the path using the recorded parent cells.

**Key characteristic:** BFS guarantees a shortest path when every movement has equal cost.

### 2. Depth-First Search (DFS)

DFS explores one direction as deeply as possible before backtracking.

It uses a **deque as a stack (LIFO)**.

Working process:
1. Insert the starting cell.
2. Remove the most recently added cell.
3. Explore its unvisited neighboring cells.
4. Add those neighbors to the stack.
5. Continue exploring deeper until the destination is found or no cells remain.

**Key characteristic:** DFS can find a valid path but does not guarantee the shortest path.

### 3. A* Search

A* uses both the distance already travelled and an estimate of the remaining distance to guide its search.

It calculates:

- **g(n):** Actual cost from the starting cell to the current cell.
- **h(n):** Estimated distance from the current cell to the destination.
- **f(n):** Combined estimated cost.

The evaluation function is:

`f(n) = g(n) + h(n)`

MazeSolver uses Manhattan distance as its heuristic:

`h(n) = |x1 - x2| + |y1 - y2|`

Working process:
1. Insert the starting cell into a priority queue.
2. Select the cell with the lowest estimated total cost.
3. Examine its neighboring cells.
4. Update their costs and parent information when a better route is found.
5. Continue until the destination is reached.
6. Reconstruct the resulting path.

**Key characteristic:** A* uses heuristic guidance to prioritize promising cells and can find an optimal path with an admissible heuristic.

## Technology Stack

| Technology | Purpose |
|---|---|
| C++ | Core application logic and algorithms |
| C++17 | Language standard |
| raylib | Graphics, rendering, input handling, and window management |
| STL Containers | Queues, deques, vectors, and priority queues |
| Random Library | Randomized maze generation |

## Installation & Setup

### Prerequisites

- Windows operating system (for the supplied build command).
- C++ compiler supporting C++17.
- raylib graphics library.
- MinGW-w64 / GCC toolchain configured with raylib.

### Clone the Repository

```bash
git clone https://github.com/Vedant-Hegu/Maze_Solver_with_GUI.git
```

Navigate into the project directory:

```bash
cd Maze_Solver_with_GUI
```

### Compile the Application

Using GCC with raylib configured:

```bash
g++ -std=c++17 -O2 maze_gui.cpp -o maze_gui -lraylib -lopengl32 -lgdi32 -lwinmm
```

### Run the Application

```powershell
.\maze_gui.exe
```

**Note:** Ensure that raylib is installed and its compiler and linker configuration is available before building.

## Application Controls

| Control | Function |
|---|---|
| New Maze | Generate a new maze |
| Clear | Remove all walls |
| BFS | Run Breadth-First Search |
| DFS | Run Depth-First Search |
| A* | Run A* Search |
| Race | Run all three algorithms simultaneously |
| Maze: Perfect / Random | Switch maze generation method |
| Difficulty | Cycle through Easy, Medium, and Hard |
| Pause / Resume | Pause or resume normal-mode execution |
| Step | Advance one solver step while paused |
| Speed - / Speed + | Adjust animation speed |
| Size | Cycle through supported maze dimensions |
| Fullscreen | Toggle fullscreen mode |
| F | Keyboard shortcut for fullscreen |
| Left Mouse Drag | Draw walls |
| Right Mouse Drag | Erase walls |

## Visualization Legend

| Element | Meaning |
|---|---|
| Green | Starting position |
| Red | Destination |
| Grey | Wall |
| Light Blue | Visited cell |
| Yellow | Frontier cell |
| Green Path | Final discovered route |

## Project Structure

```text
Maze_Solver_with_GUI/
│
├── maze_gui.cpp    # Main application source code
└── README.md       # Project documentation
```

## Learning Outcomes

This project demonstrates practical implementation and visualization of:

- Graph traversal algorithms.
- Queue, stack, and priority queue data structures.
- Recursive maze generation.
- Heuristic-based pathfinding.
- Path reconstruction using parent tracking.
- Randomized environment generation.
- Real-time graphical rendering and user interaction.
- Algorithm comparison through execution statistics.

## Future Improvements

Potential extensions include:

- Additional algorithms such as Dijkstra's Algorithm and Greedy Best-First Search.
- Exporting maze-solving animations.
- More maze-generation techniques.
- Detailed algorithm performance comparison and reporting.
- User-defined start and destination positions.

## Author

**Vedant Hegu**

GitHub: [@Vedant-Hegu](https://github.com/Vedant-Hegu)

---

*MazeSolver is an educational project developed to explore data structures, graph algorithms, maze generation, and real-time visualization using C++ and raylib.*
