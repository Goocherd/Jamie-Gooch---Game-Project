# Jamie Gooch - Space Invaders
## Overview
This project is a 2D arcade game developed in C using the Raylib library. The game is a clone of Space Invaders where you control a spaceship and shoot at oncoming enemies, gaining points as you do, while avoiding enemy bullets. This project includes features such as animated sprites, enemy AI for both movement and shooting, and gameplay based around collision.
## Controls
- A / Left Arrow → Move left
- D / Right Arrow → Move right
- SPACE → Shoot
- ESC → Exit
## How to Compile
### Windows
gcc main.c -o game.exe -lraylib -lopengl32 -lgdi32 -lwinmm
### MacOS / Linux
gcc main.c -o game -lraylib -lm -ldl -lpthread -lGL -lrt -lX11
## How to Run
### Windows 
game.exe
### MacOS / Linux
./game
## Notes
- Raylib must be installed properly before compiling
- Game was developed in Notepad++ and compiled using gcc
- Ensure textures folder is in the correct place, if not game will still run basic shapes
- Make sure to download textures
- If having issues with video, download the raw file
