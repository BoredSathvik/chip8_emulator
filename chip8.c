#include "chip8.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

uint8_t chip8_fontset[80] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
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

void init_chip8(chip8 *p_chip8)
{
    srand(time(NULL));

    p_chip8->pc = 0x200;
    p_chip8->ic = 0;
    p_chip8->sp = 0;
    p_chip8->sound_timer = 0;
    p_chip8->delay_timer = 0;

    memset(p_chip8->gfx, 0, sizeof(p_chip8->gfx));
    memset(p_chip8->stack, 0, sizeof(p_chip8->stack));
    memset(p_chip8->memory, 0, sizeof(p_chip8->memory));

    for (int i = 0; i < sizeof(chip8_fontset) / sizeof(uint8_t); i++)
    {
        p_chip8->memory[0x050 + i] = chip8_fontset[i];
    }
};

void load_rom(chip8 *p_chip8, uint8_t *p_rom, int size)
{
    for (int i = 0; i < size; i++)
    {
        p_chip8->memory[i + 0x200] = p_rom[i];
    }
}
void load_rom_file(chip8 *p_chip8, char *file_name)
{
    FILE *file = fopen(file_name, "rb");
    if (file == NULL)
    {
        perror("Failed to open file");
        return;
    }

    // 2. Move the file pointer to the very end of the file
    if (fseek(file, 0, SEEK_END) != 0)
    {
        perror("fseek to end failed");
        fclose(file);
        return;
    }

    // 3. Get the current position, which equals the total file size in bytes
    long file_size = ftell(file);
    if (file_size < 0)
    {
        perror("ftell failed to get size");
        fclose(file);
        return;
    }

    // 4. Reset the file pointer back to the beginning to prepare for reading
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        perror("fseek reset failed");
        fclose(file);
        return;
    }

    // 5. Dynamically allocate memory to hold the entire file content
    uint8_t *buffer = malloc(file_size);
    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    // 6. Read the whole file into the newly allocated array
    size_t bytes_read = fread(buffer, sizeof(uint8_t), file_size, file);
    if (bytes_read < (size_t)file_size)
    {
        if (ferror(file))
        {
            perror("Error occurred while reading the file");
        }
        else if (feof(file))
        {
            printf("Reached unexpected end of file. Only read %zu bytes.\n", bytes_read);
        }
    }
    else
    {
        printf("Successfully read all %zu bytes into the array.\n", bytes_read);
    }

    // --- Work with your data here (e.g., buffer[0], buffer[1]...) ---

    for (int i = 0; i < bytes_read; i++)
    {
        p_chip8->memory[i + 0x200] = buffer[i];
    }

    // 7. Clean up memory and file pointers
    free(buffer);
    fclose(file);
    return;
};

bool decrement_timers(chip8 *p_chip8)
{
    if (p_chip8->delay_timer > 0)
    {
        p_chip8->delay_timer--;
    }
    if (p_chip8->sound_timer > 0)
    {
        p_chip8->sound_timer--;
        if (p_chip8->sound_timer == 0)
        {
            return true;
        }
    }

    return false;
};

