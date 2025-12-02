#include "files.cc"
#include "str.cc"

#include <assert.h>
#include <ctype.h>

Splitter lineIter(Str input) {
    return splitter('\n', input);
}

void partOne(Str input) {
    Splitter iter = lineIter(input);

    int zero_count = 0;
    int pos = 50;
    Str line = {};
    while (iter.next(&line)) {
        if (line.len == 0) {
            continue;
        }

        char dirc = line[0];
        if (dirc != 'L' && dirc != 'R') {
            continue;
        }

        usize line_idx = 1;
        int count = 0;
        while (line_idx < line.len && isdigit(line[line_idx])) {
            count = (10 * count) + (line[line_idx] - '0');
            ++line_idx;
        }

        pos = (pos + (dirc == 'L' ? (-count) : (count))) % 100;
        if (pos == 0) {
            ++zero_count;
        }
    }

    printf("Zero count = %d\n", zero_count);
}

void partTwo(Str input) {
    Splitter iter = lineIter(input);

    usize total_count = 0;
    usize cur = 50;

    Str line = {};
    while (iter.next(&line)) {
        if (line.len == 0) {
            continue;
        }

        char dirc = line[0];
        if (dirc != 'L' && dirc != 'R') {
            continue;
        }

        usize rot_count = 0;
        usize line_idx = 1;
        while (line_idx < line.len && isdigit(line[line_idx])) {
            rot_count = (10 * rot_count) + (line[line_idx] - '0');
            ++line_idx;
        }

        usize mod = rot_count % 100;
        usize wrap = rot_count / 100;

        total_count += wrap;

        if (dirc == 'L') {
            // otherwise we double count
            if (cur == 0) {
                cur = 100;
            }

            if (cur <= mod) {
                ++total_count;

                cur = ((cur + 100) - mod) % 100;
            } else {
                cur = (cur - mod) % 100;
            }
        } else {
            usize avail = 100 - cur;
            if (mod >= avail) {
                ++total_count;
            }
            cur = (cur + mod) % 100;
        }
    }

    printf("Zero count = %zu\n", total_count);
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
