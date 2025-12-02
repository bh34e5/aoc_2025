#pragma once

#include "str.cc"
#include "types.cc"

#include <stdio.h>

Str readFile(char const *filename, char buf[], usize buflen) {
    Str res = {};
    usize len = 0;

    FILE *f = fopen(filename, "r");
    if (f == nullptr) {
        goto cleanup;
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (len > buflen) {
        goto cleanup;
    }

    if (fread(buf, len, 1, f) != 1) {
        goto cleanup;
    }

    res.len = len;
    res.ptr = buf;

cleanup:
    fclose(f);

    return res;
}
