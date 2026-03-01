all: main.cpp
	g++ -std=c++17 main.cpp -lraylib && ./a.out
