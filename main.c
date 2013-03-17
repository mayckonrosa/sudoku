/*
    main - main.c

    Copyright (C) 2012 Matthias Ruester

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <err.h>

#define ABS(a) ((a) < 0 ? -(a) : (a))

struct number {
    char value,
         fixed;
};

struct sudoku {
    struct number arr[9][9];
};

struct sudoku *sudoku_alloc(void)
{
    struct sudoku *s;

    s = calloc(1, sizeof(*s));

    if (!s)
        err(EXIT_FAILURE, "sudoku_alloc: calloc");

    return s;
}

void sudoku_free(struct sudoku *s)
{
    free(s);
}

void sudoku_read(char *filename, struct sudoku *s)
{
    FILE *f;
    int x, y, c;

    f = fopen(filename, "r");

    if (!f)
        err(EXIT_FAILURE, "read_sudoku: %s: fopen", filename);

    x = y = 0;

    while ((c = fgetc(f)) != EOF) {
        if (isspace(c))
            continue;

        if (x == 9) {
            x = 0;
            y++;
        }

        if (y > 8) {
            fprintf(stderr, "sudoku_read: %s: warning: "
                    "skipping character '%c' in line %d\n",
                    filename, c, y);
            continue;
        }

        if (c == '_' || c == '0' || c == 'x' || c == '-' || c == '#') {
            s->arr[y][x].value = s->arr[y][x].fixed = 0;
            x++;
            continue;
        }

        if (c >= '1' && c <= '9') {
            s->arr[y][x].fixed = 1;
            s->arr[y][x].value = c - '0';
            x++;
            continue;
        }

        errx(EXIT_FAILURE, "sudoku_read: %s: unknown character '%c' (line %d)",
             filename, c, y + 1);
    }

    if (x < 8 || y < 8)
        errx(EXIT_FAILURE, "sudoku_read: %s: some fields are not defined",
             filename);

    if (fclose(f) == EOF)
        err(EXIT_FAILURE, "sudoku_read: %s: fclose", filename);
}

void sudoku_print(struct sudoku *s)
{
    int x, y;

    for (y = 0; y < 9; y ++) {
        if (y == 0)
            puts("+-----+-----+-----+");

        for (x = 0; x < 9; x++) {
            if (x == 0 || x == 3 || x == 6)
                printf("|");

            if (s->arr[y][x].value == 0)
                printf(" %s", (x == 2 || x == 5 || x == 8) ? "" : " ");
            else
                printf("%c%s", s->arr[y][x].value + '0',
                               (x == 2 || x == 5 || x == 8) ? "" : " ");

            if (x == 8)
                printf("|\n");
        }

        if (y == 2 || y == 5 || y == 8)
            puts("+-----+-----+-----+");
    }

    puts("");
}

int is_valid(struct sudoku *s)
{
    int faults, x, y, i, j;
    int count[9];

    faults = 0;

    for (y = 0; y < 9; y++) {
        memset(count, 0, 9 * sizeof(*count));

        for (x = 0; x < 9; x++) {
            if (s->arr[y][x].value == 0) {
                faults++;
                continue;
            }

            assert(s->arr[y][x].value >= 1 && s->arr[y][x].value <= 9);

            count[s->arr[y][x].value - 1]++;
        }

        for (x = 0; x < 9; x++)
            if (count[x] != 1)
                return 0;
    }

    for (x = 0; x < 9; x++) {
        memset(count, 0, 9 * sizeof(*count));

        for (y = 0; y < 9; y++) {
            if (s->arr[y][x].value == 0)
                continue;

            assert(s->arr[y][x].value >= 1 && s->arr[y][x].value <= 9);

            count[s->arr[y][x].value - 1]++;
        }

        for (y = 0; y < 9; y++)
            if (count[y] != 1)
                return 0;
    }

    for (y = 0; y < 9; y += 3) {
        for (x = 0; x < 9; x += 3) {
            memset(count, 0, 9 * sizeof(*count));

            for (i = 0; i < 3; i++)
                for (j = 0; j < 3; j++) {
                    if (s->arr[y + i][x + j].value == 0)
                        return 0;

                    assert(s->arr[y + i][x + j].value >= 1
                           && s->arr[y + i][x + j].value <= 9);

                    count[s->arr[y + i][x + j].value - 1]++;
                }

            for (i = 0; i < 9; i++)
                if (count[i] != 1)
                    return 0;
        }
    }

    return 1;
}

int is_filled(struct sudoku *s)
{
    int x, y;

    for (y = 0; y < 9; y++)
        for (x = 0; x < 9; x++)
            if (s->arr[y][x].value == 0)
                return 0;

    return 1;
}

void get_possible(struct sudoku *s, int x, int y, int possible[9])
{
    int i, j, k, l;

    memset(possible, 0, 9 * sizeof(*possible));

    /* row */
    for (i = 0; i < 9; i++) {
        if (s->arr[y][i].value == 0)
            continue;

        assert(s->arr[y][i].value >= 1 && s->arr[y][i].value <= 9);

        possible[s->arr[y][i].value - 1]++;
    }

    /* column */
    for (i = 0; i < 9; i++) {
        if (s->arr[i][x].value == 0)
            continue;

        assert(s->arr[i][x].value >= 1 && s->arr[i][x].value <= 9);

        possible[s->arr[i][x].value - 1]++;
    }

    /* block */
    i = (y / 3) * 3;
    j = (x / 3) * 3;

    for (k = 0; k < 3; k++) {
        for (l = 0; l < 3; l++) {
            if (s->arr[i + k][j + l].value == 0)
                continue;

            assert(s->arr[i + k][j + l].value >= 1
                   && s->arr[i + k][j + l].value <= 9);

            possible[s->arr[i + k][j + l].value - 1]++;
        }
    }

    for (i = 0; i < 9; i++) {
        assert(possible[i] <= 3);
        possible[i] = !possible[i];
    }
}

