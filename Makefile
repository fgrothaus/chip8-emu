all:
	gcc src/main.c src/cpu.c -o chip8 -lSDL2

run: all
	./chip8

clean:
	rm -f chip8 a.out