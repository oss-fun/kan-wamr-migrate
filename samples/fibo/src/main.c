
/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "wasm_export.h"
#include "bh_read_file.h"
#include "bh_getopt.h"

void
print_usage(void)
{
    fprintf(stdout, "Options:\r\n");
    fprintf(stdout, "  -f [path of wasm file] \n");
}

int
main(int argc, char *argv_main[])
{
    static char global_heap_buf[512 * 1024 * 1024];
    char *buffer, error_buf[128];
    int opt;
    char *wasm_path = NULL;

    wasm_module_t module = NULL;
    wasm_module_inst_t module_inst = NULL;
    wasm_exec_env_t exec_env = NULL;
    uint32 buf_size, stack_size = 8092 * 1024, heap_size = 8092;
    wasm_function_inst_t func = NULL;
    wasm_function_inst_t func2 = NULL;
    char *native_buffer = NULL;
    uint32_t wasm_buffer = 0;
    uint32_t fibo_n = 10;
    bool restore_flag = false;
    char *img_dir = "./";

    RuntimeInitArgs init_args;
    memset(&init_args, 0, sizeof(RuntimeInitArgs));

    while ((opt = getopt(argc, argv_main, "hrf:n:")) != -1) {
        switch (opt) {
            case 'f':
                wasm_path = optarg;
                break;
            case 'r':
                restore_flag = true;
                break;
            case 'n':
                fibo_n = atoi(optarg);
                break;

            case 'h':
                print_usage();
                return 0;
            case '?':
                print_usage();
                return 0;
        }
    }
    if (optind == 1) {
        print_usage();
        return 0;
    }

    // Define an array of NativeSymbol for the APIs to be exported.
    // Note: the array must be static defined since runtime
    //            will keep it after registration
    // For the function signature specifications, goto the link:
    // https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md

    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
    init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

    if (!wasm_runtime_full_init(&init_args)) {
        printf("Init runtime environment failed.\n");
        return -1;
    }

    buffer = bh_read_file_to_buffer(wasm_path, &buf_size);

    if (!buffer) {
        printf("Open wasm app file [%s] failed.\n", wasm_path);
        goto fail;
    }

    module = wasm_runtime_load(buffer, buf_size, error_buf, sizeof(error_buf));
    if (!module) {
        printf("Load wasm module failed. error: %s\n", error_buf);
        goto fail;
    }

    module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                           error_buf, sizeof(error_buf));

    if (!module_inst) {
        printf("Instantiate wasm module failed. error: %s\n", error_buf);
        goto fail;
    }

    exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
    if (!exec_env) {
        printf("Create wasm execution environment failed.\n");
        goto fail;
    }

    uint32 argv[1];
    uint32 num = fibo_n;
    argv[0] = num;

    if (!(func =
              wasm_runtime_lookup_function(module_inst, "fibonacci", NULL))) {
        printf("The fibonacci wasm function is not found.\n");
        goto fail;
    }

    if (restore_flag) {
        if (!wasm_runtime_restore_wasm(exec_env, func, 1, argv)) {
            printf("restore wasm function fibonacci failed. %s\n",
                   wasm_runtime_get_exception(module_inst));
            goto fail;
        }
    }
    else {
        // pass 4 elements for function arguments
        if (!wasm_runtime_call_wasm(exec_env, func, 1, argv)) {
            printf("call wasm function fibonacci failed. %s\n",
                   wasm_runtime_get_exception(module_inst));
            goto fail;
        }
    }

    uint32 ret_val;
    memcpy(&ret_val, argv, sizeof(uint32));
    printf("Native finished calling wasm function fibonacci(), returned a "
           "fibonacci[%d]: %d\n",
           num, ret_val);

fail:
    if (exec_env)
        wasm_runtime_destroy_exec_env(exec_env);
    if (module_inst) {
        if (wasm_buffer)
            wasm_runtime_module_free(module_inst, wasm_buffer);
        wasm_runtime_deinstantiate(module_inst);
    }
    if (module)
        wasm_runtime_unload(module);
    if (buffer)
        BH_FREE(buffer);
    wasm_runtime_destroy();
    return 0;
}
