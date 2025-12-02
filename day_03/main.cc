#include "files.cc"
#include "str.cc"
#include "types.cc"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>

struct Max {
    usize idx;
    char c;
};

Max getMax(Str input) {
    assert(input.len > 0);

    Max max = {};
    max.c = input[0];

    usize idx = 1;
    while (idx < input.len) {
        if (input[idx] > max.c) {
            max.idx = idx;
            max.c = input[idx];
        }
        ++idx;
    }

    return max;
}

inline usize tenPow(usize n) {
    usize res = 1;
    while (n--) {
        res *= 10;
    }
    return res;
}

usize maxFromStr(Str str, usize dig_count) {
    assert(dig_count > 0);
    assert(str.len >= dig_count);

    if (dig_count == 1) {
        Max m = getMax(str);

        assert(isdigit(m.c));
        return m.c - '0';
    }

    Max mUpper = getMax(str.slice(0, str.len - (dig_count - 1)));
    assert(isdigit(mUpper.c));

    usize nUpper = tenPow(dig_count - 1) * (mUpper.c - '0');
    usize nLower = maxFromStr(str.slice(mUpper.idx + 1), dig_count - 1);

    return nUpper + nLower;
}

void partOne(Str input) {
    Splitter line_splitter = splitter('\n', input);

    usize total = 0;

    Str line;
    while (line_splitter.next(&line)) {
        if (line.len == 0) {
            continue;
        }

        total += maxFromStr(line, 2);
    }

    printf("Total = %zu\n", total);
}

void partTwo(Str input) {
    Splitter line_splitter = splitter('\n', input);

    usize total = 0;

    Str line;
    while (line_splitter.next(&line)) {
        if (line.len == 0) {
            continue;
        }

        total += maxFromStr(line, 12);
    }

    printf("Total = %zu\n", total);
}

int main(int argc, char const *argv[]) {
#define BUF_LEN (1024 * 1024)
    char buf[BUF_LEN];

    if (argc < 2) {
        return 1;
    }

    int part = argv[1][0] == '2' ? 2 : 1;
    char const *filename = argc > 2 ? argv[2] : "input.txt";

    Str input = readFile(filename, buf, BUF_LEN);
    if (part == 1) {
        partOne(input);
    } else {
        partTwo(input);
    }

    return 0;
#undef BUF_LEN
}
