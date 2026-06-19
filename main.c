#include "raylib.h"
#include "chip8.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

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
    _chip8.quirk = false; // if we should increment index counter in 0xFs55 and 0xFx65 and other quirks ask ai or something, MODERN ONES PUT THIS TO FALSE

    init_chip8(&_chip8);
    load_rom_file(&_chip8, "chip8_roms/1-chip8-logo.ch8");

    InitAudioDevice();
    Sound beep = LoadSound("resources/beep.mp3");

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
