#include "arena.cc"
#include "files.cc"
#include "str.cc"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

inline usize ulmin(usize a, usize b) { return a < b ? a : b; }
inline usize ulmax(usize a, usize b) { return a > b ? a : b; }

struct Point {
    usize x;
    usize y;
};

usize countPoints(Str input) {
    usize count = 0;

    Str line;
    Splitter lines = splitter('\n', input);
    while (lines.next(&line)) {
        if (line.len == 0) {
            continue;
        }
        ++count;
    }

    return count;
}

void readPoints(Arena *arena, Str input, usize *plen, Point **ppoints) {
    usize len = countPoints(input);
    Point *points = pushArr<Point>(arena, len);

    *plen = len;
    *ppoints = points;

    usize point_ind = 0;
    Point *cur_point = points;

    Str line;
    Splitter lines = splitter('\n', input);
    while (lines.next(&line)) {
        if (line.len == 0) {
            continue;
        }

        Str s_x, s_y;
        Splitter point_splitter = splitter(',', line);
        assert(point_splitter.next(&s_x));
        assert(point_splitter.next(&s_y));
        assert(!point_splitter.next(nullptr));

        usize x = strtoul(s_x.ptr, nullptr, 10);
        usize y = strtoul(s_y.ptr, nullptr, 10);

        Point new_point = {};
        new_point.x = x;
        new_point.y = y;

        assert(point_ind < len);
        *cur_point = new_point;

        ++point_ind;
        ++cur_point;
    }
}

usize numPointsBetween(usize a, usize b) {
    usize count = 0;
    if (a > b) {
        count = a - b + 1;
    } else if (b > a) {
        count = b - a + 1;
    } else {
        count = 1;
    }
    return count;
}

usize areaOf(Point a, Point b) {
    usize rows = numPointsBetween(a.y, b.y);
    usize cols = numPointsBetween(a.x, b.x);
    usize area = rows * cols;
    return area;
}

usize largestRectangle(Point *p_a, Point *p_b, usize len, Point *points) {
    assert(len >= 2);

    Point point_a;
    Point point_b;
    usize max_area = 0;

    if (len == 2) {
        // if there are only two points, this is the largest rectangle

        point_a = points[0];
        point_b = points[1];
        max_area = areaOf(point_a, point_b);
    } else {
        // the largest rectangle either uses the first point or it doesn't
        Point cur_a;
        Point cur_b;
        usize cur_area = largestRectangle(&cur_a, &cur_b, len - 1, points + 1);

        for (usize i = 1; i < len; ++i) {
            usize test_area = areaOf(points[0], points[i]);
            if (test_area > cur_area) {
                cur_a = points[0];
                cur_b = points[i];
                cur_area = test_area;
            }
        }

        point_a = cur_a;
        point_b = cur_b;
        max_area = cur_area;
    }

    *p_a = point_a;
    *p_b = point_b;

    return max_area;
}

void partOne(Str input) {
#define BUF_LEN (1024 * 1024)
    char buf[BUF_LEN];

    Arena arena = fromBuffer(buf, BUF_LEN);

    usize points_len;
    Point *points;

    readPoints(&arena, input, &points_len, &points);

    Point a, b;
    usize largest_area = largestRectangle(&a, &b, points_len, points);

    printf("Largest area = %zu\n", largest_area);

#undef BUF_LEN
}

void hold() { (void)nullptr; }

bool isValidRectangle(Point a, Point b, usize len, Point *points) {
    assert(len >= 2);

    Point last = points[len - 1];
    for (usize i = 0; i < len; ++i) {
        Point next = points[i];

        if (next.x == last.x) {
            // vertical line
            // if this vertical lies within the target rectangle's vertical,
            // check that both sides of the target are on the same side of this
            // vertical

            usize rect_min_y = ulmin(a.y, b.y);
            usize rect_max_y = ulmax(a.y, b.y);

            usize line_min_y = ulmin(next.y, last.y);
            usize line_max_y = ulmax(next.y, last.y);

            bool no_intersect =
                (line_max_y <= rect_min_y) || (rect_max_y <= line_min_y);
            bool does_intersect = !no_intersect;
            if (does_intersect) {
                if (a.x > next.x && b.x < next.x) {
                    hold();
                    return false;
                } else if (a.x < next.x && b.x > next.x) {
                    hold();
                    return false;
                }
            }
        } else if (next.y == last.y) {
            // horizontal line
            // if this horizontal lies within the target rectangle's horizontal,
            // check that both sides of the target are on the same side of this
            // horizontal

            usize rect_min_x = ulmin(a.x, b.x);
            usize rect_max_x = ulmax(a.x, b.x);

            usize line_min_x = ulmin(next.x, last.x);
            usize line_max_x = ulmax(next.x, last.x);

            bool no_intersect =
                (line_max_x <= rect_min_x) || (rect_max_x <= line_min_x);
            bool does_intersect = !no_intersect;
            if (does_intersect) {
                if (a.y < next.y && b.y > next.y) {
                    hold();
                    return false;
                } else if (a.y > next.y && b.y < next.y) {
                    hold();
                    return false;
                }
            }
        } else {
            assert(0 && "Unreachable");
        }

        last = next;
    }

    return true;
}

usize largestRectangleInInterior(Point *p_a, Point *p_b, usize len,
                                 Point *points, usize total_len,
                                 Point *all_points) {
    assert(len >= 2);

    Point point_a;
    Point point_b;
    usize max_area = 0;

    if (len == 2) {
        // if there are only two points, this is the largest rectangle and it
        // must lie in the "inside"

        point_a = points[0];
        point_b = points[1];
        max_area = areaOf(point_a, point_b);
    } else {
        // the largest rectangle either uses the first point or it doesn't
        Point cur_a;
        Point cur_b;
        usize cur_area = largestRectangleInInterior(
            &cur_a, &cur_b, len - 1, points + 1, total_len, all_points);

        for (usize i = 1; i < len; ++i) {
            if (!isValidRectangle(points[0], points[i], total_len,
                                  all_points)) {
                continue;
            }

            usize test_area = areaOf(points[0], points[i]);
            if (test_area > cur_area) {
                cur_a = points[0];
                cur_b = points[i];
                cur_area = test_area;
            }
        }

        point_a = cur_a;
        point_b = cur_b;
        max_area = cur_area;
    }

    *p_a = point_a;
    *p_b = point_b;

    return max_area;
}

void partTwo(Str input) {
#define BUF_LEN (1024 * 1024)
    char buf[BUF_LEN];

    Arena arena = fromBuffer(buf, BUF_LEN);

    usize points_len;
    Point *points;

    readPoints(&arena, input, &points_len, &points);

    Point a, b;
    usize largest_area = largestRectangleInInterior(&a, &b, points_len, points,
                                                    points_len, points);

    printf("Largest area = %zu\n", largest_area);

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
