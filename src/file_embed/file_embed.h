#ifndef FILE_EMBED_H
#define FILE_EMBED_H

#include <stddef.h>
#include <stdint.h>

#define FILE_EMBED_DECLARE(name) \
    extern const unsigned char file_embed_##name##_begin[]; \
    extern const unsigned char file_embed_##name##_end[]

#define FILE_EMBED_DATA(name) (file_embed_##name##_begin)
#define FILE_EMBED_SIZE(name) \
    ((size_t)((uintptr_t)file_embed_##name##_end - \
              (uintptr_t)file_embed_##name##_begin))

#endif
