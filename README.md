# Two Player Cooperative Console Game

A cooperative puzzle game where two players must work together to progress through rooms and reach the final destination.

## How to Compile

### Linux/macOS
```bash
make
```

### Windows (with MinGW)
```bash
g++ -std=c++17 -o game.exe main.cpp Game.cpp Player.cpp Room.cpp Screen.cpp Point.cpp Utils.cpp
```

## How to Run
```bash
./game      # Linux/macOS
game.exe    # Windows
```

## Controls

### Player 1 ($)
- **W** - Move Up
- **X** - Move Down
- **A** - Move Left
- **D** - Move Right
- **S** - Stop
- **E** - Drop Item

### Player 2 (&)
- **I** - Move Up
- **M** - Move Down
- **J** - Move Left
- **L** - Move Right
- **K** - Stop
- **O** - Drop Item

### System
- **ESC** - Pause Game
- **H** - Return to Main Menu (from pause)

## Game Mechanics

### Objects
- **W** - Wall: Blocks movement and explosion
- **\\** - Switch (OFF): Walk into it to toggle ON
- **/** - Switch (ON): Activated switch
- **Z** - Switch Wall: Blocks movement until condition is met (switches or explosion)
- **K** - Key: Pick up by walking into it (REQUIRED for doors)
- **0-9** - Doors: BOTH players need keys and must stand on the door together
- **@** - Bomb: Pick up and drop to explode (5 second fuse, radius 5)

### Bomb Mechanics
- Pick up a bomb (@) by walking into it
- Press DROP (E or O) to place the bomb
- Bomb explodes after ~5 seconds with radius of 5 blocks
- Explosion destroys everything EXCEPT walls and doors
- Objects behind walls/doors are protected
- **GAME OVER if**: Bomb destroys a key OR hits a player

### Room Transitions
- **BOTH players must**:
  1. Each have a key in their inventory
  2. Stand on the same door at the same time
- Keys are consumed when passing through a door

## Room Puzzles

### Room 0 (Starting Room)
- Find and activate 2 switches (\\)
- Switch Walls (Z) blocking the key will be removed
- Each player picks up a key (K)
- Both players go to door 0 together

### Room 1 (Bomb Puzzle)
- The key is trapped behind obstacles
- Pick up the bomb (@) and use it to blast through the obstacles
- Be careful not to destroy the key!
- Both players pick up keys and exit through door 1

### Room 2 (Final Room)
- Congratulations! You've won!

## Project Structure

```
game/
├── main.cpp          # Entry point
├── Game.h/cpp        # Main game logic, state machine, bomb handling
├── Player.h/cpp      # Player movement, inventory, door detection
├── Room.h/cpp        # Room management, puzzles, explosion logic
├── Screen.h/cpp      # Screen rendering
├── Point.h/cpp       # Position and movement
├── Object.h          # Game objects (switches, keys, bombs, etc.)
├── Layouts.h         # Room layouts and menu screens
├── Constants.h       # Enums and constants
├── Console.h         # Cross-platform console functions
├── Utils.h/cpp       # Utility functions
├── Makefile          # Build configuration
└── README.md         # This file
```

## Technical Notes

- No inheritance or polymorphism used
- Cross-platform support (Windows and Unix/Linux/macOS)
- Screen size: 80x25 characters
- Game area: 80x21 (bottom 4 rows for inventory display)
- Bomb uses line-of-sight calculation (Bresenham's algorithm)
- Both players must cooperate to progress through rooms
