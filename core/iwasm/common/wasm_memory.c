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

/*
struct Pool_Info {
    unsigned int p_abs;
    void *p_raw;
    Data_Type type;
    struct Pool_Info *next;
}
*/

void
insert(void *addr)
{
    int i, j;
    void *p;
    if (addr_list_size > MAX_LIST_SIZE - 1) {
        exit(1);
    }

    for (i = 0; i < addr_list_size; i++) {
        if (addr < addr_list[i]) {
            for (j = i; j < addr_list_size + 1; j++) {
                p = addr_list[i];
                addr_list[i] = addr;
                addr = p;
            }
            return i;
        }
    }
    addr_list[i] = addr;
    return i;
}

void
alloc_info(void *addr, Data_Type type)
{
    Pool_Info *info = malloc(sizeof(Pool_Info));
    *info = { .p_abs = addr - pool_allocator, .p_raw = addr, .type = type };
    pool_list[info.p_abs] = info;
}

void
alloc_infos(void *addr, Data_Type type, size_t size)
{
    switch (type) {
        case:
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
