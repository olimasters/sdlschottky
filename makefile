standard: sdlschottky.cpp
	g++ -o standard sdlschottky.cpp -lSDL2 -std=c++14
optimised: sdlschottky.cpp
	g++ -o optimised sdlschottky.cpp -lSDL2 -std=c++14 -O3
profiled: sdlschottky.cpp
	g++ -o profiled sdlschottky.cpp -lSDL2 -std=c++14 -pg
