#include <stdint.h>

#define MEMORY_SIZE 4096
#define REGISTER_COUNT 16
#define STACK_SIZE 16

typedef struct
{
    uint8_t memory[MEMORY_SIZE];    // 4KB RAM
    uint8_t V[REGISTER_COUNT];      // Die 16 Register V0-VF
    uint16_t I;                     // Index-Register (für Speicheradressen)
    uint16_t pc;                    // Program Counter (wo sind wir im Code?)

    uint16_t stack[STACK_SIZE];     // Der Stack für Unterprogramme
    uint8_t sp;                     // Stack Pointer (zeigt auf das aktuelle Stack-Level)

    uint8_t  delay_timer;           // Zählt mit 60Hz runter (für Timing)
    uint8_t  sound_timer;           // Zählt mit 60Hz runter (beim Wert > 0 piept's)

    uint8_t  display[64 * 32];      // Der Bildschirm-Buffer (Pixel an/aus)
    uint8_t  keypad[16];            // Status der 16 Tasten
}Chip8;


void chip8_init(Chip8 *chip8);