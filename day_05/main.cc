#include "arena.cc"
#include "files.cc"
#include "str.cc"
#include "types.cc"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

inline usize ulmin(usize a, usize b) { return a < b ? a : b; }
inline usize ulmax(usize a, usize b) { return a > b ? a : b; }

struct Range {
    union {
        Range *next;
        Range *next_free;
    };

    usize lower;
    usize upper;
};

inline bool distinct(Range a, Range b) {
    return a.upper < b.lower || b.upper < a.lower;
}
inline bool intersect(Range a, Range b) { return !distinct(a, b); }

Range *getRange(Arena *arena, Range **free_list, usize lower, usize upper) {
    Range *next = nullptr;

    if (*free_list != nullptr) {
        next = *free_list;
        *free_list = next->next_free;
    } else {
        next = push<Range>(arena);
    }

    *next = {};
    next->lower = lower;
    next->upper = upper;

    return next;
}

void insertRange(Range *range, Range **root, Range **free_list) {
    while (*root != nullptr && (*root)->lower < range->lower &&
           (*root)->upper < range->lower) {
        root = &(*root)->next;
    }

    if (*root == nullptr) {
        *root = range;
    } else if (intersect(**root, *range)) {
        (*root)->lower = ulmin((*root)->lower, range->lower);
        (*root)->upper = ulmax((*root)->upper, range->upper);

        range->next_free = *free_list;
        *free_list = range;

        if ((*root)->next != nullptr) {
            Range *next = (*root)->next;
            (*root)->next = next->next;
            insertRange(next, root, free_list);
        }
    } else {
        range->next = *root;
        *root = range;
    }
}

Range *buildRanges(Arena *arena, Splitter *lines) {
    Range *root = nullptr;
    Range *first_free = nullptr;

    Str line;
    while (true) {
        assert(lines->next(&line));
        if (line.len == 0) {
            break;
        }

        Splitter dash_splitter = splitter('-', line);
        Str first, second;
        assert(dash_splitter.next(&first));
        assert(dash_splitter.next(&second));
        assert(!dash_splitter.next(nullptr));

        usize lower = strtoul(first.ptr, nullptr, 10);
        usize upper = strtoul(second.ptr, nullptr, 10);

        Range *next = getRange(arena, &first_free, lower, upper);
        insertRange(next, &root, &first_free);
    }

    return root;
}

bool inRange(usize i, Range *ranges) {
    bool res = false;

    while (ranges != nullptr) {
        if (ranges->lower <= i && i <= ranges->upper) {
            res = true;
            break;
        }

        ranges = ranges->next;
    }

    return res;
}

void partOne(Str input) {
#define BUF_LEN (1024 * 1024)
    char buf[BUF_LEN];

    Splitter lines = splitter('\n', input);
    Arena arena = fromBuffer(buf, BUF_LEN);

    Range *ranges = buildRanges(&arena, &lines);

    Str line;
    usize count = 0;
    while (lines.next(&line)) {
        usize i = strtoul(line.ptr, nullptr, 10);

        if (inRange(i, ranges)) {
            ++count;
        }
    }

    printf("Total count = %zu\n", count);

#undef BUF_LEN
}

void partTwo(Str input) {
#define BUF_LEN (1024 * 1024)
    char buf[BUF_LEN];

    Splitter lines = splitter('\n', input);
    Arena arena = fromBuffer(buf, BUF_LEN);

    Range *ranges = buildRanges(&arena, &lines);

    usize count = 0;
    while (ranges != nullptr) {
        count += ranges->upper - ranges->lower + 1;
        ranges = ranges->next;
    }

    printf("Total count = %zu\n", count);

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
