#include "raylib.h"
#include "chip8.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t test[] = {
    0x6E, 0x00, // LD Ve 0x00  # sprite number
    0x60, 0x00, // LD V0 0x00  # x coord
    0x61, 0x00, // LD V1 0x00  # y coord
    0xFE, 0x29, // LD F Ve     # load sprite
    0xD0, 0x15, // DRW V0 V1 5 # draw sprite

    0x70, 0x05, // ADD V0 0x05 # mov x by one
    0x71, 0x00, // ADD V1 0x00 # mov y by one
    0x7E, 0x01, // ADD Ve 0x01 # get next number
    // 0x12, 0x06, // JP 0x206
    0x00, 0xE0, // CLS

    0x00, 0x00, 0x00};

// MAZE GENARATOR
uint8_t test2[] = {
    0xa2, 0x1e, // LD I 0x21E # load sprite 2
    0xc2, 0x01, // RND V2 0x01 # rand no btw 0 & 2
    0x32, 0x01, // SE V2 0x01 # skip if rand no == 2
    0xa2, 0x1a, // LD I 0x21A # load sprite 1
    0xd0, 0x14, // DRW V0 V1 4 # draw sprite at V0 V1 and size 4 bytes
    0x70, 0x04, // ADD V0 0x04 # add 0x04 to V0
    0x30, 0x40, // SE V0 0x40 # skip if V0 == 0x40
    0x12, 0x00, // JP 200 # jump to 200
    0x60, 0x00, // LD V0 0x00
    0x71, 0x04, // ADD V1 0x04 # add 0x04 to V1
    0x31, 0x20, // SE V1 0x20 # skip if V1 == 0x20
    0x12, 0x00, // JP 200 # jump to 200
    0x12, 0x18, // JP 218 # jump to same instruction
    // Sprite 1
    0x80, // 10000000
    0x40, // 01000000
    0x20, // 00100000
    0x10, // 00010000
    // Sprite 2
    0x20,  // 00100000
    0x40,  // 01000000
    0x80,  // 10000000
    0x10}; // 00010000

int width = 64;
int height = 32;
int scaling = 10;

chip8 _chip8;

float accumulator = 0;
float tick_rate = 1.0f / 60.0f;
int cycles_per_frame = 10;
// cycles_per_frame / tick_rate is the Emulator Speed

void update_input(chip8 *p_chip8);

int main()
{
    _chip8.width = width;
    _chip8.height = height;
    _chip8.quirk = false; // if we should increment index counter in 0xFs55 and 0xFx65, MODERN ONES PUT THIS TO FALSE

    init_chip8(&_chip8);
    // load_rom(&_chip8, test3, sizeof(test3));
    load_rom_file(&_chip8, "chip8_roms/2-ibm-logo.ch8");

    InitAudioDevice();
    Sound beep = LoadSound("beep.mp3");

    InitWindow(width * scaling, height * scaling, "chip8_emulator");
    SetTargetFPS(60);

    RenderTexture2D canvas = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

    BeginTextureMode(canvas);
    ClearBackground(BLACK);
    EndTextureMode();

    while (!WindowShouldClose())
    {
        // Cpu cycle
        accumulator += GetFrameTime();

        if (accumulator >= tick_rate)
        {

            accumulator -= tick_rate;
            // Run A cpu cycle here

            update_input(&_chip8);
            for (int i = 0; i < cycles_per_frame; i++)
            {
                simulate_cpu_cycle(&_chip8);
            }

            if (decrement_timers(&_chip8))
                PlaySound(beep);
        }

        // Draws gfx to screen

        BeginTextureMode(canvas);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 255});
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                if (_chip8.gfx[x + (y * width)] == 1)
                {
                    DrawRectangle(x * scaling, y * scaling, scaling, scaling, WHITE);
                }
            }
        }
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureRec(canvas.texture, (Rectangle){0, 0, (float)canvas.texture.width, (float)-canvas.texture.height}, (Vector2){0, 0}, WHITE);
        // DrawText(TextFormat("FPS %d", GetFPS()), 10, 10, 20, WHITE);
        EndDrawing();

        // Input cycle
        memcpy(_chip8.key_prev, _chip8.key, sizeof(_chip8.key));
    }

    UnloadSound(beep);
    UnloadRenderTexture(canvas);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}

void update_input(chip8 *p_chip8)
{

    /*
    +-+-+-+-+                +-+-+-+-+
    |1|2|3|C|                |1|2|3|4|
    +-+-+-+-+                +-+-+-+-+
    |4|5|6|D|                |Q|W|E|R|
    +-+-+-+-+       =>       +-+-+-+-+
    |7|8|9|E|                |A|S|D|F|
    +-+-+-+-+                +-+-+-+-+
    |A|0|B|F|                |Z|X|C|V|
    +-+-+-+-+                +-+-+-+-+
    */
    p_chip8->key[0x0] = IsKeyDown(KEY_X);
    p_chip8->key[0x1] = IsKeyDown(KEY_ONE);
    p_chip8->key[0x2] = IsKeyDown(KEY_TWO);
    p_chip8->key[0x3] = IsKeyDown(KEY_THREE);
    p_chip8->key[0x4] = IsKeyDown(KEY_Q);
    p_chip8->key[0x5] = IsKeyDown(KEY_W);
    p_chip8->key[0x6] = IsKeyDown(KEY_E);
    p_chip8->key[0x7] = IsKeyDown(KEY_A);
    p_chip8->key[0x8] = IsKeyDown(KEY_S);
    p_chip8->key[0x9] = IsKeyDown(KEY_D);
    p_chip8->key[0xA] = IsKeyDown(KEY_Z);
    p_chip8->key[0xB] = IsKeyDown(KEY_C);
    p_chip8->key[0xC] = IsKeyDown(KEY_FOUR);
    p_chip8->key[0xD] = IsKeyDown(KEY_R);
    p_chip8->key[0xE] = IsKeyDown(KEY_F);
    p_chip8->key[0xF] = IsKeyDown(KEY_V);
}
