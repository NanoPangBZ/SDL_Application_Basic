#include "view.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>

#include "framework/file_embed/file_embed.h"

FILE_EMBED_DECLARE(font);

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 500
#define FONT_SIZE 42.0f
#define LINE_GAP 18.0f

static const char *text_lines[] = {
    "中文：测试",
    "English: Test",
    "한국어: 테스트",
    "日本語：テスト"
};

typedef struct view_ctx_t {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_IOStream *font_io;
    TTF_Font *font;
    SDL_Texture *textures[SDL_arraysize(text_lines)];
    SDL_FRect rects[SDL_arraysize(text_lines)];
    bool sdl_initialized;
    bool ttf_initialized;
    SDL_AtomicInt running;
} view_ctx_t;

static view_ctx_t view_ctx;

static void draw_circle(SDL_Renderer *renderer, int cx, int cy, int radius)
{
    SDL_SetRenderDrawColor(renderer, 35, 105, 190, 255);
    for (int dy = -radius; dy <= radius; ++dy) {
        int half_width = (int)sqrt((double)radius * radius - (double)dy * dy);
        SDL_RenderLine(renderer, (float)(cx - half_width), (float)(cy + dy),
                       (float)(cx + half_width), (float)(cy + dy));
    }
}

static void view_cleanup(void *args)
{
    (void)args;
    for (size_t i = 0; i < SDL_arraysize(text_lines); ++i) {
        SDL_DestroyTexture(view_ctx.textures[i]);
        view_ctx.textures[i] = NULL;
    }
    TTF_CloseFont(view_ctx.font);
    view_ctx.font = NULL;
    if (view_ctx.font_io) {
        SDL_CloseIO(view_ctx.font_io);
        view_ctx.font_io = NULL;
    }
    SDL_DestroyRenderer(view_ctx.renderer);
    SDL_DestroyWindow(view_ctx.window);
    view_ctx.renderer = NULL;
    view_ctx.window = NULL;
    if (view_ctx.ttf_initialized) {
        TTF_Quit();
        view_ctx.ttf_initialized = false;
    }
    if (view_ctx.sdl_initialized) {
        SDL_Quit();
        view_ctx.sdl_initialized = false;
    }
}

static int view_init(void *args)
{
    float total_height = 0.0f;
    float y;
    (void)args;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }
    view_ctx.sdl_initialized = true;
    if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        goto fail;
    }
    view_ctx.ttf_initialized = true;
    if (!SDL_CreateWindowAndRenderer("Multilingual text", WINDOW_WIDTH, WINDOW_HEIGHT,
                                     0, &view_ctx.window, &view_ctx.renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        goto fail;
    }

    view_ctx.font_io = SDL_IOFromConstMem(FILE_EMBED_DATA(font), FILE_EMBED_SIZE(font));
    if (!view_ctx.font_io) {
        SDL_Log("Font memory stream failed: %s", SDL_GetError());
        goto fail;
    }
    view_ctx.font = TTF_OpenFontIO(view_ctx.font_io, false, FONT_SIZE);
    if (!view_ctx.font) {
        SDL_Log("TTF_OpenFontIO failed: %s", SDL_GetError());
        goto fail;
    }

    for (size_t i = 0; i < SDL_arraysize(text_lines); ++i) {
        SDL_Surface *surface = TTF_RenderText_Blended(view_ctx.font, text_lines[i], 0,
                                                       (SDL_Color){255, 255, 255, 255});
        if (!surface) {
            SDL_Log("Text rendering failed: %s", SDL_GetError());
            goto fail;
        }
        view_ctx.textures[i] = SDL_CreateTextureFromSurface(view_ctx.renderer, surface);
        view_ctx.rects[i].w = (float)surface->w;
        view_ctx.rects[i].h = (float)surface->h;
        SDL_DestroySurface(surface);
        if (!view_ctx.textures[i]) {
            SDL_Log("Texture creation failed: %s", SDL_GetError());
            goto fail;
        }
        total_height += view_ctx.rects[i].h;
    }

    total_height += LINE_GAP * (SDL_arraysize(text_lines) - 1);
    y = (WINDOW_HEIGHT - total_height) / 2.0f;
    for (size_t i = 0; i < SDL_arraysize(text_lines); ++i) {
        view_ctx.rects[i].x = (WINDOW_WIDTH - view_ctx.rects[i].w) / 2.0f;
        view_ctx.rects[i].y = y;
        y += view_ctx.rects[i].h + LINE_GAP;
    }
    SDL_SetAtomicInt(&view_ctx.running, 1);
    return 0;

fail:
    view_cleanup(NULL);
    return -1;
}

static int view_enter(void *args)
{
    float sin_pos = 0.0f;
    (void)args;
    while (SDL_GetAtomicInt(&view_ctx.running)) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
                SDL_SetAtomicInt(&view_ctx.running, 0);
            }
        }
        SDL_SetRenderDrawColor(view_ctx.renderer, 0, 0, 0, 255);
        SDL_RenderClear(view_ctx.renderer);
        draw_circle(view_ctx.renderer, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2,
                    150 + (int)(100.0f * sinf(sin_pos)));
        for (size_t i = 0; i < SDL_arraysize(text_lines); ++i) {
            SDL_RenderTexture(view_ctx.renderer, view_ctx.textures[i], NULL,
                              &view_ctx.rects[i]);
        }
        SDL_RenderPresent(view_ctx.renderer);
        sin_pos += 0.08f;
        SDL_Delay(16);
    }
    return 0;
}

static int view_exit(void *args)
{
    (void)args;
    SDL_SetAtomicInt(&view_ctx.running, 0);
    return 0;
}

const framework_user_module_opt_t view_module_opt = {
    .name = "view",
    .init = view_init,
    .enter = view_enter,
    .exit = view_exit,
    .cleanup = view_cleanup
};
