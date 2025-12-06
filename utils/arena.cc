#pragma once

#include "types.cc"

#include <assert.h>

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
    assert(arena->size + size <= arena->capacity);

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
