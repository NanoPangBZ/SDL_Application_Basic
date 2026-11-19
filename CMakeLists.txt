cmake_minimum_required(VERSION 3.16)

project(SDL_Application_Basic
    VERSION 0.1.0
    LANGUAGES C ASM
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 仅构建静态 SDL3，关闭共享库 / 测试 / 示例
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/SDL3 EXCLUDE_FROM_ALL)

# SDL3_ttf 使用随源码提供的 FreeType，避免目标机器依赖字体库 DLL。
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDLTTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)
set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLTTF_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/SDL3_ttf EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME}
    src/main.c
    src/framework/framework.c
    src/module/view/view.c
)

include(src/framework/file_embed/EmbedFile.cmake)
embed_file(${PROJECT_NAME} font "resource/font/NotoSansCJK-Regular.ttc")
target_include_directories(${PROJECT_NAME} PRIVATE src)

# 静态链接 SDL3（符号打进可执行文件）
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3-static
    SDL3_ttf::SDL3_ttf-static
)

# Unix 上 sin/sqrt 需要链接 libm
if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

# MinGW：静态链接 libgcc、winpthread 等运行时库；系统 UCRT 仍由 Windows 提供。
if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -static)
endif()
