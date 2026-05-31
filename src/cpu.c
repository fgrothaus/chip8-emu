#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> // Für String Funktionen, Datentyp FILE, etc.

#include "cpu.h"

// ##### HEXDUMP #####  hexdump muss mit -C ausgeführt werden, da Linux die Big Endian Datei sonst als Little Endian ausgibt
//                      -C sorgt dafür, dass das System die ROM strikt Stück für Stück durchgeht, wie Sie in der SSD steht.
//                        Das ist dann also Big Endian, bei .ch8 Dateien

// 00000000 und 00000010 etc. sind ein Offset für die Ausgabe von hexdump.
// 00000000: Hier fängt die Datei an (Byte 0).
// 00000010: Hier liegt das 17. Byte der Datei (Dezimal 16).
// 00000020: Hier liegt das 33. Byte der Datei (Dezimal 32).
// 00000080: Hier liegt das 129. Byte der Datei (Dezimal 128).
// hexdump -C "IBM Logo.ch8"
// 00000000  00 e0 a2 2a 60 0c 61 08  d0 1f 70 09 a2 39 d0 1f  |...*`.a...p..9..|
// 00000010  a2 48 70 08 d0 1f 70 04  a2 57 d0 1f 70 08 a2 66  |.Hp...p..W..p..f|
// 00000020  d0 1f 70 08 a2 75 d0 1f  12 28 ff 00 ff 00 3c 00  |..p..u...(....<.|
// 00000030  3c 00 3c 00 3c 00 ff 00  ff ff 00 ff 00 38 00 3f  |<.<.<........8.?|
// 00000040  00 3f 00 38 00 ff 00 ff  80 00 e0 00 e0 00 80 00  |.?.8............|
// 00000050  80 00 e0 00 e0 00 80 f8  00 fc 00 3e 00 3f 00 3b  |...........>.?.;|
// 00000060  00 39 00 f8 00 f8 03 00  07 00 0f 00 bf 00 fb 00  |.9..............|
// 00000070  f3 00 e3 00 43 e0 00 e0  00 80 00 80 00 80 00 80  |....C...........|
// 00000080  00 e0 00 e0                                       |....|
// 00000084


// Jedes Zeichen besteht aus fünf Byte. Beispiel für 0:
// F0 = 11110000 => 1111
// 90 = 10010000 => 1  1
// 90 = 10010000 => 1  1
// 90 = 10010000 => 1  1
// F0 = 11110000 => 1111
// Die 1en formen die Zeichen (in dem Fall die 0)
// Es würde für das Formen der Zeichen auch ein Nibble reichen, aber die kleinste Einheit auf einem Computer ist ein Byte, deswegen wird das hintere Nibble nicht genutzt.

uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x05, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_init(Chip8* chip8) {
    for (int i = 0; i < MEMORY_SIZE; i++) chip8->memory[i] = 0;
    for (int i = 0; i < REGISTER_COUNT; i++) chip8->V[i] = 0;
    chip8->I = 0;
    chip8->pc = 0x200; // Programme starten beim CHIP-8 immer bei Adresse 0x200 (512 Dezimal) - Adressen von 0x000 - 0x1FF waren für den Interpreter (das OS) selbst reserviert.

    for (int i = 0; i < STACK_SIZE; i++) chip8->stack[i] = 0;
    chip8->sp = 0;

    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    for (int i = 0; i < 64*32; i++) chip8->display[i] = 0;
    for (int i = 0; i < 16; i++) chip8->keypad[i] = 0;

    // Die ersten 0x200 Speicheradressen sind dem BIOS vorbehalten, deswegen werden die Fonts (koventionell) ab Adresse 0x50 hinzugefügt
    for (int i = 0; i < 80; i++) {
        chip8->memory[0x50 + i] = fontset[i];
    }

    printf("CHIP-8 bereit. RAM geladen, PC steht auf 0x200.\n");
}

