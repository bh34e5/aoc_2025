#include "arena.cc"
#include "files.cc"
#include "str.cc"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void safeSet(char c, char *pc) {
    if (pc != nullptr) {
        *pc = c;
    }
}

void safeInc(usize *pu, usize n) {
    if (pu != nullptr) {
        *pu += n;
    }
}

void partOne(Str input) {
    Splitter lines = splitter('\n', input);

    usize total_split = 0;
    usize width = 0;

    Str line;
    Str last_line;
    assert(lines.next(&last_line));
    assert(last_line.len > 0);
    width = last_line.len;

    while (lines.next(&line)) {
        if (line.len > 0) {
            assert(line.len == width);

            for (usize i = 0; i < width; ++i) {
                char prev = last_line[i];
                char cur = line[i];

                char *cur_prev = i == 0 ? nullptr : &line[i - 1];
                char *cur_next = i == width - 1 ? nullptr : &line[i + 1];

                if (prev == '|' || prev == 'S') {
                    if (cur == '^') {
                        safeSet('|', cur_prev);
                        safeSet('|', cur_next);

                        ++total_split;
                    } else {
                        line[i] = '|';
                    }
                }
            }
        }

        last_line = line;
    }

    printf("Total splits = %zu\n", total_split);
}

void partTwo(Str input) {
#define BUF_LEN (1024 * 1024)
    char buf[BUF_LEN];

    Splitter lines = splitter('\n', input);

    Arena _arena = fromBuffer(buf, BUF_LEN);
    Arena *arena = &_arena;

    usize width = 0;

    Str line;
    Str last_line;
    assert(lines.next(&last_line));
    assert(last_line.len > 0);
    width = last_line.len;

    usize *numbers_a = pushArr<usize>(arena, width);
    usize *numbers_b = pushArr<usize>(arena, width);
    for (usize i = 0; i < width; ++i) {
        numbers_a[i] = last_line[i] == 'S' ? 1 : 0;
    }

    usize *prev_nums = numbers_a;
    usize *cur_nums = numbers_b;

    while (lines.next(&line)) {
        memcpy(cur_nums, prev_nums, width * sizeof(usize));

        if (line.len > 0) {
            assert(line.len == width);

            for (usize i = 0; i < width; ++i) {
                bool first = i == 0;
                bool last = i == width - 1;

                char prev = last_line[i];
                char cur = line[i];

                char *cur_prev = first ? nullptr : &line[i - 1];
                char *cur_next = last ? nullptr : &line[i + 1];

                usize *cur_prev_n = first ? nullptr : &cur_nums[i - 1];
                usize *cur_next_n = last ? nullptr : &cur_nums[i + 1];

                if (prev == '|' || prev == 'S') {
                    if (cur == '^') {
                        safeSet('|', cur_prev);
                        safeSet('|', cur_next);

                        safeInc(cur_prev_n, prev_nums[i]);
                        safeInc(cur_next_n, prev_nums[i]);

                        cur_nums[i] = 0;
                    } else {
                        line[i] = '|';
                    }
                }
            }
        }

        last_line = line;

        {
            usize *tmp = prev_nums;
            prev_nums = cur_nums;
            cur_nums = tmp;
        }
    }

    memcpy(cur_nums, prev_nums, width * sizeof(usize));

    usize total_paths = 0;
    for (usize i = 0; i < width; ++i) {
        total_paths += cur_nums[i];
    }

    printf("Total paths = %zu\n", total_paths);
#undef BUF_LEN
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
