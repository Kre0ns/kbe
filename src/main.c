#include <stdio.h>

#include "raylib.h"

#include "gb.h"
#include "cart.h"
#include "cpu.h"
#include "bus.h"
#include "ppu.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define SCALE 4

static void image_from_frame(gb_t *gb, Image *image);

int main()
{
    gb_t gb = {0};
    gb_init(&gb);

    cart_load(&gb, "./Tetris (World) (Rev 1).gb");

    printf("Title: %s\n", (char*)(gb.cart.rom + 0x134));

    InitWindow(SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, "kbe");

    SetTargetFPS(60);

    Image image = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
    Texture2D texture = LoadTextureFromImage(image);

    while (!WindowShouldClose())
    {
        while (!gb.ppu.frame_ready) ppu_step(&gb, cpu_step(&gb));
        gb.ppu.frame_ready = false;

        image_from_frame(&gb, &image);
        UpdateTexture(texture, image.data);

        BeginDrawing();

            DrawTextureEx(texture, (Vector2){0, 0}, 0.0f, (float)SCALE, WHITE);

        EndDrawing();
    }

    CloseWindow();

    cart_free(&gb);

    return 0;
}

static void image_from_frame(gb_t *gb, Image *image)
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            uint8_t color = 255;

            switch (gb->ppu.frame_buffer[y][x])
            {
            case 1: color = 128;
                break;
            
            case 2: color = 64;
                break;

            case 3: color = 0;
                break;
            
            default:
                break;
            }

            ((uint8_t *)image->data)[y * SCREEN_WIDTH + x] = color;
        }
    }
}