int chip8_load_rom(Chip8* chip8, const char* filename) {
    // const char *filename übergibt den Pointer auf char 1 von "roms/IBM Logo.ch8". Das OS weiß dann, wo auf der SSD es gucken muss.
    FILE* file = fopen(filename, "rb"); // Es wird ein Pointer auf einen FILE-Stream (Notizbuch) returned => fopen gibt FILE-Pointer zurück.
    // Das FILE-Notizbuch ist ein Struct und wird vom OS verwaltet. Da stehen Metadaten drin, wie "Wo befindet sich der Lesecursor", Pointer auf internen Buffer des OS, etc.
    // Es wurde mit fopen lediglich eine Verbindung hergestellt. Es fließen noch KEINE Bytes!
    if (file == NULL) {
        printf("Fehler: Die ROM-Datei '%s' konnte nicht gefunden werden!\n", filename);
        return -1;
    }

    // Größe der Datei herausfinden und prüfen, ob Sie überhaupt in den RAM passt
    fseek(file, 0, SEEK_END); // Ans Ende der Datei springen
    long rom_size = ftell(file); // 132 Byte - ftell sagt, an welchem Byte der Lesecursor der Datei aktuell steht
    rewind(file); // An den Anfang der Datei springen
    // printf("Die Größe der ROM: %ld Bytes\n", rom_size);

    if (rom_size > (MEMORY_SIZE - 0x200)) { // wenn 132 Bytes > (4096 - 512)Bytes
        printf("Fehler: ROM ist zu groß für den CHIP-8 Speicher!\n");
        fclose(file); // Wichtig: Datei wieder schließen!
        return -1;
    }

    // Bytes in den RAM schaufeln - size_t als Zähler, der soweit zählen kann, wie der RAM groß ist.
    size_t bytes_read = fread(&chip8->memory[0x200], 1, rom_size, file); // fread(Ziel-Buffer, Größe eines Elements, Anzahl Elemente, Datei-Pointer)

    if (bytes_read != rom_size) {
        printf("Fehler beim Einlesen der ROM-Daten!\n");
        fclose(file);
        return -1;
    }

    fclose(file); // Filestream wieder schließen. Datei ist nun im RAM.
    printf("ROM '%s' erfolgreich geladen! (%zu Bytes in den RAM kopiert)\n", filename, bytes_read); // %zu z = size type, u = unsigned
    
    return 0;
}

