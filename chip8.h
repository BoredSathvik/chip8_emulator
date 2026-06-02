#ifndef CHIP8_H
#define chip8_H

#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    uint8_t memory[4096];
    uint8_t gfx[64 * 32];

    uint8_t v[16];

    unsigned short ic;
    unsigned short pc;

    unsigned short stack[0x10];
    unsigned short sp;

    uint8_t delay_timer;
    uint8_t sound_timer;

    uint8_t key_prev[16]; // key layout in the previous cycle
    uint8_t key[16];

    int width;
    int height;

    bool quirk;
} chip8;

void init_chip8(chip8 *p_chip8);
void load_rom(chip8 *p_chip8, uint8_t *p_rom, int size);
void load_rom_file(chip8 *p_chip8, char *file_name);
void simulate_cpu_cycle(chip8 *p_chip8);
bool decrement_timers(chip8 *p_chip8);

#endif