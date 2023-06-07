/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int
lifegame()
{
    int N = 10;
    int map[2][N + 2][N + 2];
    int i, j, turn, k, tmp;

    memset(map, 0, sizeof(map));
    for (i = 1; i <= N; i++)
        for (j = 1; j <= N; j++)
            map[0][i][j] = (i * j) % 2;

    turn = 0;
    for (k = 0;; k++) {
        // printf("\033[2J");
        printf("k:%d\n", k);
        for (i = 1; i <= N; i++) {
            for (j = 1; j <= N; j++)
                putchar(map[turn][i][j] ? '#' : '.');
            puts("");
        }
        puts("");

        for (i = 1; i <= N; i++) {
            for (j = 1; j <= N; j++) {
                tmp = map[turn][i + 1 > N ? 1 : i + 1][j]
                      + map[turn][i - 1 ?: N][j]
                      + map[turn][i][j + 1 > N ? 1 : j + 1]
                      + map[turn][i][j - 1 ?: N]
                      + map[turn][i + 1 > N ? 1 : i + 1][j + 1 > N ? 1 : j + 1]
                      + map[turn][i + 1 > N ? 1 : i + 1][j - 1 ?: N]
                      + map[turn][i - 1 ?: N][j + 1 > N ? 1 : j + 1]
                      + map[turn][i - 1 ?: N][j - 1 ?: N];
                if (map[turn][i][j]) {
                    map[turn ^ 1][i][j] = (tmp == 2 || tmp == 3);
                }
                else {
                    map[turn ^ 1][i][j] = (tmp == 3);
                }
            }
        }

        turn ^= 1;
    }

    return 0;
}

float
generate_float(int iteration, double seed1, float seed2)
{
    printf("calling into WASM function: %s\n", __FUNCTION__);

    lifegame();
    // for (int i = 0; i < 100000; i++) {
    //     printf("count: %d\n", i);
    // }
    printf("exit from WASM function: %s\n", __FUNCTION__);

    return 0;
}

int main() {
    generate_float(0,0,0);
}
