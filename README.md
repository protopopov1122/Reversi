## Reversi (Othello) game

This project is a reversi game implementation. Currently it includes AI engine (employs Negamax, Alpha-Beta pruning and node caching) and user
interface with several game modes (human-human, human-ai, ai-ai).

TODO:
* add reversi-specific evaluation heuristics to the engine
* extend UI - add game tree viewer for ai modes and display current state assesment

### Installation
Project is tested on Linux and Windows operating systems. Project uses CMake to build, code is written using C++17 standard so compatible compiler should be used.
The only runtime dependency is wxWidgets library (minimal version 3.0.4).

Building on Linux:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Motivation
Basically, this is university homework for AI course, however I've always been interested in reversi game, since it has both 
simple mechanics and interesting gameplay

### Author & License
Author: Jevgēnijs Protopopovs \
Project is licensed under the terms of the 3-clause BSD license. See LICENSE
