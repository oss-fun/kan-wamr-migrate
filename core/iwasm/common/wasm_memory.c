/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */
#include <stdlib.h>
#include "wasm_runtime_common.h"
#include "bh_platform.h"
#include "mem_alloc.h"
#include "wasm_memory.h"

typedef enum Memory_Mode {
    MEMORY_MODE_UNKNOWN = 0,
    MEMORY_MODE_POOL,
    MEMORY_MODE_ALLOCATOR
} Memory_Mode;

static Memory_Mode memory_mode = MEMORY_MODE_UNKNOWN;

static mem_allocator_t pool_allocator = NULL;

static void *(*malloc_func)(unsigned int size) = NULL;
static void *(*realloc_func)(void *ptr, unsigned int size) = NULL;
static void (*free_func)(void *ptr) = NULL;

static unsigned int global_pool_size;

#define MAX_LIST_SIZE
static Pool_Info **root_info = NULL;
static Pool_Info *pool_list[MAX_LIST_SIZE] = {};
static unsigned int addr_list_size = 0;

static void (*dump_data[ERRORT])(void *p);


#define CASE_INFOS(data_type) \
    case data_type##T:\
        for(i=0;i<size;i++){\
            info = malloc(sizeof(Pool_Info));\
            *info = { .p_abs = addr - pool_allocator, .p_raw = addr, .type = type};\
            pool_list[info.p_abs] = info;\
            (data_type*)addr++;\
        }\
        break;



void
alloc_info(void *addr, Data_Type type)
{
    Pool_Info *info = malloc(sizeof(Pool_Info));
    *info = { .p_abs = addr - pool_allocator, .p_raw = addr, .type = type};
    pool_list[info.p_abs] = info;
}

void
alloc_infos(void *addr, Data_Type type, size_t size)
{
    Pool_Info *info;
    int i;
    switch (type) {
        CASE_INFOS(char)
        //CASE_INFOS(charTT)
        CASE_INFOS(uint8)
        CASE_INFOS(uint16)
        CASE_INFOS(uint32)
        CASE_INFOS(uint64)
        
        CASE_INFOS(gc_heap_t)
        CASE_INFOS(base_addr)
        CASE_INFOS(WASIContext)
        CASE_INFOS(WASMThreadArg)
        CASE_INFOS(ExternRefMapNode)

        CASE_INFOS(fd_table)
        CASE_INFOS(fd_prestats)
        CASE_INFOS(argv_environ_values)

        CASE_INFOS(uvwasi_t)
        CASE_INFOS(uvwasi_preopen_t)

        CASE_INFOS(wasm_val_t)
        CASE_INFOS(WASMExecEnv)

        CASE_INFOS(NativeSymbolsNode)

        CASE_INFOS(WASMModuleCommon)
        CASE_INFOS(WASMModuleInstanceCommon)
        CASE_INFOS(WASMModuleMemConsumption)
        CASE_INFOS(WASMModuleInstMemConsumption)
        CASE_INFOS(WASMMemoryInstanceCommon)
        CASE_INFOS(WASMSection)
        CASE_INFOS(WASMCApiFrame)

        CASE_INFOS(WASMSharedMemNode)

        CASE_INFOS(WASMModule)
        CASE_INFOS(WASMFunction)
        //CASE_INFOS(WASMFunctionTT)
        CASE_INFOS(WASMGlobal)
        CASE_INFOS(WASMExport)
        //CASE_INFOS(V128T)
       CASE_INFOS( WASMValue)
        CASE_INFOS(InitializerExpression)
        CASE_INFOS(WASMType)
        //CASE_INFOS(WASMTypeTT)
        CASE_INFOS(WASMTable)
        CASE_INFOS(WASMMemory)
        CASE_INFOS(WASMTableImport)
        CASE_INFOS(WASMMemoryImport)
        CASE_INFOS(WASMFunctionImport)
        CASE_INFOS(WASMGlobalImport)
        CASE_INFOS(WASMImport)
        CASE_INFOS(WASMTableSeg)
        CASE_INFOS(WASMDataSeg)
        CASE_INFOS(BlockAddr)
        CASE_INFOS(WASIArguments)
        CASE_INFOS(StringNode)
        CASE_INFOS(BlockType)
        CASE_INFOS(WASMBranchBlock)
    
        CASE_INFOS(WASMInterpFrame)

        CASE_INFOS(BranchBlock)
        CASE_INFOS(WASMLoaderContext)
        CASE_INFOS(Const)

        CASE_INFOS(WASMModuleInstance)

        CASE_INFOS(WASMFunctionInstance)
        CASE_INFOS(WASMMemoryInstance)
        //CASE_INFOS(WASMMemoryInstanceTT)
        CASE_INFOS(WASMTableInstance)
        //CASE_INFOS(WASMTableInstanceTT)
        CASE_INFOS(WASMGlobalInstance)
        CASE_INFOS(WASMExportFuncInstance)
        CASE_INFOS(WASMRuntimeFrame)

        CASE_INFOS(WASMOpcode)
        CASE_INFOS(WASMMiscEXTOpcode)
        CASE_INFOS(WASMSimdEXTOpcode)
        CASE_INFOS(WASMAtomicEXTOpcode)

    }
}