void chip8_cycle(Chip8* chip8) {

    // FETCH - Ein Opcode besteht aus 2 Byte/16 Bit. Deswegen müssen die 8 Bit Paare aus Memory zusammengeklebt werden.
    // Der erste Befehl ist somit 0x00e0
    // Die Werte werden jeweils in Zweierpärchen über ein logisches Oder verknüpft, indem der erste Teil um 8 Bit nach links geshiftet wird.
    // Dadurch entsteht beispielsweise so eine Struktur:
    // 01010101 00000000 | 00000000 11001100 => 01010101 11001100 - Werte wurden quasi zusammengeklebt.
    uint16_t opcode = (chip8->memory[chip8->pc] << 8) | (chip8->memory[chip8->pc + 1]); // Hier wird der Opcode aus dem RAM geladen.
    chip8->pc += 2;

    // DECODE & EXECUTE
    // Die ersten 4 Bits (Nibble) dienen zum Gruppieren des Opcodes in unterschiedliche Kategorien 0-F
    // opcode & 0xF000 für: 01011111 11000100 & 11110000 00000000 => 01010000 00000000
    switch (opcode & 0xF000) {

        // Hier werden dann die pozenziellen Cases von 0-F gruppiert.
        // Der CHIP-8 hat insgesamt 35 Opcodes

        case 0x0000:
            // 00E0 Bildschirm löschen
            // 00EE Unterprogramm-Rücksprung (return) von ausgeführtem Opcode
            switch (opcode) {
                case 0x00E0:
                    for (int i = 0; i < 64*32; i++) chip8->display[i] = 0;
                    printf("  [EXECUTE] Bildschirm gelöscht.\n");
                    break;
                case 0x00EE:
                    if (chip8->sp > 0)
                    {
                        chip8->sp--;
                        chip8->pc = chip8->stack[chip8->sp]; // Hier steht die vorherige Adresse drin, damit der pc weitermachen kann.
                        printf("  [EXECUTE] Rücksprung aus Unterprogramm zu Adresse: 0x%03X\n", chip8->pc);
                    } else {
                        printf("  [FEHLER] Stack Underflow! Kein Unterprogramm aktiv.\n");
                    }
                    
                    break;
                default:
                    printf("  [FEHLER] Unbekannter 0x0000 Opcode: 0x%04X\n", opcode);
                    break;
            }

            break;
        case 0x1000:
            // 1NNN Springe zur Adresse NNN (Jump)
            // Die {}-Brackets sind notwendig, um den Geltungsbereich der Variablen nur innerhalb des cases zu halten,
            // da die Adresse nnn häufiger benötigt wird
            {
                uint16_t nnn = opcode & 0x0FFF; // Eine Adresse ist 3 Nibble groß. Es gibt also 2^12 = 4096 Adressen (siehe MEMORY_SIZE)
                chip8->pc = nnn;
            }
            break;
        case 0x2000:
            // 2NNN Rufe Unterprogramm auf Adresse NNN auf (Call) - 0x2NNN = Gegenstück zu 0x00EE
            {
                uint16_t nnn = opcode & 0x0FFF;
                if (chip8->sp < STACK_SIZE)
                {
                    chip8->stack[chip8->sp] = chip8->pc;
                    chip8->sp++;
                    chip8->pc = nnn;
                    printf("  [EXECUTE] Call Unterprogramm bei: 0x%03X (Rücksprung zu: 0x%03X)\n", nnn, chip8->stack[chip8->sp - 1]);
                } else {
                    printf("  [FEHLER] Stack Overflow! Zu viele Unterprogramme verschachtelt.\n");
                }
                
            }
            break;
        case 0x3000:
            // 3XNN Überspringe nächsten Befehl, wenn Vx == NN. Quasi das if (ohne else) Konstrukt in Assembler/Opcodes bei Gleichheit
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                
                if (chip8->V[x] == nn)
                {
                    chip8->pc += 2;
                }
            }
            break;
        case 0x4000:
            // 4XNN Überspringe nächsten Befehl, wenn Vx != NN. Quasi das if (ohne else) Konstrukt in Assembler/Opcodes bei Gleichheit
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                
                if (chip8->V[x] != nn)
                {
                    chip8->pc += 2;
                }
            }
            break;
        case 0x5000:
            // 5XY0 Überspringe nächsten Befehl, wenn Vx == Vy
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t y = (opcode & 0x00F0) >> 4;

                if (chip8->V[x] == chip8->V[y])
                {
                    chip8->pc += 2;
                }
            }            
            break;
        case 0x6000:
            // 6XNN Setze Register Vx (Index der CPU) auf den Wert NN
            {
                uint8_t x = (opcode & 0x0F00) >> 8; // 1010 0101 1111 0111 => 1. 0000 0101 0000 0000 2. 0000 0000 0000 0101
                uint8_t nn = opcode & 0x00FF;
                chip8->V[x] = nn; // V entspricht den CPU Registern. Der Opcode wurde aus dem RAM in das entsprechende CPU-Register geladen
            }
            break;
        case 0x7000:
            // 7XNN Addiere den Wert NN zum Register Vx
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                chip8->V[x] += nn;
            }
            break;
        case 0x8000:
            // 9 Befehle: 8XY0-8XYE Mathematische Operationen. Hier teilen sich 9 Befehle das 8er-Register! Du prüfst sie am Ende mit opcode & 0x000F
            break;
        case 0x9000:
            // 9XY0 Überspringe nächsten Befehl, wenn Vx != Vy
            break;
        case 0xA000:
            // ANNN Setze Index-Register I auf Adresse NNN
            // HIER
            break;
        case 0xB000:
            // BNNN Springe zur Adresse NNN + V0
            break;
        case 0xC000:
            // CXNN Setze Vx auf eine Zufallszahl, die mit NN per UND verknüpft wird
            break;
        case 0xD000:
            // DXYN Der wichtigste Befehl! Zeichne ein Sprite auf den Bildschirm
            break;
        case 0xE000:
            // EX9E, EXA1 Tastaturabfragen, je nachdem, ob eine Taste gedrückt ist
            break;
        case 0xF000:
            // 9 Befehle: Verschiedene Timer-, Tastatur- und Speicheroperationen (z. B. FX07, FX55). Werden über die letzten zwei Stellen (opcode & 0x00FF) unterschieden
            break;
    }
}