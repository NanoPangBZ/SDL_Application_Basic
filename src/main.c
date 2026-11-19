#include <SDL3/SDL_main.h>

#include "framework/framework.h"
#include "module/view/view.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    return framework_app_start(&view_module_opt, 1, NULL);
}
