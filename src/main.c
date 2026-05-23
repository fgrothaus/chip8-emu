#include <stdio.h>

#include "cpu.h"

int main() {
    // run code: make run
    Chip8 chip8;


    chip8_init(&chip8);
    printf("%d\n", chip8.delay_timer); // chip8-> Schreibweise nur bei Pointern

    return 0;
}