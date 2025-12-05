#include "files.cc"
#include "str.cc"

#include <string.h>

struct Dims {
    usize rows;
    usize cols;
};

Dims getDims(Str input) {
    bool first = true;
    usize rows = 0;
    usize cols = 0;

    Str line;
    Splitter line_spliiter = splitter('\n', input);
    while (line_spliiter.next(&line)) {
        if (line.len == 0) {
            break;
        }

        ++rows;
        if (first) {
            first = false;
            cols = line.len;
        } else {
            assert(line.len == cols);
        }
    }

    Dims dims = {};
    dims.rows = rows;
    dims.cols = cols;

    return dims;
}

struct Input {
    Dims dims;
    Str *rows;
    usize **counts;
};

void resetCounts(Dims dims, usize **counts) {
    usize row_size = dims.cols * sizeof(usize);
    for (usize i = 0; i < dims.rows; ++i) {
        usize *next_row = counts[i];
        memset(next_row, 0, row_size);

        next_row += dims.cols;
    }
}

void fillCounts(Input input) {
    for (usize row_idx = 0; row_idx < input.dims.rows; ++row_idx) {
        Str row_str = input.rows[row_idx];

        bool r_is_first = row_idx == 0;
        bool r_is_last = row_idx == input.dims.rows - 1;

        usize *p_row_counts = r_is_first ? nullptr : input.counts[row_idx - 1];
        usize *c_row_counts = input.counts[row_idx];
        usize *n_row_counts = r_is_last ? nullptr : input.counts[row_idx + 1];

        for (usize col_idx = 0; col_idx < input.dims.cols; ++col_idx) {
            bool c_is_first = col_idx == 0;
            bool c_is_last = col_idx == input.dims.cols - 1;

            if (row_str[col_idx] == '@') {
                // add count to neighbors
                if (!r_is_first) {
                    if (!c_is_first) {
                        ++p_row_counts[col_idx - 1];
                    }
                    ++p_row_counts[col_idx];
                    if (!c_is_last) {
                        ++p_row_counts[col_idx + 1];
                    }
                }

                if (!c_is_first) {
                    ++c_row_counts[col_idx - 1];
                }
                if (!c_is_last) {
                    ++c_row_counts[col_idx + 1];
                }

                if (!r_is_last) {
                    if (!c_is_first) {
                        ++n_row_counts[col_idx - 1];
                    }
                    ++n_row_counts[col_idx];
                    if (!c_is_last) {
                        ++n_row_counts[col_idx + 1];
                    }
                }
            }
        }
    }
}

void removeRemovable(Input input_a, Input input_b) {
    assert(input_a.dims.rows == input_b.dims.rows);
    assert(input_a.dims.cols == input_b.dims.cols);

    for (usize row_idx = 0; row_idx < input_a.dims.rows; ++row_idx) {
        Str row_a = input_a.rows[row_idx];
        Str row_b = input_b.rows[row_idx];

        usize *counts_a = input_a.counts[row_idx];

        bool r_is_first = row_idx == 0;
        bool r_is_last = row_idx == input_a.dims.rows - 1;

        usize *pb_row_counts =
            r_is_first ? nullptr : input_b.counts[row_idx - 1];
        usize *cb_row_counts = input_b.counts[row_idx];
        usize *nb_row_counts =
            r_is_last ? nullptr : input_b.counts[row_idx + 1];

        for (usize col_idx = 0; col_idx < input_a.dims.cols; ++col_idx) {
            bool c_is_first = col_idx == 0;
            bool c_is_last = col_idx == input_a.dims.cols - 1;

            if (row_a[col_idx] == '@' && counts_a[col_idx] < 4) {
                row_b[col_idx] = '.';

                // remove count from neighbors
                if (!r_is_first) {
                    if (!c_is_first) {
                        --pb_row_counts[col_idx - 1];
                    }
                    --pb_row_counts[col_idx];
                    if (!c_is_last) {
                        --pb_row_counts[col_idx + 1];
                    }
                }

                if (!c_is_first) {
                    --cb_row_counts[col_idx - 1];
                }
                if (!c_is_last) {
                    --cb_row_counts[col_idx + 1];
                }

                if (!r_is_last) {
                    if (!c_is_first) {
                        --nb_row_counts[col_idx - 1];
                    }
                    --nb_row_counts[col_idx];
                    if (!c_is_last) {
                        --nb_row_counts[col_idx + 1];
                    }
                }
            }
        }
    }
}

void copyStringsFromBToA(Input input_a, Input input_b) {
    for (usize row_idx = 0; row_idx < input_b.dims.rows; ++row_idx) {
        Str a = input_a.rows[row_idx];
        Str b = input_b.rows[row_idx];
        memcpy(a.ptr, b.ptr, b.len);
    }
}

void copyCountsFromBToA(Input input_a, Input input_b) {
    for (usize row_idx = 0; row_idx < input_a.dims.rows; ++row_idx) {
        usize *a = input_a.counts[row_idx];
        usize *b = input_b.counts[row_idx];
        memcpy(a, b, input_b.dims.cols * sizeof(*b));
    }
}

