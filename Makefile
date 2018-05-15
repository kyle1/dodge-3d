all: dodge3d

dodge3d: dodge3d.cpp
	g++ dodge3d.cpp -Wall -lX11 -lGL -lGLU -lm ppm.cpp /usr/lib/x86_64-linux-gnu/libopenal.so /usr/lib/x86_64-linux-gnu/libalut.so ./libggfonts.a -o dodge3d

clean:
	rm -f dodge3d
	rm -f *.o