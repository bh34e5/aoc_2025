#include "files.cc"
#include "types.cc"

#include <assert.h>
#include <stdio.h>

void readRange(Str range, usize *lower, usize *upper) {
    Splitter dashSplitter = splitter('-', range);

    Str low_str;
    Str high_str;

    assert(dashSplitter.next(&low_str));
    assert(dashSplitter.next(&high_str));
    assert(!dashSplitter.next(nullptr));

    *lower = strtoull(low_str.ptr, nullptr, 10);
    *upper = strtoull(high_str.ptr, nullptr, 10);
}

bool isRepeatedPartOne(usize num) {
    usize last_top = 1;
    usize cur_top = 10;
    while (cur_top <= num) {
        usize test = cur_top + 1;

        if (num % test == 0) {
            usize divisor = num / test;
            if (last_top <= divisor && divisor < cur_top) {
                return true;
            }
        }

        last_top = cur_top;
        cur_top *= 10;
    }

    return false;
}

void partOne(Str input) {
    Str line;
    Splitter line_splitter = splitter('\n', input);
    assert(line_splitter.next(&line));
    assert(!line_splitter.next(nullptr));

    usize total = 0;

    Str range = {};
    Splitter comma_splitter = splitter(',', line);
    while (comma_splitter.next(&range)) {
        usize low, high;
        readRange(range, &low, &high);

        for (usize i = low; i <= high; ++i) {
            if (isRepeatedPartOne(i)) {
                total += i;
            }
        }
    }

    printf("Total = %zu\n", total);
}

bool isRepeatedPartTwo(usize num) {
    usize last_top = 1;
    usize cur_top = 10;
    while (cur_top <= num) {
        usize next_to_add = cur_top * cur_top;
        usize test = cur_top + 1;
        while (test <= num) {
            if (num % test == 0) {
                usize divisor = num / test;
                if (last_top <= divisor && divisor < cur_top) {
                    return true;
                }
            }

            test += next_to_add;
            next_to_add *= cur_top;
        }

        last_top = cur_top;
        cur_top *= 10;
    }

    return false;
}

void partTwo(Str input) {
    Str line;
    Splitter line_splitter = splitter('\n', input);
    assert(line_splitter.next(&line));
    assert(!line_splitter.next(nullptr));

    usize total = 0;

    Str range = {};
    Splitter comma_splitter = splitter(',', line);
    while (comma_splitter.next(&range)) {
        usize low, high;
        readRange(range, &low, &high);

        for (usize i = low; i <= high; ++i) {
            if (isRepeatedPartTwo(i)) {
                total += i;
            }
        }
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
