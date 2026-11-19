#include "framework.h"

#include <SDL3/SDL.h>
#include <stdlib.h>

typedef struct framework_user_module_t {
    SDL_Thread *thread;
    const framework_user_module_opt_t *opt;
    int result;
} framework_user_module_t;

typedef struct framework_ctx_t {
    framework_user_module_t *modules;
    int count;
    void *args;
    SDL_AtomicInt stopping;
} framework_ctx_t;

static framework_ctx_t framework_ctx;

static int SDLCALL user_module_thread_wrapper(void *arg)
{
    framework_user_module_t *module = (framework_user_module_t *)arg;
    module->result = module->opt->enter ? module->opt->enter(framework_ctx.args) : 0;
    framework_app_exit();
    return module->result;
}

void framework_app_exit(void)
{
    if (!SDL_CompareAndSwapAtomicInt(&framework_ctx.stopping, 0, 1)) {
        return;
    }
    for (int i = 0; i < framework_ctx.count; ++i) {
        if (framework_ctx.modules[i].opt->exit) {
            framework_ctx.modules[i].opt->exit(framework_ctx.args);
        }
    }
}

int framework_app_start(const framework_user_module_opt_t modules[], int count, void *args)
{
    int initialized = 0;
    int result = 0;

    if (!modules || count <= 0 || framework_ctx.modules) {
        return -1;
    }
    framework_ctx.modules = (framework_user_module_t *)calloc((size_t)count,
                                                              sizeof(*framework_ctx.modules));
    if (!framework_ctx.modules) {
        return -1;
    }
    framework_ctx.count = count;
    framework_ctx.args = args;
    SDL_SetAtomicInt(&framework_ctx.stopping, 0);
    for (int i = 0; i < count; ++i) {
        framework_ctx.modules[i].opt = &modules[i];
    }

    for (int i = 0; i < count; ++i) {
        if (modules[i].init && (result = modules[i].init(args)) != 0) {
            framework_ctx.count = initialized;
            goto done;
        }
        initialized++;
    }

    /* 首个模块在主线程运行，供 SDL 窗口和事件循环使用。 */
    for (int i = 1; i < count; ++i) {
        framework_user_module_t *module = &framework_ctx.modules[i];
        module->thread = SDL_CreateThread(user_module_thread_wrapper, module->opt->name, module);
        if (!module->thread) {
            SDL_Log("SDL_CreateThread failed: %s", SDL_GetError());
            result = -1;
            goto done;
        }
    }

    if (modules[0].enter) {
        result = modules[0].enter(args);
    }

done:
    framework_app_exit();
    for (int i = 1; i < count; ++i) {
        framework_user_module_t *module = &framework_ctx.modules[i];
        if (module->thread) {
            SDL_WaitThread(module->thread, NULL);
            if (result == 0 && module->result != 0) {
                result = module->result;
            }
        }
    }
    for (int i = initialized - 1; i >= 0; --i) {
        if (modules[i].cleanup) {
            modules[i].cleanup(args);
        }
    }
    free(framework_ctx.modules);
    framework_ctx.modules = NULL;
    framework_ctx.count = 0;
    framework_ctx.args = NULL;
    return result;
}
