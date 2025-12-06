#include "arena.cc"
#include "files.cc"
#include "str.cc"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

inline usize ulmax(usize a, usize b) { return a > b ? a : b; }

struct Row {
    Row *next;
    usize *vals;
};

enum Op {
    Op_Sum = '+',
    Op_Multiply = '*',
};

usize unitFor(Op op) {
    switch (op) {
    case Op_Sum:
        return 0;
    case Op_Multiply:
        return 1;
    }
}

struct Operations {
    Op *ops;
};

struct ProblemSet {
    Row *rows;
    Operations operations;
    usize cols;
};

ProblemSet readProblemSetPartOne(Arena *arena, Str input) {
    ProblemSet res = {};

    Str line;
    bool first = true;

    Splitter lines = splitter('\n', input);
    while (lines.next(&line)) {
        assert(line.len > 0);

        if (line[0] == Op_Sum || line[0] == Op_Multiply) {
            assert(!first);

            usize *nums = pushArr<usize>(arena, res.cols);
            Op *ops = pushArr<Op>(arena, res.cols);

            Row *cur_row = push<Row>(arena);
            *cur_row = {};
            cur_row->next = res.rows;
            cur_row->vals = nums;
            res.rows = cur_row;

            res.operations = {};
            res.operations.ops = ops;

            Str op_str;
            Splitter spaces = splitter(' ', line);
            for (usize i = 0; i < res.cols; ++i) {
                while (spaces.next(&op_str) && op_str.len == 0)
                    ;

                Op op = (Op)op_str[0];
                assert(op == Op_Sum || op == Op_Multiply);

                nums[i] = unitFor(op);
                ops[i] = op;
            }

            while (true) {
                bool had_more = spaces.next(&op_str);
                if (had_more) {
                    assert(op_str.len == 0);
                } else {
                    break;
                }
            }
        } else {
            if (first) {
                first = false;

                res.cols = 0;

                Str number_str;
                Splitter spaces = splitter(' ', line);
                while (spaces.next(&number_str)) {
                    if (number_str.len > 0) {
                        ++res.cols;
                    }
                }
            }

            usize *nums = pushArr<usize>(arena, res.cols);

            Row *cur_row = push<Row>(arena);
            *cur_row = {};
            cur_row->next = res.rows;
            cur_row->vals = nums;
            res.rows = cur_row;

            Str number_str;
            Splitter spaces = splitter(' ', line);
            for (usize i = 0; i < res.cols; ++i) {
                while (spaces.next(&number_str) && number_str.len == 0)
                    ;

                usize number = strtoul(number_str.ptr, nullptr, 10);
                nums[i] = number;
            }

            while (true) {
                bool had_more = spaces.next(&number_str);
                if (had_more) {
                    assert(number_str.len == 0);
                } else {
                    break;
                }
            }
        }
    }

    return res;
}

void partOne(Str input) {
#define BUF_LEN (1024 * 1024)
    char buf[BUF_LEN];

    Arena arena = fromBuffer(buf, BUF_LEN);
    ProblemSet problem_set = readProblemSetPartOne(&arena, input);

    Row *values = problem_set.rows;
    Row *next_values = values->next;
    while (next_values != nullptr) {
        for (usize i = 0; i < problem_set.cols; ++i) {
            Op op = problem_set.operations.ops[i];

            switch (op) {
            case Op_Sum: {
                values->vals[i] += next_values->vals[i];
            } break;
            case Op_Multiply: {
                values->vals[i] *= next_values->vals[i];
            } break;
            }
        }

        next_values = next_values->next;
    }

    usize total = 0;
    for (usize i = 0; i < problem_set.cols; ++i) {
        total += values->vals[i];
    }

    printf("Results total = %zu\n", total);

#undef BUF_LEN
}

struct Dims {
    usize rows;
    usize cols;
};

struct MappedInput {
    Dims dims;
    Str *lines;
};