void
free_info(void *addr)
{
    if (pool_list[addr - pool_allocator] != 0) {
        free(pool_list[addr - pool_allocator]);
        pool_list[addr - pool_allocator] = 0;
    }
}

static bool
wasm_memory_init_with_pool(void *mem, unsigned int bytes)
{
    mem_allocator_t _allocator = mem_allocator_create(mem, bytes);

    if (_allocator) {
        memory_mode = MEMORY_MODE_POOL;
        pool_allocator = _allocator;
        global_pool_size = bytes;
        return true;
    }
    LOG_ERROR("Init memory with pool (%p, %u) failed.\n", mem, bytes);
    return false;
}

static bool
wasm_memory_init_with_allocator(void *_malloc_func,
                                void *_realloc_func,
                                void *_free_func)
{
    if (_malloc_func && _free_func && _malloc_func != _free_func) {
        memory_mode = MEMORY_MODE_ALLOCATOR;
        malloc_func = _malloc_func;
        realloc_func = _realloc_func;
        free_func = _free_func;
        return true;
    }
    LOG_ERROR("Init memory with allocator (%p, %p, %p) failed.\n",
              _malloc_func, _realloc_func, _free_func);
    return false;
}

bool
wasm_runtime_memory_init(mem_alloc_type_t mem_alloc_type,
                         const MemAllocOption *alloc_option)
{
    if (mem_alloc_type == Alloc_With_Pool)
        return wasm_memory_init_with_pool(alloc_option->pool.heap_buf,
                                          alloc_option->pool.heap_size);
    else if (mem_alloc_type == Alloc_With_Allocator)
        return wasm_memory_init_with_allocator(
          alloc_option->allocator.malloc_func,
          alloc_option->allocator.realloc_func,
          alloc_option->allocator.free_func);
    else if (mem_alloc_type == Alloc_With_System_Allocator)
        return wasm_memory_init_with_allocator(os_malloc, os_realloc, os_free);
    else
        return false;
}

void
wasm_runtime_memory_destroy()
{
    if (memory_mode == MEMORY_MODE_POOL)
        mem_allocator_destroy(pool_allocator);
    memory_mode = MEMORY_MODE_UNKNOWN;
}

unsigned
wasm_runtime_memory_pool_size()
{
    if (memory_mode == MEMORY_MODE_POOL)
        return global_pool_size;
    else
        return 1 * BH_GB;
}

static inline void *
wasm_runtime_malloc_internal(unsigned int size)
{
    if (memory_mode == MEMORY_MODE_UNKNOWN) {
        LOG_WARNING(
          "wasm_runtime_malloc failed: memory hasn't been initialize.\n");
        return NULL;
    }
    else if (memory_mode == MEMORY_MODE_POOL) {
        return mem_allocator_malloc(pool_allocator, size);
    }
    else {
        return malloc_func(size);
    }
}

static inline void *
wasm_runtime_realloc_internal(void *ptr, unsigned int size)
{
    if (memory_mode == MEMORY_MODE_UNKNOWN) {
        LOG_WARNING(
          "wasm_runtime_realloc failed: memory hasn't been initialize.\n");
        return NULL;
    }
    else if (memory_mode == MEMORY_MODE_POOL) {
        return mem_allocator_realloc(pool_allocator, ptr, size);
    }
    else {
        if (realloc_func)
            return realloc_func(ptr, size);
        else
            return NULL;
    }
}

static inline void
wasm_runtime_free_internal(void *ptr)
{
    if (!ptr) {
        LOG_WARNING("warning: wasm_runtime_free with NULL pointer\n");
        return;
    }

    if (memory_mode == MEMORY_MODE_UNKNOWN) {
        LOG_WARNING("warning: wasm_runtime_free failed: "
                    "memory hasn't been initialize.\n");
    }
    else if (memory_mode == MEMORY_MODE_POOL) {
        mem_allocator_free(pool_allocator, ptr);
    }
    else {
        free_func(ptr);
    }
}

void *
wasm_runtime_malloc(unsigned int size)
{
    if (size == 0) {
        LOG_WARNING("warning: wasm_runtime_malloc with size zero\n");
        /* At lease alloc 1 byte to avoid malloc failed */
        size = 1;
    }

    return wasm_runtime_malloc_internal(size);
}

void *
wasm_runtime_realloc(void *ptr, unsigned int size)
{
    return wasm_runtime_realloc_internal(ptr, size);
}

void
wasm_runtime_free(void *ptr)
{
    wasm_runtime_free_internal(ptr);
}
