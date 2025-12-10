#pragma once

#include "types.cc"

#include <assert.h>

struct Str {
    usize len;
    char *ptr;

    char &operator[](usize idx) {
        assert(idx < this->len && "Out of bounds");
        return this->ptr[idx];
    }

    Str slice(usize start) {
        assert(start <= this->len && "Out of bounds");
        return this->slice(start, this->len);
    }

    Str slice(usize start, usize end) {
        assert(start <= this->len && "Out of bounds");
        assert(end <= this->len && "Out of bounds");
        assert(start <= end && "Invalid slice");

        Str res = {};
        if (start != end) {
            res.len = end - start;
            res.ptr = this->ptr + start;
        }

        return res;
    }

    char *begin() { return this->ptr; }
    char *end() { return this->ptr + this->len; }

    char const *cbegin() const { return this->ptr; }
    char const *cend() const { return this->ptr + this->len; }
};

struct Splitter {
    Str str;
    char target;
    usize last_idx;
    usize idx;

    bool next(Str *pstr) {
        usize res_idx = 0;
        bool found = false;

        while (this->idx < this->str.len) {
            usize idx = this->idx++;
            char c = this->str[idx];
            if (c == this->target) {
                found = true;
                res_idx = idx;
                break;
            }
        }

        if (!found && this->idx == this->str.len &&
            this->last_idx < this->str.len) {
            found = true;
            res_idx = this->str.len;
        }

        if (found) {
            usize start = this->last_idx;
            this->last_idx = res_idx + 1;

            if (pstr != nullptr) {
                *pstr = this->str.slice(start, res_idx);
            }
        }

        return found;
    }
};

Splitter splitter(char target, Str input) {
    Splitter res = {};
    res.str = input;
    res.target = target;

    return res;
}
