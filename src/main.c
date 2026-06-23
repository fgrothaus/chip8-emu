#include <stdio.h>
#include <unistd.h> // Für usleep

#include "cpu.h"

int main() {
    // run code: make run
    Chip8 chip8;


    chip8_init(&chip8);
    printf("delay_timer nach Init: %d\n", chip8.delay_timer); // chip8-> Schreibweise nur bei Pointern

    chip8_load_rom(&chip8, "roms/IBM Logo.ch8");

    printf("\n--- SPEICHER-KONTROLLE ---\n");
    printf("Byte bei RAM-Adresse 0x200: 0x%02X\n", chip8.memory[0x200]);
    printf("Byte bei RAM-Adresse 0x201: 0x%02X\n", chip8.memory[0x201]);
    

    int timer_accumulator = 0;

    while (1)
    {   
        chip8_cycle(&chip8); // Einen Opcode verarbeiten
        timer_accumulator++;


        if (timer_accumulator >= 8)
        {
            // delay_timer wird über Opcode 0xFX07 gesetzt. Bsp. Spieler soll zwei Sekunden warten => delay_timer = 120.
            // Das warten, bis der nächste Opcode über chip8_cycle ausgeführt wird, wird in der main.c dann über den delay_timer gesteuert.
            // Eine Iteration einer Loop dauert ca. 2ms => timer_accumulator = 8 * 2ms = 16ms => Ziemlich genau die Zeit pro Frequenz (T) von 60Hz (1000ms / 60 = 16,66ms)
            // Eine Frequenz beschreibt die Häufigkeit pro Sekunde. Deswegen sind T = 1/Frequenz in Hz
            // T = 1s/60Hz = 16,66ms => Wenn delay_timer = 120 ist, wird 120*16,66ms gerechnet => 1999,2ms => ~2s
            if (chip8.delay_timer > 0)
            {
                chip8.delay_timer--;
            }

            if (chip8.sound_timer > 0)
            {
                chip8.sound_timer--;
                // Beep Ton triggern über Lib
            }

            timer_accumulator = 0;
        }

        usleep(2000); // u in usleep steht für Microseconds (Wimpernschlag - zu vernachlässigen)
    }
    
    return 0;
}