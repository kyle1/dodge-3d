all: game

game: game.cpp
	g++ game.cpp -Wall -lX11 -lGL -lGLU -lm ./libggfonts.a -o game

clean:
	rm -f game
	rm -f *.o

