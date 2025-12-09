#include "arena.cc"
#include "files.cc"
#include "str.cc"

#include <assert.h>
#include <stdlib.h>

inline usize ulmin(usize a, usize b) { return a < b ? a : b; }
inline usize ulmax(usize a, usize b) { return a > b ? a : b; }

usize absDiff(usize a, usize b) {
    usize res = a < b ? b - a : a - b;
    return res;
}

struct Point {
    usize x;
    usize y;
    usize z;

    usize circuit_id;
};

usize sqDistBetween(Point a, Point b) {
    usize dx = absDiff(a.x, b.x);
    usize dy = absDiff(a.y, b.y);
    usize dz = absDiff(a.z, b.z);
    return dx * dx + dy * dy + dz * dz;
}

struct PointPair {
    usize idx_a;
    usize idx_b;
    usize sq_dist;
};

Point *pointA(PointPair pair, Point *points) {
    Point *res = points + pair.idx_a;
    return res;
}

Point *pointB(PointPair pair, Point *points) {
    Point *res = points + pair.idx_b;
    return res;
}

Point pointFrom(Str line) {
    Splitter commas = splitter(',', line);

    Str a, b, c;
    assert(commas.next(&a));
    assert(commas.next(&b));
    assert(commas.next(&c));
    assert(!commas.next(nullptr));

    usize x = strtoul(a.ptr, nullptr, 10);
    usize y = strtoul(b.ptr, nullptr, 10);
    usize z = strtoul(c.ptr, nullptr, 10);

    Point p = {};
    p.x = x;
    p.y = y;
    p.z = z;

    return p;
}

void loadPoints(Arena *arena, Str input, usize *plen, Point **ppoints) {
    usize line_count = 0;
    {
        Str line;
        Splitter lines = splitter('\n', input);
        while (lines.next(&line)) {
            if (line.len > 0) {
                ++line_count;
            }
        }
    }

    Point *points = pushArr<Point>(arena, line_count);

    *plen = line_count;
    *ppoints = points;

    Str line;
    Splitter lines = splitter('\n', input);
    while (lines.next(&line)) {
        *points = pointFrom(line);
        points += 1;
    }
}

void loadPointPairs(Arena *arena, usize points_len, Point *points, usize *plen,
                    PointPair **ppairs) {
    usize len = (points_len * points_len - points_len) / 2;
    PointPair *pairs = pushArr<PointPair>(arena, len);

    *plen = len;
    *ppairs = pairs;

    for (usize i = 0; i < points_len; ++i) {
        for (usize j = i + 1; j < points_len; ++j) {
            Point a = points[i];
            Point b = points[j];

            PointPair pair = {};
            pair.idx_a = i;
            pair.idx_b = j;
            pair.sq_dist = sqDistBetween(a, b);

            *pairs = pair;
            pairs += 1;
        }
    }
}

void sortOnce(usize len, PointPair *pairs) {
    if (len <= 1) {
        return;
    }

    usize min_idx = 0;
    usize cur_min = pairs[0].sq_dist;

    for (usize i = 1; i < len; ++i) {
        usize next = pairs[i].sq_dist;
        if (next < cur_min) {
            min_idx = i;
            cur_min = next;
        }
    }

    if (min_idx > 0) {
        PointPair temp = pairs[0];

        pairs[0] = pairs[min_idx];
        pairs[min_idx] = temp;
    }
}

void reassignAToB(usize id_a, usize id_b, usize len, Point *points) {
    for (usize i = 0; i < len; ++i) {
        Point *p = points + i;
        if (p->circuit_id == id_a) {
            p->circuit_id = id_b;
        }
    }
}

