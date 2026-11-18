#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdint.h>
#include "font_data.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 500
#define FONT_SIZE 42.0f
#define LINE_GAP 18.0f

static void draw_circle(SDL_Renderer *renderer, int cx, int cy, int radius)
{
    SDL_SetRenderDrawColor(renderer, 35, 105, 190, 255);
    for (int dy = -radius; dy <= radius; ++dy) {
        int half_width = (int)sqrt((double)radius * radius - (double)dy * dy);
        SDL_RenderLine(renderer, (float)(cx - half_width), (float)(cy + dy),
                       (float)(cx + half_width), (float)(cy + dy));
    }
}

int main(int argc, char *argv[])
{
    static const char *lines[] = {
        "中文：测试",
        "English: Test",
        "한국어: 테스트",
        "日本語：テスト"
    };
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *textures[SDL_arraysize(lines)] = {0};
    SDL_FRect rects[SDL_arraysize(lines)] = {0};
    TTF_Font *font = NULL;
    SDL_IOStream *font_io = NULL;
    bool running = true;
    int exit_code = 1;
    float total_height = 0.0f;
    float y;
    float sin_pos = 0.0f;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        goto cleanup;
    }
    if (!SDL_CreateWindowAndRenderer("Multilingual text", WINDOW_WIDTH, WINDOW_HEIGHT,
                                     0, &window, &renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        goto cleanup;
    }

    font_io = SDL_IOFromConstMem(g_font_data,
                                 (size_t)((uintptr_t)g_font_data_end - (uintptr_t)g_font_data));
    if (!font_io) {
        SDL_Log("Font memory stream failed: %s", SDL_GetError());
        goto cleanup;
    }
    font = TTF_OpenFontIO(font_io, false, FONT_SIZE);
    if (!font) {
        SDL_Log("TTF_OpenFontIO failed: %s", SDL_GetError());
        goto cleanup;
    }

    for (size_t i = 0; i < SDL_arraysize(lines); ++i) {
        SDL_Surface *surface = TTF_RenderText_Blended(font, lines[i], 0,
                                                       (SDL_Color){255, 255, 255, 255});
        if (!surface) {
            SDL_Log("Text rendering failed: %s", SDL_GetError());
            goto cleanup;
        }
        textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        rects[i].w = (float)surface->w;
        rects[i].h = (float)surface->h;
        SDL_DestroySurface(surface);
        if (!textures[i]) {
            SDL_Log("Texture creation failed: %s", SDL_GetError());
            goto cleanup;
        }
        total_height += rects[i].h;
    }

    total_height += LINE_GAP * (SDL_arraysize(lines) - 1);
    y = (WINDOW_HEIGHT - total_height) / 2.0f;
    for (size_t i = 0; i < SDL_arraysize(lines); ++i) {
        rects[i].x = (WINDOW_WIDTH - rects[i].w) / 2.0f;
        rects[i].y = y;
        y += rects[i].h + LINE_GAP;
    }

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
                running = false;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        draw_circle(renderer, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2,
                    150 + (int)(100.0f * sinf(sin_pos)));
        for (size_t i = 0; i < SDL_arraysize(lines); ++i) {
            SDL_RenderTexture(renderer, textures[i], NULL, &rects[i]);
        }
        SDL_RenderPresent(renderer);
        sin_pos += 0.08f;
        SDL_Delay(16);
    }
    exit_code = 0;

cleanup:
    for (size_t i = 0; i < SDL_arraysize(lines); ++i) {
        SDL_DestroyTexture(textures[i]);
    }
    TTF_CloseFont(font);
    SDL_CloseIO(font_io);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return exit_code;
}
