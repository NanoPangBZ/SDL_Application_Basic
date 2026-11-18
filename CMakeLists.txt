cmake_minimum_required(VERSION 3.16)

project(SDL_Application_Basic
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 仅构建本应用所需的 SDL3，关闭测试与示例以加快配置/编译
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/SDL3 EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME}
    src/main.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
)

# Unix 上 sin/sqrt 需要链接 libm
if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

# Windows：把 SDL3.dll 拷到可执行文件旁，方便直接运行
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copy SDL3 shared library next to executable"
    )
endif()