usize totalCountMovable(Input input) {
    usize total = 0;
    for (usize row_idx = 0; row_idx < input.dims.rows; ++row_idx) {
        Str row = input.rows[row_idx];
        usize *counts = input.counts[row_idx];

        for (usize col_idx = 0; col_idx < input.dims.cols; ++col_idx) {
            if (row[col_idx] == '@' && counts[col_idx] < 4) {
                ++total;
            }
        }
    }
    return total;
}

Input readInput(Str input_str, void **pbuf, usize buf_len) {
    Dims dims = getDims(input_str);

    usize row_size = dims.cols * sizeof(usize);
    usize str_sizes = dims.rows * sizeof(Str);
    usize count_ptr_size = dims.rows * sizeof(usize *);
    usize count_arr_sizes = dims.rows * row_size;
    usize total = str_sizes + count_ptr_size + count_arr_sizes;
    assert(total <= buf_len);

    void *buf = *pbuf;
    *pbuf = (char *)buf + total;

    Str *next_line = (Str *)buf;
    usize **next_row_ptr = (usize **)((char *)buf + str_sizes);
    usize *next_row = (usize *)((char *)buf + str_sizes + count_ptr_size);

    Splitter line_spliiter = splitter('\n', input_str);
    for (usize i = 0; i < dims.rows; ++i) {
        assert(line_spliiter.next(next_line));
        *next_row_ptr = next_row;

        next_line += 1;
        next_row_ptr += 1;
        next_row += dims.cols;
    }

    Input input = {};
    input.dims = dims;
    input.rows = (Str *)buf;
    input.counts = (usize **)((char *)buf + str_sizes);

    resetCounts(input.dims, input.counts);

    return input;
}

Input copyStructure(Input input, void **pbuf, usize buf_len) {
    Dims dims = input.dims;

    usize row_size = dims.cols * sizeof(usize);
    usize str_ptr_size = dims.rows * sizeof(Str);
    usize str_arr_sizes = dims.rows * dims.cols * sizeof(char);
    usize count_ptr_size = dims.rows * sizeof(usize *);
    usize count_arr_sizes = dims.rows * row_size;
    usize total =
        str_ptr_size + str_arr_sizes + count_ptr_size + count_arr_sizes;
    assert(total <= buf_len);

    void *buf = *pbuf;
    *pbuf = (char *)buf + total;

    Str *next_str_ptr = (Str *)buf;
    char *next_str = (char *)((char *)buf + str_ptr_size);
    usize **next_row_ptr =
        (usize **)((char *)buf + str_ptr_size + str_arr_sizes);
    usize *next_row =
        (usize *)((char *)buf + str_ptr_size + str_arr_sizes + count_ptr_size);
    for (usize i = 0; i < dims.rows; ++i) {
        Str cur_line = input.rows[i];

        Str str_copy = {};
        str_copy.len = cur_line.len;
        str_copy.ptr = next_str;
        memcpy(next_str, cur_line.ptr, cur_line.len);

        *next_str_ptr = str_copy;
        *next_row_ptr = next_row;

        next_str_ptr += 1;
        next_str += cur_line.len;
        next_row_ptr += 1;
        next_row += dims.cols;
    }

    Input dupe = {};
    dupe.dims = input.dims;
    dupe.rows = (Str *)buf;
    dupe.counts = (usize **)((char *)buf + str_ptr_size + str_arr_sizes);

    resetCounts(dupe.dims, dupe.counts);

    return dupe;
}

void partOne(Str input) {
#define BUF_LEN (1024 * 1024)
    char buf_[BUF_LEN];

    void *buf = buf_;

    Input board = readInput(input, &buf, BUF_LEN);
    fillCounts(board);
    usize valid_count = totalCountMovable(board);

    printf("Valid roll count = %zu\n", valid_count);

#undef BUF_LEN
}

void partTwo(Str input) {
#define BUF_LEN (1024 * 1024)
    char buf_[BUF_LEN];

    void *buf = buf_;

    Input board_a = readInput(input, &buf, BUF_LEN);
    Input board_b =
        copyStructure(board_a, &buf, BUF_LEN - ((char *)buf - (char *)buf_));

    Input *cur_a = &board_a;
    Input *cur_b = &board_b;

    fillCounts(*cur_a);
    fillCounts(*cur_b);

    usize total = 0;
    usize last = 0;
    do {
        usize next = totalCountMovable(*cur_a);
        removeRemovable(*cur_a, *cur_b);

        copyStringsFromBToA(*cur_a, *cur_b);
        copyCountsFromBToA(*cur_a, *cur_b);

        total += next;
        last = next;
        {
            Input *tmp = cur_a;
            cur_a = cur_b;
            cur_b = tmp;
        }
    } while (last != 0);

    printf("Total removable = %zu\n", total);

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
