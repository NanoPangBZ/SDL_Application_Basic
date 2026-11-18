cmake_minimum_required(VERSION 3.16)

project(SDL_Application_Basic
    VERSION 0.1.0
    LANGUAGES C
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

add_executable(${PROJECT_NAME}
    src/main.c
)

# 静态链接 SDL3（符号打进可执行文件）
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3-static
)

# Unix 上 sin/sqrt 需要链接 libm
if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

# MinGW：静态链接 libgcc、winpthread 等运行时库；系统 UCRT 仍由 Windows 提供。
if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -static)
endif()
