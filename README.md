# Conway's game of life

### Description
Rules of the Game of Life
 - A live cell dies if it has fewer than two live neighbors.
 - A live cell with two or three live neighbors lives on to the next generation.
 - A live cell with more than three live neighbors dies.
 - A dead cell will be brought back to live if it has exactly three live neighbors.

### Controls
Press `Enter` key for pause/unpause simulation. Empty field is always paused. Click mouse to toggle dead/alive cell. Press `Escape` to exit.

### Install:
```sh
git clone https://github.com/pppppptttttt/conways-game-of-life.git
cd conways-game-of-life
cmake -DCMAKE_BUILD_TYPE=Release .
make
```

### Launch:
```sh
./gol
```

You can also provide file with default pattern. '`.`' means dead cell, any other symbol - living cell. For example:
```
./gol pattern.txt
```
