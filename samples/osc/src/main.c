/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "wasm_export.h"
#include "bh_read_file.h"
#include "bh_getopt.h"


static int app_argc;
static char **app_argv;

void
print_usage(void)
{
    fprintf(stdout, "Options:\r\n");
    fprintf(stdout, "  -f [path of wasm file] \n");
}

static void *
app_instance_main(wasm_module_inst_t module_inst)
{
    const char *exception;

    wasm_application_execute_main(module_inst, app_argc, app_argv);
    if ((exception = wasm_runtime_get_exception(module_inst)))
        printf("%s\n", exception);
    return NULL;
}

int
main(int argc, char *argv_main[])
{
    static char global_heap_buf[2048 * 1024 * 10];
    char *buffer, error_buf[128];
    int opt;
    char *wasm_path = NULL;

    wasm_module_t module = NULL;
    wasm_module_inst_t module_inst = NULL;
    wasm_exec_env_t exec_env = NULL;
    uint32 buf_size, stack_size = 64 * 1024, heap_size = 2048 * 1024;
    wasm_function_inst_t func = NULL;
    wasm_function_inst_t func2 = NULL;
    char *native_buffer = NULL;
    uint32_t wasm_buffer = 0;
    bool restore_flag = false;

    RuntimeInitArgs init_args;
    memset(&init_args, 0, sizeof(RuntimeInitArgs));

    while ((opt = getopt(argc, argv_main, "hf:r")) != -1) {
        switch (opt) {
            case 'f':
                wasm_path = optarg;
                break;
            case 'r':
                restore_flag = true;
                break;
            case 'h':
                print_usage();
                return 0;
            case '?':
                print_usage();
                return 0;
        }
    }
    if (optind == 1 && !restore_flag) {
        print_usage();
        return 0;
    }

    // Define an array of NativeSymbol for the APIs to be exported.
    // Note: the array must be static defined since runtime
    //            will keep it after registration
    // For the function signature specifications, goto the link:
    // https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md

    // static NativeSymbol native_symbols[] = {
    //     {
    //         "intToStr", // the name of WASM function name
    //         intToStr,   // the native function pointer
    //         "(i*~i)i",  // the function prototype signature, avoid to use i32
    //         NULL        // attachment is NULL
    //     },
    //     {
    //         "get_pow", // the name of WASM function name
    //         get_pow,   // the native function pointer
    //         "(ii)i",   // the function prototype signature, avoid to use i32
    //         NULL       // attachment is NULL
    //     },
    //     { 
    //         "calculate_native", 
    //         calculate_native, 
    //         "(iii)i", 
    //         NULL 
    //     },
    //     {
    //         "wrapped_sleep",
    //         wrapped_sleep,
    //         "(ii)i",
    //         NULL
    //     }
    // };

    init_args.mem_alloc_type = Alloc_With_System_Allocator;
    // init_args.mem_alloc_type = Alloc_With_Pool;
    // init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
    // init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

    // Native symbols need below registration phase
    // init_args.n_native_symbols = sizeof(native_symbols) / sizeof(NativeSymbol);
    // init_args.native_module_name = "env";
    // init_args.native_symbols = native_symbols;

    if (!wasm_runtime_full_init(&init_args)) {
        printf("Init runtime environment failed.\n");
        return -1;
    }

    if (restore_flag) {
        uint32 argv[2];
        printf("call restore_runtime\n");
        wasm_runtime_restore(2, argv);
        printf("end restore_runtime\n");
        return 0;
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

    app_instance_main(module_inst);

    // exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
    // if (!exec_env) {
    //     printf("Create wasm execution environment failed.\n");
    //     goto fail;
    // }
    // 
    // uint32 argv[2];
    // if (!(func = wasm_runtime_lookup_function(module_inst, "main",
    //                                           NULL))) {
    //     printf("The main wasm function is not found.\n");
    //     goto fail;
    // }
    // printf("calling\n");
    // // pass 4 elements for function arguments
    // if (!wasm_runtime_call_wasm(exec_env, func, 2, argv)) {
    //     printf("call wasm function main failed. %s\n",
    //            wasm_runtime_get_exception(module_inst));
    //     goto fail;
    // }


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
        //BH_FREE(buffer);
    wasm_runtime_destroy();
    return 0;
}
