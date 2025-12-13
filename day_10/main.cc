#include "arena.cc"
#include "files.cc"
#include "str.cc"

#include <assert.h>
#include <string.h>

struct MachineOne {
    usize button_len;
    usize target;

    usize button_count;
    usize *buttons;
};

struct MachineTwo {
    usize button_len;
    usize *joltages;

    usize button_count;
    usize **buttons;
};

usize countButtons(Str line) {
    Splitter pieces = splitter(' ', line);
    assert(pieces.next(nullptr));

    Str piece;
    usize count = 0;
    while (pieces.next(&piece)) {
        if (piece.len == 0) {
            continue;
        }
        if (piece[0] == '{') {
            break;
        }

        ++count;
    }
    return count;
}

#define OFF '.'
#define ON '#'

void targetFrom(Str str, usize *pbutton_len, usize *ptarget) {
    assert(str.len > 2);
    assert(str[0] == '[');
    assert(str[str.len - 1] == ']');

    Str target_str = str.slice(1, str.len - 1);

    usize button_len = 0;
    usize target = 0;

    for (char c : target_str) {
        button_len += 1;
        target <<= 1;

        switch (c) {
        case OFF: {
            target += 0;
        } break;
        case ON: {
            target += 1;
        } break;
        default: {
            assert(0 && "Unreachable");
        } break;
        }
    }

    *pbutton_len = button_len;
    if (ptarget != nullptr) {
        *ptarget = target;
    }
}

usize buttonFrom(Str str, usize button_len) {
    assert(str.len > 2);
    assert(str[0] == '(');
    assert(str[str.len - 1] == ')');

    Str numbers_str = str.slice(1, str.len - 1);

    usize button = 0;

    Str number_str;
    Splitter numbers = splitter(',', numbers_str);
    while (numbers.next(&number_str)) {
        if (number_str.len == 0) {
            continue;
        }

        usize num = strtoul(number_str.ptr, nullptr, 10);
        usize to_add = 1 << (button_len - 1 - num);

        button += to_add;
    }

    return button;
}

void readButtonsOne(Arena *arena, Str line, Splitter *pieces, usize button_len,
                    usize *plen, usize **pbuttons) {
    usize button_count = countButtons(line);
    usize *buttons = pushArr<usize>(arena, button_count);

    usize button_idx = 0;
    usize *cur_button = buttons;

    Str piece;

    Splitter backup = *pieces;
    while (pieces->next(&piece)) {
        if (piece.len == 0) {
            continue;
        }
        if (piece[0] == '{') {
            break;
        }

        assert(button_idx < button_count);
        *cur_button = buttonFrom(piece, button_len);

        ++button_idx;
        ++cur_button;

        backup = *pieces;
    }
    // reset to before reading the joltage string
    *pieces = backup;

    *plen = button_count;
    *pbuttons = buttons;
}

MachineOne readMachineOne(Arena *arena, Str line) {
    Splitter pieces = splitter(' ', line);

    Str target_str;
    assert(pieces.next(&target_str));

    usize button_len = 0;
    usize target = 0;
    targetFrom(target_str, &button_len, &target);

    usize button_count = 0;
    usize *buttons = nullptr;
    readButtonsOne(arena, line, &pieces, button_len, &button_count, &buttons);

    MachineOne machine = {};
    machine.button_len = button_len;
    machine.target = target;
    machine.button_count = button_count;
    machine.buttons = buttons;

    return machine;
}

void fillTableOne(usize button_max, usize *combos, usize start_state,
                  usize button_count, usize *buttons) {
    usize next_count = combos[start_state] + 1;

    for (usize i = 0; i < button_count; ++i) {
        usize button_i = buttons[i];
        usize next_state = start_state ^ button_i;
        if (next_count < combos[next_state]) {
            combos[next_state] = next_count;

            fillTableOne(button_max, combos, next_state, button_count, buttons);
        }
    }
}

void partOne(Str input) {
#define BUF_LEN (1024 * 1024)
    char buf[BUF_LEN];

    Arena arena = fromBuffer(buf, BUF_LEN);

    usize total = 0;

    Str line;
    Splitter lines = splitter('\n', input);
    while (lines.next(&line)) {
        if (line.len == 0) {
            continue;
        }

        Arena backup = arena;
        {
            MachineOne machine = readMachineOne(&arena, line);

            usize button_max = 1 << machine.button_len;
            usize *combos = pushArr<usize>(&arena, button_max);
            for (usize i = 0; i < button_max; ++i) {
                combos[i] = 0 - 1;
            }

            combos[0] = 0;
            fillTableOne(button_max, combos, 0, machine.button_count,
                         machine.buttons);

            usize min_steps = combos[machine.target];
            total += min_steps;
        }
        arena = backup;
    }

    printf("total min steps = %zu\n", total);

#undef BUF_LEN
}

