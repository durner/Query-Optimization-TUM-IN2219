all:		clean first

clean:
			rm -rf ./bin

first:
			@mkdir -p bin
			g++ -Wall -Werror -march=native -std=c++11 -g -O3 -Iinclude -o bin/first test/first.cpp src/first.cpp
