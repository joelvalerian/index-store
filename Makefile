index-store: main.cpp
	g++ -std=c++17 -o index-store main.cpp index.cpp utils.cpp

clean:
	rm -rf data index.deque index.map index-store