void handle_0000(chip8 *p_chip8, uint16_t opcode)
{
    switch (opcode) // clr & ret
    {
    case 0x00E0: // 00E0 - Clear the screen  (WORKS)
        memset(p_chip8->gfx, 0, sizeof(p_chip8->gfx));

        break;
    case 0x00EE: // 00EE - Return from subroutine
        p_chip8->sp--;
        p_chip8->pc = p_chip8->stack[p_chip8->sp];
        break;

    default:
        printf("Invalid opcode 0x%04X\n", opcode);
        break;
    }
};
void handle_8000(chip8 *p_chip8, uint16_t opcode)
{

    uint8_t s = p_chip8->v[(opcode & 0x0F00) >> 8];
    uint8_t t = p_chip8->v[(opcode & 0x00F0) >> 4];

    switch (opcode & 0x000F)
    {
    case 0x0000: // 8st0 - Vs = Vt
        p_chip8->v[(opcode & 0x0F00) >> 8] = p_chip8->v[(opcode & 0x00F0) >> 4];
        break;

    case 0x0001: // 8st1 - Perform logical OR on register `s` and `t` and store in `t`
        p_chip8->v[(opcode & 0x0F00) >> 8] = s | t;
        break;

    case 0x0002: // 8st2 - Perform logical AND on register `s` and `t` and store in `t`
        p_chip8->v[(opcode & 0x0F00) >> 8] = s & t;
        break;

    case 0x0003: // 8st3 - Perform logical XOR on register `s` and `t` and store in `t`
        p_chip8->v[(opcode & 0x0F00) >> 8] = s ^ t;
        break;

    case 0x0004: // 8st4 - Add `s` to `t` and store in `s` - register F set on carry
        p_chip8->v[(opcode & 0x0F00) >> 8] = (s + t) & 0xFF;
        p_chip8->v[0xF] = (s + t) > 0xFF ? 1 : 0;
        break;

    case 0x0005: // 8st5 - Subtract `t` from `s` and store in `s` - register F set on not borrow
        p_chip8->v[(opcode & 0x0F00) >> 8] = s - t;
        p_chip8->v[0xF] = s >= t ? 1 : 0;
        break;

    case 0x0006: // 8st6 - Shift bits in register `s` 1 bit to the right - bit 0 shifts to register F
        if (p_chip8->quirk)
        {
            p_chip8->v[(opcode & 0x0F00) >> 8] = t;
        }

        p_chip8->v[(opcode & 0x0F00) >> 8] >>= 1;
        p_chip8->v[0xF] = s & 0x1;
        break;

    case 0x0007: // 8st7 - Subtract `s` from `t` and store in `s` - register F set on not borrow
        p_chip8->v[(opcode & 0x0F00) >> 8] = t - s;
        p_chip8->v[0xF] = t >= s ? 1 : 0;
        break;

    case 0x000E: // 8stE - Shift bits in register `s` 1 bit to the left - bit 7 shifts to register F
        if (p_chip8->quirk)
        {
            p_chip8->v[(opcode & 0x0F00) >> 8] = t;
        }

        p_chip8->v[(opcode & 0x0F00) >> 8] <<= 1;
        p_chip8->v[0xF] = (s & 0x80) >> 7;
        break;
    default:
        printf("Invalid Opcode 0x%04X\n", opcode);
        break;
    }
};
void handle_D000(chip8 *p_chip8, uint16_t opcode)
{
    uint8_t s = p_chip8->v[(opcode & 0x0F00) >> 8] % p_chip8->width;  // x-coord
    uint8_t t = p_chip8->v[(opcode & 0x00F0) >> 4] % p_chip8->height; // y-coord
    uint8_t n = opcode & 0x000F;                                      // size

    p_chip8->v[0xF] = 0;
    for (int y = 0; y < n; y++)
    {
        uint8_t sprite_byte = p_chip8->memory[p_chip8->ic + y];
        // printf("0x%02X\n", sprite_byte);

        for (int x = 0; x < 8; x++)
        {
            uint8_t sprite_pixel = (sprite_byte & (0b10000000 >> x)) >> (7 - x);
            if (sprite_pixel == 1)
            {
                if (s + x < p_chip8->width && t + y < p_chip8->height)
                {
                    uint8_t *screen_pixel = &p_chip8->gfx[((s + x)) + (((t + y)) * p_chip8->width)];
                    if (*screen_pixel == 1)
                    {
                        p_chip8->v[0xF] = 1;
                    }

                    (*screen_pixel) ^= sprite_pixel;
                }

                // printf("%d ", *screen_pixel);
            }
        }
        // printf("\n");
    }
};
void handle_E000(chip8 *p_chip8, uint16_t opcode)
{
    uint8_t s = p_chip8->v[(opcode & 0x0F00) >> 8];
    switch (opcode & 0x00FF)
    {
    case 0x009E: // Es9E skip next instruction if s is pressed
        if (p_chip8->key[s])
        {
            p_chip8->pc += 2;
        }
        break;
    case 0x00A1: // Es9E skip next instruction if s is not pressed
        if (!p_chip8->key[s])
        {
            p_chip8->pc += 2;
        }
        break;
    default:
        printf("Invalid Opcode 0x%04X\n", opcode);
        break;
    }
};
void handle_F000(chip8 *p_chip8, uint16_t opcode)
{
    uint8_t s = p_chip8->v[(opcode & 0x0F00) >> 8];
    uint8_t t = p_chip8->v[(opcode & 0x00F0) >> 4];

    switch (opcode & 0x00FF)
    {
    case 0x0007: // `Fs15` - Load register `s` with value delay timer;
        p_chip8->v[(opcode & 0x0F00) >> 8] = p_chip8->delay_timer;
        break;
    case 0x000A: // `Fs0A` - Wait for keypress and store in register `s` (WORKS)
        for (int i = 0; i < sizeof(p_chip8->key); i++)
        {
            if (p_chip8->key_prev[i] == 1 && p_chip8->key[i] == 0)
            // if (p_chip8->key[i] == 1)
            {
                p_chip8->v[(opcode & 0x0F00) >> 8] = i;
                p_chip8->pc += 2;
                break;
            }
        }
        p_chip8->pc -= 2;
        break;
    case 0x0015: // `Fs15` - Load delay timer with value in register `s`
        p_chip8->delay_timer = p_chip8->v[(opcode & 0x0F00) >> 8];
        break;
    case 0x0018: // `Fs18` - Load sound timer with value in register `s`
        p_chip8->sound_timer = p_chip8->v[(opcode & 0x0F00) >> 8];
        break;
    case 0x001E: // `Fs1E` - Add value in register `s` to index
        p_chip8->ic += s;
        break;
    case 0x0029: //`Fs29` - Load index with sprite from register `s` (WORKS)
        p_chip8->ic = 0x050 + (s * 5);
        // printf("index counter 0x%02X\n", (opcode & 0x0F00) >> 8);
        break;
    case 0x0033: // `Fs33` - Store the binary coded decimal value of register `s` at index
        p_chip8->memory[p_chip8->ic] = s / 100;
        p_chip8->memory[p_chip8->ic + 1] = (s % 100) / 10;
        p_chip8->memory[p_chip8->ic + 2] = s % 10;
        break;
    case 0x0055:
    { // `Fs55` - Store the values of registers V0 to Vs in memory at index then put i = i + s + 1
        uint8_t s = (opcode & 0x0F00) >> 8;
        for (int i = 0; i <= s; i++)
        {
            p_chip8->memory[p_chip8->ic + i] = p_chip8->v[i];
        }
        if (p_chip8->quirk)
        {
            p_chip8->ic += s + 1;
        }
        break;
    }
    case 0x0065: // `Fs65` - Store the values of memory at index in registers V0 to Vs then put i = i + s + 1
    {
        uint8_t s = (opcode & 0x0F00) >> 8;
        for (int i = 0; i <= s; i++)
        {
            p_chip8->v[i] = p_chip8->memory[p_chip8->ic + i];
        }
        if (p_chip8->quirk)
        {
            p_chip8->ic += s + 1;
        }
        break;
    }
    default:
        printf("Invalid Opcode 0x%04X\n", opcode);
        break;
    }
};

