/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

int
lifegame()
{
    int N = 20;
    int map[2][N + 2][N + 2];
    int i, j, turn, k, tmp;

    memset(map, 0, sizeof(map));
    int init_map[22][22] = {
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0},
{0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0},
{0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0},
{0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0},
{0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0},
{0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0},
{0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0},
{0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0},
{0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0},
{0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0},
{0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0},
{0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0},
{0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0},
{0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0},
{0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0},
{0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0},
{0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0},
{0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0},
{0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    for (i = 1; i <= N; i++)
        for (j = 1; j <= N; j++)
            map[0][i][j] = init_map[i][j];

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

int
main()
{
    printf("calling into WASM function: %s\n", __FUNCTION__);

    int N = 30;
    int map[2][N + 2][N + 2];
    memset(map, 0, sizeof(map));

    lifegame();
    // for (int i = 0; i < 100000; i++) {
    //     printf("count: %d\n", i);
    //     sleep(1);
    // }
    printf("exit from WASM function: %s\n", __FUNCTION__);

    return 0;
}

