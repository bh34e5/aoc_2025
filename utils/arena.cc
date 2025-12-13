#pragma once

#include "types.cc"

#include <stdio.h>
#include <stdlib.h>

struct Arena {
    void *data;
    usize size;
    usize capacity;
};

Arena fromBuffer(char *buf, usize buf_len) {
    Arena res = {};
    res.data = buf;
    res.capacity = buf_len;

    return res;
}

void *pushSize(Arena *arena, usize size) {
    if (arena->size + size > arena->capacity) {
        fprintf(stderr,
                "Allocation failed. Requested %zu bytes, but only have %zu "
                "bytes remaining\n",
                size, arena->capacity - arena->size);
        exit(1);
    }

    void *res = (char *)arena->data + arena->size;
    arena->size += size;

    return res;
}

template <typename T> T *push(Arena *arena) {
    void *res = pushSize(arena, sizeof(T));
    return (T *)res;
}

template <typename T> T *pushArr(Arena *arena, usize count) {
    void *res = nullptr;
    if (count > 0) {
        res = pushSize(arena, count * sizeof(T));
    }
    return (T *)res;
}