void partOne(Str input) {
#define BUF_LEN (1024 * 1024 * 1024)
    char *buf = new char[BUF_LEN];

    Arena arena = fromBuffer(buf, BUF_LEN);

    usize points_len = 0;
    Point *points = nullptr;
    loadPoints(&arena, input, &points_len, &points);

    usize pairs_len = 0;
    PointPair *pairs = nullptr;
    loadPointPairs(&arena, points_len, points, &pairs_len, &pairs);

    usize connection_count = 1000;
#if SAMPLE
    connection_count = 10;
#endif

    usize next_id = 1;

    usize rem_pairs_len = pairs_len;
    PointPair *rem_pairs = pairs;
    while (connection_count > 0) {
        sortOnce(rem_pairs_len, rem_pairs);

        PointPair smallest_pair = *rem_pairs;

        rem_pairs_len -= 1;
        rem_pairs += 1;

        Point *pa = pointA(smallest_pair, points);
        Point *pb = pointB(smallest_pair, points);

        usize min_id = ulmin(pa->circuit_id, pb->circuit_id);
        usize max_id = ulmax(pa->circuit_id, pb->circuit_id);

        if (max_id == 0) {
            usize cur_id = next_id++;
            pa->circuit_id = cur_id;
            pb->circuit_id = cur_id;
        } else if (min_id == 0) {
            pa->circuit_id = max_id;
            pb->circuit_id = max_id;
        } else if (min_id != max_id) {
            reassignAToB(max_id, min_id, points_len, points);
        }

        --connection_count;
    }

    usize max_a = 1;
    usize max_b = 1;
    usize max_c = 1;
    for (usize id = 1; id < next_id; ++id) {
        usize id_count = 0;
        for (usize i = 0; i < points_len; ++i) {
            if (points[i].circuit_id == id) {
                ++id_count;
            }
        }

        if (id_count > max_c) {
            max_c = id_count;
        }

        if (max_c > max_b) {
            usize temp = max_b;
            max_b = max_c;
            max_c = temp;
        }

        if (max_b > max_a) {
            usize temp = max_a;
            max_a = max_b;
            max_b = temp;
        }
    }

    usize product = max_a * max_b * max_c;
    printf("Product three = %zu\n", product);

    delete[] buf;

#undef BUF_LEN
}

void partTwo(Str input) {
#define BUF_LEN (1024 * 1024 * 1024)
    char *buf = new char[BUF_LEN];

    Arena arena = fromBuffer(buf, BUF_LEN);

    usize points_len = 0;
    Point *points = nullptr;
    loadPoints(&arena, input, &points_len, &points);

    usize pairs_len = 0;
    PointPair *pairs = nullptr;
    loadPointPairs(&arena, points_len, points, &pairs_len, &pairs);

    usize connection_count = points_len - 1;

    usize next_id = 1;

    usize last_x_a = 0;
    usize last_x_b = 0;

    usize rem_pairs_len = pairs_len;
    PointPair *rem_pairs = pairs;
    while (connection_count > 0) {
        sortOnce(rem_pairs_len, rem_pairs);

        PointPair smallest_pair = *rem_pairs;

        rem_pairs_len -= 1;
        rem_pairs += 1;

        Point *pa = pointA(smallest_pair, points);
        Point *pb = pointB(smallest_pair, points);

        last_x_a = pa->x;
        last_x_b = pb->x;

        usize min_id = ulmin(pa->circuit_id, pb->circuit_id);
        usize max_id = ulmax(pa->circuit_id, pb->circuit_id);

        if (max_id == 0) {
            usize cur_id = next_id++;
            pa->circuit_id = cur_id;
            pb->circuit_id = cur_id;

            --connection_count;
        } else if (min_id == 0) {
            pa->circuit_id = max_id;
            pb->circuit_id = max_id;

            --connection_count;
        } else if (min_id != max_id) {
            reassignAToB(max_id, min_id, points_len, points);

            --connection_count;
        }
    }

    usize max_a = 1;
    usize max_b = 1;
    usize max_c = 1;
    for (usize id = 1; id < next_id; ++id) {
        usize id_count = 0;
        for (usize i = 0; i < points_len; ++i) {
            if (points[i].circuit_id == id) {
                ++id_count;
            }
        }

        if (id_count > max_c) {
            max_c = id_count;
        }

        if (max_c > max_b) {
            usize temp = max_b;
            max_b = max_c;
            max_c = temp;
        }

        if (max_b > max_a) {
            usize temp = max_a;
            max_a = max_b;
            max_b = temp;
        }
    }

    usize product = last_x_a * last_x_b;
    printf("Product x's = %zu\n", product);

    delete[] buf;

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
