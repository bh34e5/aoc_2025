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

template <typename T> T *push(Arena *arena) {
    assert(arena->size + sizeof(T) <= arena->capacity);

    T *res = (T *)(void *)((char *)arena->data + arena->size);
    arena->size += sizeof(T);

    return res;
}