void readButtonTwo(Str piece, usize button_len, usize *cur_button) {
    assert(piece.len > 0);
    assert(piece[0] == '(');
    assert(piece[piece.len - 1] == ')');

    Str nums_str = piece.slice(1, piece.len - 1);

    memset(cur_button, 0, button_len * sizeof(usize));

    Str num;
    Splitter nums = splitter(',', nums_str);
    while (nums.next(&num)) {
        if (num.len == 0) {
            continue;
        }

        usize button_num = strtoul(num.ptr, nullptr, 10);
        assert(button_num < button_len);

        cur_button[button_num] = 1;
    }
}

usize countItems(usize button_len, usize *button) {
    usize count = 0;
    for (usize i = 0; i < button_len; ++i) {
        if (button[i]) {
            ++count;
        }
    }
    return count;
}

void sortButtons(usize button_len, usize button_count, usize **buttons) {
    if (button_count <= 1) {
        return;
    }

    usize max_idx = 0;
    usize max_count = countItems(button_len, buttons[0]);

    for (usize i = 1; i < button_count; ++i) {
        usize test_count = countItems(button_len, buttons[i]);
        if (test_count > max_count) {
            max_idx = i;
            max_count = test_count;
        }
    }

    // move max to front
    {
        usize *tmp = buttons[0];
        buttons[0] = buttons[max_idx];
        buttons[max_idx] = tmp;
    }

    // sort the rest
    sortButtons(button_len, button_count - 1, buttons + 1);
}

void readButtonsTwo(Arena *arena, Str line, Splitter *pieces, usize button_len,
                    usize *plen, usize ***pbuttons) {
    usize button_count = countButtons(line);

    usize **buttons = pushArr<usize *>(arena, button_count);
    for (usize i = 0; i < button_count; ++i) {
        buttons[i] = pushArr<usize>(arena, button_len);
    }

    usize **cur_button = buttons;

    Str piece;
    Splitter backup = *pieces;
    while (pieces->next(&piece)) {
        if (piece.len == 0) {
            continue;
        }
        if (piece[0] == '{') {
            break;
        }

        readButtonTwo(piece, button_len, *cur_button);
        ++cur_button;

        backup = *pieces;
    }

    sortButtons(button_len, button_count, buttons);

    // reset to before reading the joltage string
    *pieces = backup;

    *plen = button_count;
    *pbuttons = buttons;
}

void readJoltages(Arena *arena, Str joltage_str, usize button_len,
                  usize **pjoltages) {
    usize *joltages = pushArr<usize>(arena, button_len);

    assert(joltage_str.len > 0);
    assert(joltage_str[0] == '{');
    assert(joltage_str[joltage_str.len - 1] == '}');

    Str counts_str = joltage_str.slice(1, joltage_str.len - 1);

    usize *cur_joltage = joltages;

    Str count;
    Splitter counts = splitter(',', counts_str);
    while (counts.next(&count)) {
        if (count.len == 0) {
            continue;
        }

        usize count_num = strtoul(count.ptr, nullptr, 10);
        *cur_joltage = count_num;

        ++cur_joltage;
    }

    *pjoltages = joltages;
}

MachineTwo readMachineTwo(Arena *arena, Str line) {
    Splitter pieces = splitter(' ', line);

    Str target_str;
    assert(pieces.next(&target_str));

    usize button_len = 0;
    targetFrom(target_str, &button_len, nullptr);

    usize button_count = 0;
    usize **buttons = nullptr;
    readButtonsTwo(arena, line, &pieces, button_len, &button_count, &buttons);

    Str joltage_str;
    assert(pieces.next(&joltage_str));

    usize *joltages;
    readJoltages(arena, joltage_str, button_len, &joltages);

    MachineTwo machine = {};
    machine.button_len = button_len;
    machine.joltages = joltages;
    machine.button_count = button_count;
    machine.buttons = buttons;

    return machine;
}

void add(usize len, usize *target, usize *button) {
    for (usize i = 0; i < len; ++i) {
        target[i] += button[i];
    }
}

bool trySubtract(usize len, usize *target, usize *button) {
    bool successful = true;
    usize i = 0;
    while (i < len) {
        if (target[i] < button[i]) {
            successful = false;
            break;
        }
        target[i] -= button[i];

        ++i;
    }

    if (!successful) {
        add(i, target, button);
    }

    return successful;
}

bool allZeros(usize len, usize *vals) {
    while (len-- > 0) {
        if (*(vals++) != 0) {
            return false;
        }
    }
    return true;
}

struct HashItem {
    bool occupied;

    // key
    usize *target;

    // value
    bool solvable;
    usize min_steps;
};

struct HashList {
    usize capacity;
    usize count;

    HashItem *items;
};

usize getHashKey(usize button_len, usize *button, usize buckets) {
    usize hash = 1;
    for (usize i = 0; i < button_len; ++i) {
        hash = (31 * hash) + (7 * button[i]);
    }
    return hash % buckets;
}

bool isItem(usize button_len, usize *target, HashItem *item) {
    assert(item != nullptr);
    assert(item->occupied);

    if (target == item->target) {
        return true;
    }

    usize *item_target = item->target;
    while (button_len-- > 0) {
        if (*(target++) != *(item_target++)) {
            return false;
        }
    }
    return true;
}

