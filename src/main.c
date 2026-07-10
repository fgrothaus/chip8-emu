#include <stdio.h>
#ifdef _WIN32
    #include <windows.h> // Für Sleep() auf Windows
#else
    #include <unistd.h>    // Für usleep() auf Linux
#endif
#include <SDL2/SDL.h> // Für Grafiken

#include "cpu.h"

#define SCREEN_SCALE 10
#define CH8_WIDTH 64
#define CH8_HEIGHT 32

void debug_render_terminal(Chip8* chip8) {
    // Terminal aufräumen/nach oben flashen (ANSI Escape Code)
    printf("\033[H"); 

    printf("\n+----------------------------------------------------------------+\n");
    for (int y = 0; y < CH8_HEIGHT; y++) {
        printf("|");
        for (int x = 0; x < CH8_WIDTH; x++) {
            if (chip8->display[x + (y * 64)] == 1) {
                printf("#"); // Leuchtender Pixel
            } else {
                printf(" "); // Schwarzer Pixel
            }
        }
        printf("|\n");
    }
    printf("+----------------------------------------------------------------+\n");
}

int main(int argc, char* argv[])
{
    // run code: make run
    Chip8 chip8;

    chip8_init(&chip8);
    chip8_load_rom(&chip8, "roms/Pong.ch8");

    printf("\n--- SPEICHER-KONTROLLE ---\n");
    printf("Byte bei RAM-Adresse 0x200: 0x%02X\n", chip8.memory[0x200]);
    printf("Byte bei RAM-Adresse 0x201: 0x%02X\n", chip8.memory[0x201]);
    
    // --- SDL2 INITIALISIERUNG ---
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL Fehler: %s\n", SDL_GetError());
        return -1;
    }

    // Fenster erstellen
    SDL_Window* window = SDL_CreateWindow(
        "Mein genialer CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        CH8_WIDTH * SCREEN_SCALE, CH8_HEIGHT * SCREEN_SCALE,
        SDL_WINDOW_SHOWN
    );

    // Renderer (Pinsel) erstellen
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);


    int timer_accumulator = 0;
    int running = 1;
    SDL_Event event;

    while (running)
    {
        // 1. SDL2 Events abfragen (z.B. Fenster schließen)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_1: chip8.keypad[0x1] = 1; break; // Links Hoch
                    case SDLK_q: chip8.keypad[0x4] = 1; break; // Links Runter
                    
                    case SDLK_2: chip8.keypad[0x2] = 1; break; // Rechts Hoch (Variante 1)
                    case SDLK_w: chip8.keypad[0x5] = 1; break; // Rechts Runter (Variante 1)
                    
                    case SDLK_4: chip8.keypad[0xC] = 1; break; // Rechts Hoch (Variante 2)
                    case SDLK_r: chip8.keypad[0xD] = 1; break; // Rechts Runter (Variante 2)
                }
            }
            else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_1: chip8.keypad[0x1] = 0; break;
                    case SDLK_q: chip8.keypad[0x4] = 0; break;
                    
                    case SDLK_2: chip8.keypad[0x2] = 0; break;
                    case SDLK_w: chip8.keypad[0x5] = 0; break;
                    
                    case SDLK_4: chip8.keypad[0xC] = 0; break;
                    case SDLK_r: chip8.keypad[0xD] = 0; break;
                }
            }
        }

        chip8_cycle(&chip8); // Einen Opcode verarbeiten
        timer_accumulator++;


        if (timer_accumulator >= 8)
        {
            // delay_timer wird über Opcode 0xFX07 gesetzt. Bsp. Spieler soll zwei Sekunden warten => delay_timer = 120.
            // Das warten, bis der nächste Opcode über chip8_cycle ausgeführt wird, wird in der main.c dann über den delay_timer gesteuert.
            // Eine Iteration einer Loop dauert ca. 2ms => timer_accumulator = 8 * 2ms = 16ms => Ziemlich genau die Zeit pro Frequenz (T) von 60Hz (1000ms / 60 = 16,66ms)
            // Eine Frequenz beschreibt die Häufigkeit pro Sekunde. Deswegen sind T = 1/Frequenz in Hz
            // T = 1s/60Hz = 16,66ms => Wenn delay_timer = 120 ist, wird 120*16,66ms gerechnet => 1999,2ms => ~2s
            if (chip8.delay_timer > 0) chip8.delay_timer--;
            if (chip8.sound_timer > 0) chip8.sound_timer--; // Beep Ton triggern über Lib
            
            //debug_render_terminal(&chip8);

            timer_accumulator = 0;
        }

        // 2. GRAFIK-RENDERING: Wir zeichnen den aktuellen Zustand des CHIP-8 Displays
        // Hintergrund komplett schwarz färben
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Wir setzen die Zeichenfarbe für leuchtende Pixel auf Weiß
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // Wir scannen das flache display-Array (64 * 32 = 2048 Pixel)
        for (int y = 0; y < CH8_HEIGHT; y++) {
            for (int x = 0; x < CH8_WIDTH; x++) {
                // Wir nutzen wieder unsere gewohnte Formel fürs flache Array!
                if (chip8.display[x + (y * 64)] == 1) {
                    // Ein Quadrat (Rechteck) für den skalierten Pixel definieren
                    SDL_Rect pixel_rect;
                    pixel_rect.x = x * SCREEN_SCALE;
                    pixel_rect.y = y * SCREEN_SCALE;
                    pixel_rect.w = SCREEN_SCALE;
                    pixel_rect.h = SCREEN_SCALE;

                    // Das Quadrat weiß ausfüllen
                    SDL_RenderFillRect(renderer, &pixel_rect);
                }
            }
        }

        // Den virtuellen Zeichenpuffer auf den echten Monitor bringen
        SDL_RenderPresent(renderer);

        #ifdef _WIN32
            Sleep(2); // Windows nutzt Millisekunden (2 ms)
        #else
            usleep(2000); // u in usleep steht für Microseconds (Wimpernschlag - zu vernachlässigen)
        #endif
    }
    
    printf("SDL2 erfolgreich gestartet!\n");

    // Aufräumen, wenn das Programm beendet wird
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}