# 1. Betriebssystem automatisch erkennen
ifeq ($(OS),Windows_NT)
    # --- WINDOWS SETUP --- run Programm: mingw32-make run 
    TARGET = chip8.exe
    RM = del /Q /F
    # Das Windows-Compiler-Monster mit allen System-Bibliotheken
    LIBS = -lmingw32 -lSDL2main -lSDL2 -mwindows -lwinmm -limm32 -lversion -lole32 -loleaut32 -luuid -lgdi32 -lsetupapi -lshlwapi
    CFLAGS = -Iinclude -Llib
else
    # --- LINUX SETUP ---
    TARGET = chip8
    RM = rm -f
    # Unter Linux reicht das einfache lSDL2-Flag
    LIBS = -lSDL2
    CFLAGS = 
endif

# 2. Die eigentlichen Build-Befehle (bleiben für beide Systeme gleich!)
all:
	gcc src/main.c src/cpu.c $(CFLAGS) -o $(TARGET) $(LIBS)

run: all
	./$(TARGET)

clean:
	$(RM) $(TARGET)