HashItem *getItem(usize button_len, usize *button, HashList *list) {
    HashItem *result = nullptr;

    usize base = getHashKey(button_len, button, list->capacity);
    for (usize i = 0; i < list->capacity; ++i) {
        usize key = (base + i) % list->capacity;
        HashItem *item = list->items + key;
        if (!item->occupied) {
            break;
        } else if (item->occupied && isItem(button_len, button, item)) {
            result = item;
            break;
        }
    }

    return result;
}

void _insertItem(usize button_len, HashItem item, HashList *list,
                 bool allow_recurse = true) {
    if (button_len == 4) {
        usize mycheck[] = {3, 1, 4, 3};
        HashItem test = {};
        test.occupied = true;
        test.target = mycheck;

        if (isItem(button_len, item.target, &test)) {
            printf("Adding the one\n");
        }
    }

    HashItem *prior_entry = getItem(button_len, item.target, list);
    assert(prior_entry == nullptr);

    if (list->count > list->capacity / 4 * 3) {
        assert(allow_recurse);

        usize prev_cap = list->capacity;
        HashItem *prev_items = list->items;

        usize new_cap = list->capacity * 2;
        HashItem *new_items = new HashItem[new_cap];
        assert(new_items != nullptr);
        memset(new_items, 0, new_cap * sizeof(HashItem));

        printf("Resizing to %zu\n", new_cap);

        list->capacity = new_cap;
        list->count = 0;
        list->items = new_items;

        for (usize i = 0; i < prev_cap; ++i) {
            HashItem prev_item = prev_items[i];
            if (prev_item.occupied) {
                _insertItem(button_len, prev_item, list, false);
            }
        }

        delete[] prev_items;
    }

    usize base = getHashKey(button_len, item.target, list->capacity);
    for (usize i = 0; i < list->capacity; ++i) {
        usize key = (base + i) % list->capacity;
        HashItem *place = list->items + key;
        if (!place->occupied) {
            *place = item;

            ++list->count;
            break;
        }
    }
}

void insertItem(usize button_len, usize *target, bool solvable, usize min_steps,
                HashList *list) {
    HashItem item = {};
    item.occupied = true;
    item.target = target;
    item.solvable = solvable;
    item.min_steps = min_steps;

    _insertItem(button_len, item, list);
}

bool minResolve(Arena *arena, HashList *results, usize button_len,
                usize *target, usize button_count, usize **buttons,
                usize *min_resolve) {
    HashItem *result_item = getItem(button_len, target, results);
    if (result_item != nullptr) {
        *min_resolve = result_item->min_steps;

        return result_item->solvable;
    }

    if (allZeros(button_len, target)) {
        usize *res_target = pushArr<usize>(arena, button_len);
        memcpy(res_target, target, button_len * sizeof(usize));

        insertItem(button_len, res_target, true, 0, results);

        *min_resolve = 0;

        return true;
    }

    usize cur_min = 0 - 1;
    bool any_solution = false;

    for (usize i = 0; i < button_count; ++i) {
        usize *button = buttons[i];

        if (trySubtract(button_len, target, button)) {
            usize test_min = 0;
            bool has_solution = minResolve(arena, results, button_len, target,
                                           button_count, buttons, &test_min);

            add(button_len, target, button);

            if (!has_solution) {
                continue;
            }

            test_min += 1; // for the above subtraction

            any_solution = true;
            if (test_min < cur_min) {
                cur_min = test_min;
            }
        }
    }

    usize *res_target = pushArr<usize>(arena, button_len);
    memcpy(res_target, target, button_len * sizeof(usize));

    insertItem(button_len, res_target, any_solution, cur_min, results);

    *min_resolve = cur_min;

    return any_solution;
}

void partTwo(Str input) {
#define BUF_LEN ((usize)1024 * 1024 * 1024 * 1024)
    char *bufOne = new char[BUF_LEN];

    Arena arena = fromBuffer(bufOne, BUF_LEN);

    usize total = 0;

    HashList list = {};
    list.capacity = 1024;
    list.items = new HashItem[list.capacity];
    memset(list.items, 0, list.capacity * sizeof(HashItem));

    Str line;
    Splitter lines = splitter('\n', input);
    while (lines.next(&line)) {
        if (line.len == 0) {
            continue;
        }

        list.count = 0;
        memset(list.items, 0, list.capacity * sizeof(HashItem));

        Arena backup = arena;
        {
            MachineTwo machine = readMachineTwo(&arena, line);

            for (usize i = 0; i < machine.button_count; ++i) {
                printf("(");
                for (usize j = 0; j < machine.button_len; ++j) {
                    printf("%zu,", machine.buttons[i][j]);
                }
                printf("),\n");
            }
            printf("{");
            for (usize i = 0; i < machine.button_len; ++i) {
                printf("%zu,", machine.joltages[i]);
            }
            printf("}\n");

            usize min_resolve = 0;
            bool solvable =
                minResolve(&arena, &list, machine.button_len, machine.joltages,
                           machine.button_count, machine.buttons, &min_resolve);

            assert(solvable);

            printf("  %zu\n\n", min_resolve);

            total += min_resolve;
        }
        arena = backup;
    }

    printf("total = %zu\n", total);

    delete[] list.items;
    delete[] bufOne;
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
