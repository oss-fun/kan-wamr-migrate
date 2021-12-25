/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

unsigned long
fibonacci(int n)
{
    unsigned long f;

    if (n <= 2) {
        return 1L;
    }
    else {
        f = fibonacci(n - 1) + fibonacci(n - 2);
        return f;
    }
}

int
main(int argc,char *argv[])
{
    int n;
    if(argc>1){
        n = atoi(argv[1]);
    }

    printf("Native fibonacci[%d]: %ld\n", n, fibonacci(n));

    return 0;
}
