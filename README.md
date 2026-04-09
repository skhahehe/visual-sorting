# Sorting Algorithm Visualizer 

A high-performance, interactive educational tool built with C++ and SFML 3.0 to visualize how different sorting algorithms operate in real-time. This project was developed as a Data Structures & Algorithms (DSA) End Term Project.

##  Features

### 5 Sorting Algorithms:

- **Bubble Sort**: Visualizes the "bubbling up" of largest elements.
- **Insertion Sort**: Visualizes building a sorted array one item at a time.
- **Selection Sort**: Visualizes finding the minimum element and placing it.
- **Quick Sort (Partition Sort)**: Visualizes the divide-and-conquer pivot strategy.
- **Merge Sort**: Visualizes the recursive splitting and merging of arrays.

### Custom Input
Users can input their own specific set of numbers to sort.

### Real-time Visualization
Watch bars swap (Red), compare (Yellow), and lock into place (Green).

### Interactive Controls:
- **Speed Control**: Adjust animation speed dynamically (Speed + / Speed -).
- **Reset**: Reverts the array to its original unsorted state.
- **New Array**: Allows entering a fresh dataset without restarting the app.
- **Data Display**: Values are displayed at the base of each bar for clarity.

##  Technologies Used

- **Language**: C++ (C++17 standard)
- **Graphics Library**: SFML 3.0.2 (Simple and Fast Multimedia Library)
- **Build System**: GNU Make (macOS) / MinGW g++ (Windows)
- **Platform**: macOS & Windows

##  macOS Setup

### Prerequisites

- **C++ Compiler**: (e.g., g++ or clang++)
- **Homebrew**: (Package manager for macOS)
- **SFML 3 & pkg-config**:

```bash
brew install sfml pkg-config
```

### Building & Running

**Build:**
```bash
make
```

**Run:**
```bash
open SortVisualizer.app
```

**Clean (Optional):**
```bash
make clean
```

##  Windows Setup

Running C++ with external libraries on Windows requires a bit more manual setup than macOS.

### Prerequisites

- **MinGW-w64 (GCC Compiler)**: Install a compiler like MinGW-w64 (ensure g++ is in your system PATH).
- **SFML 3.0.2 (GCC MinGW version)**: Download the GCC MinGW 64-bit binaries from the [SFML Website](https://www.sfml-dev.org/download/sfml/3.0.2/).

### Installation Steps

**Prepare SFML:**
- Extract the downloaded SFML folder (e.g., to `C:\SFML-3.0.2`).

**Compile:**
- Open your terminal (PowerShell or CMD) in the project folder.
- Run the following command (adjust the paths `C:\SFML-3.0.2` to where you actually extracted SFML):

```bash
g++ -I C:\SFML-3.0.2\include -L C:\SFML-3.0.2\lib main.cpp -o SortVisualizer.exe -lsfml-graphics -lsfml-window -lsfml-system
```

**Copy DLLs (Crucial Step):**
- Go to `C:\SFML-3.0.2\bin`.
- Copy all `.dll` files (e.g., `sfml-graphics-3.dll`, `openal32.dll`, etc.).
- Paste them into the same folder as your newly created `SortVisualizer.exe`.
- **Note**: Without this step, the application will crash immediately upon opening.

**Run:**
```bash
.\SortVisualizer.exe
```

##  How to Use

1. **Input Data**: Upon launching, type a sequence of space-separated numbers (e.g., `50 100 25 75 10`) and press Enter.
2. **Select Algorithm**: Click on any of the algorithm buttons (Bubble, Insertion, etc.) at the top.
3. **Watch**: The sorting process will begin immediately.
   - **Yellow Bars**: Elements currently being compared.
   - **Red Bars**: Elements being swapped or moved.
   - **Green Bars**: Default state.

### Controls:
- **Speed +/-**: Click to slow down or speed up the animation.
- **Reset**: Click to restart the sort with the same numbers.
- **New Array**: Click to go back to the input screen.

##  Algorithm Complexity Overview

| Algorithm | Time (Avg) | Space | Description |
|-----------|-----------|-------|-------------|
| Bubble Sort | $O(n^2)$ | $O(1)$ | Simple swapping of adjacent elements. |
| Insertion Sort | $O(n^2)$ | $O(1)$ | Efficient for small or nearly sorted lists. |
| Selection Sort | $O(n^2)$ | $O(1)$ | Minimizes the number of swaps. |
| Quick Sort | $O(n \log n)$ | $O(\log n)$ | Fast, divide-and-conquer strategy. |
| Merge Sort | $O(n \log n)$ | $O(n)$ | Stable sort, guarantees $O(n \log n)$ performance. |


Built with ❤️ and C++