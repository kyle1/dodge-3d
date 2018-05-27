all: game

game: game.cpp
	g++ game.cpp -Wall -lX11 -lGL -lGLU -lm ppm.cpp /usr/lib/x86_64-linux-gnu/libopenal.so /usr/lib/x86_64-linux-gnu/libalut.so ./libggfonts.a -o dodge3d

clean:
	rm -f game
	rm -f *.o