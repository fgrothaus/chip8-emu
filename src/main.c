#include <stdio.h>

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
    return 0;
}