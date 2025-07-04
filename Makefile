

#g++ -o hello.exe hello.cpp -ISDL3/x86_64-w64-mingw32/include/ -LSDL3/x86_64-w64-mingw32/lib/ -lSDL3
CC=x86_64-w64-mingw32-g++
CFLAGS = -lSDL2 -lSDL2main -lSDL2_image -mwindows -ISDL2/x86_64-w64-mingw32/include/ -LSDL2/x86_64-w64-mingw32/lib/
main: main.cpp
	$(CC) $< $(CFLAGS) -o main.exe
run:
	wine main.exe
clean:
	rm -rf main.exe
