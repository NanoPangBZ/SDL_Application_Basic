#pragma once

#include <stdint.h>

/* 框架用户模块 */
typedef struct framework_user_module_opt_t{
    const char* name;
    int (*init)(void* args);
    int (*enter)(void* args);
    int (*exit)(void* args);
    void (*cleanup)(void* args);
}framework_user_module_opt_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 启动应用
 * @param user_module_opt_array 框架用户模块数组
 * @param module_count 框架用户模块数量
 * @param args 框架用户模块参数指针
 * @return 0 成功，其他 失败
*/
int framework_app_start( const framework_user_module_opt_t user_module_opt_array[] , int module_count , void* args );

/**
 * @brief 退出应用
*/
void framework_app_exit( void );

#ifdef __cplusplus
}
#endif