void simulate_cpu_cycle(chip8 *p_chip8)
{
    // Fetch
    uint16_t opcode = p_chip8->memory[p_chip8->pc] << 8 | p_chip8->memory[p_chip8->pc + 1];

    uint8_t s = p_chip8->v[(opcode & 0x0F00) >> 8];
    uint8_t t = p_chip8->v[(opcode & 0x00F0) >> 4];
    // printf("0x%03X %04X\n", p_chip8->pc, opcode);

    p_chip8->pc += 2;

    switch (opcode & 0xF000)
    {
    case 0x0000:
        handle_0000(p_chip8, opcode);
        break;
    case 0x1000: // 1nnn - Jump to address `nnn` (WORKS)
        p_chip8->pc = opcode & 0x0FFF;
        break;
    case 0x2000: // 2nnn - Call routine at address `nnn`
        p_chip8->stack[p_chip8->sp] = p_chip8->pc;
        p_chip8->sp++;
        p_chip8->pc = opcode & 0x0FFF;
        break;
    case 0x3000: // 3snn - Skip next instruction if register `s` value equals `nn` (WORKS)
        if (s == (opcode & 0x00FF))
            p_chip8->pc += 2;
        break;

    case 0x4000: // 4snn - Skip next instruction if register `s` value not equals `nn`
        if (s != (opcode & 0x00FF))
            p_chip8->pc += 2;
        break;

    case 0x5000: // 5st0- Skip if register `s` value equals register `t` value
        if (s == t)
            p_chip8->pc += 2;
        break;
    case 0x6000: // 6snn - Load register `s` with value `nn` (WORKS)
        p_chip8->v[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        // printf("Loading value 0x%02X to register 0x%X (value: 0x%02X)\n", opcode & 0x00FF, (opcode & 0x0F00) >> 8, p_chip8->v[(opcode & 0x0F00) >> 8]);
        break;
    case 0x7000: // 7snn - Add value `nn` to register `s` (WORKS)
        p_chip8->v[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        // printf("Adding 0x%02X to reg 0x%02X (value: 0x%02X)\n", opcode & 0x00FF, (opcode & 0x0F00) >> 8, p_chip8->v[(opcode & 0x0F00) >> 8]);
        break;
    case 0x8000: // operators
        handle_8000(p_chip8, opcode);
        break;
    case 0x9000: // 9st0 - Skip next instruction if register `s` not equal register `t`
        if (s != t)
            p_chip8->pc += 2;
        break;
    case 0xA000: // Annn - Load index with value `nnn` (WORKS)
        p_chip8->ic = opcode & 0x0FFF;
        break;
    case 0xB000: // Bnnn - Jump to address `nnn` + v0
        p_chip8->pc = (opcode & 0x0FFF) + p_chip8->v[0];
        break;
    case 0xC000: // Ctnn - Generate random number between 0 and `nn` and store in `t` (WORKS)
        p_chip8->v[(opcode & 0x0F00) >> 8] = (rand() % 256) & ((opcode & 0x00FF));
        break;
    case 0xD000: // Dstn - Draw `n` byte sprite at x location reg `s`, y location reg `t` (WORKS)
        handle_D000(p_chip8, opcode);
        break;
    case 0xE000:
        handle_E000(p_chip8, opcode);
        break;
    case 0xF000: // misc
        handle_F000(p_chip8, opcode);
        break;
    default:
        printf("Invalid Opcode 0x%04X\n", opcode);
        break;
    }
}