int solve_recursive(struct sudoku *s, int x, int y)
{
    int i, x_n, y_n;
    int possible[9];

    x_n = x;
    y_n = y;

    while ((x_n == x && y_n == y) || s->arr[y_n][x_n].fixed) {
        x_n++;

        if (x_n == 9) {
            y_n++;
            x_n = 0;
        }

        if (y_n == 9) {
            x_n = y_n = -1;
            break;
        }
    }

    if (s->arr[y][x].fixed)
        return x_n != -1 && solve_recursive(s, x_n, y_n);

    get_possible(s, x, y, possible);

    for (i = 0; i < 9; i++) {
        if (!possible[i])
            continue;

        s->arr[y][x].value = i + 1;

        if (is_filled(s))
            return 1;

        if (x_n != -1 && solve_recursive(s, x_n, y_n))
            return 1;
    }

    s->arr[y][x].value = 0;

    return 0;
}

int sudoku_solve(struct sudoku *s)
{
    return solve_recursive(s, 0, 0);
}

int all_fixed(struct sudoku *s)
{
    int x, y;

    for (y = 0; y < 9; y++)
        for (x = 0; x < 9; x++)
            if (!s->arr[y][x].fixed)
                return 0;

    return 1;
}

int main(int argc, char *argv[])
{
    struct sudoku *s;
    int i;

    s = sudoku_alloc();

    for (i = 1; i < argc; i++) {
        sudoku_read(argv[i], s);

        if (all_fixed(s)) {
            if (is_valid(s))
                fprintf(stderr, "%s: sudoku already solved\n", argv[i]);
            else
                fprintf(stderr, "%s: sudoku has no free field\n", argv[i]);

            continue;
        }

        puts("sudoku read:");
        sudoku_print(s);

        if (!sudoku_solve(s)) {
            fprintf(stderr, "%s: sudoku could not be solved\n", argv[i]);
            continue;
        }

        puts("sudoku solved:");
        sudoku_print(s);
    }

    sudoku_free(s);

    return 0;
}