Dims getDims(Str input) {
    Dims res = {};

    Str line;
    Splitter lines = splitter('\n', input);
    while (lines.next(&line)) {
        if (line.len == 0) {
            break;
        }

        res.rows = res.rows + 1;
        res.cols = ulmax(res.cols, line.len);
    }

    return res;
}

MappedInput mapInput(Arena *arena, Str input) {
    Dims dims = getDims(input);
    Dims new_dims = {};
    new_dims.rows = dims.cols;
    new_dims.cols = dims.rows;

    Str *old_strs = pushArr<Str>(arena, dims.rows);
    Splitter lines = splitter('\n', input);
    for (usize i = 0; i < dims.rows; ++i) {
        assert(lines.next(old_strs + i));
    }

    Str *new_strs = pushArr<Str>(arena, new_dims.rows);
    char *rotated_str =
        pushArr<char>(arena, new_dims.rows * (new_dims.cols + 1) + 1);
    rotated_str[new_dims.rows * (new_dims.cols + 1)] = '\0';

    char *cur_out_row = rotated_str;
    for (usize i = 0; i < new_dims.rows; ++i) {
        Str *new_str_i = new_strs + i;
        *new_str_i = {};
        new_str_i->len = new_dims.cols;
        new_str_i->ptr = cur_out_row;

        for (usize j = 0; j < new_dims.cols; ++j) {
            Str old_str = old_strs[j];

            usize old_char_idx = new_dims.rows - 1 - i;
            char old_char =
                old_char_idx < old_str.len ? old_str[old_char_idx] : ' ';

            (*new_str_i)[j] = old_char;
        }
        cur_out_row[new_dims.cols] = '\n';

        cur_out_row += new_dims.cols + 1;
    }

    MappedInput res = {};
    res.dims = new_dims;
    res.lines = new_strs;

    return res;
}

struct Param {
    Param *next;
    usize val;
};

void pushNumber(Arena *arena, usize num, Param **params, Param **free_list) {
    Param *cur = nullptr;
    if (*free_list != nullptr) {
        cur = *free_list;
        *free_list = (*free_list)->next;
    } else {
        cur = push<Param>(arena);
    }

    *cur = {};
    cur->next = *params;
    cur->val = num;

    *params = cur;
}

usize pop(Param **params, Param **free_list) {
    assert(*params != nullptr);

    Param *head = *params;
    *params = (*params)->next;

    usize res = head->val;

    *head = {};
    head->next = *free_list;

    *free_list = head;

    return res;
}

void partTwo(Str input) {
#define BUF_LEN (1024 * 1024)
    char buf[BUF_LEN];

    Arena arena = fromBuffer(buf, BUF_LEN);
    MappedInput mapped = mapInput(&arena, input);

    Param *cur_param = nullptr;
    Param *free_list = nullptr;

    usize total = 0;
    bool expect_empty_line = false;
    for (usize i = 0; i < mapped.dims.rows; ++i) {
        Str line = mapped.lines[i];
        if (expect_empty_line) {
            for (char c : line) {
                assert(c == ' ');
            }
            expect_empty_line = false;
        } else {
            assert(isspace(line[0]) || isdigit(line[0]));

            usize num = strtoul(line.ptr, nullptr, 10);
            pushNumber(&arena, num, &cur_param, &free_list);

            char last = line[line.len - 1];
            if (last == Op_Sum || last == Op_Multiply) {
                usize calc_res = unitFor((Op)last);

                while (cur_param != nullptr) {
                    usize next = pop(&cur_param, &free_list);

                    switch ((Op)last) {
                    case Op_Sum: {
                        calc_res += next;
                    } break;
                    case Op_Multiply: {
                        calc_res *= next;
                    } break;
                    }
                }

                total += calc_res;
                expect_empty_line = true;
            }
        }
    }

    printf("Results total = %zu\n", total);

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
