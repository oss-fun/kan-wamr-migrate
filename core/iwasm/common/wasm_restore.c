#include <stdio.h>
#include <stdlib.h>

#include "wasm_exec_env.h"
#include "wasm_memory.h"
#include "../interpreter/wasm_runtime.h"
#include "wasm_dump.h"
#include "wasm_restore.h"

static Frame_Info *root_info = NULL, *tail_info = NULL;

static inline WASMInterpFrame *
ALLOC_FRAME(WASMExecEnv *exec_env, uint32 size, WASMInterpFrame *prev_frame)
{
    WASMInterpFrame *frame = wasm_exec_env_alloc_wasm_frame(exec_env, size);

    if (frame) {
        wasm_dump_alloc_frame(frame, exec_env);
        frame->prev_frame = prev_frame;
#if WASM_ENABLE_PERF_PROFILING != 0
        frame->time_started = os_time_get_boot_microsecond();
#endif
    }
    else {
        wasm_set_exception((WASMModuleInstance *)exec_env->module_inst,
                           "wasm operand stack overflow");
    }

    return frame;
}

WASMInterpFrame *
wasm_restore_frame(WASMExecEnv *exec_env)
{
    WASMModuleInstance *module_inst =
        (WASMModuleInstance *)exec_env->module_inst;
    WASMInterpFrame *frame, *prev_frame = wasm_exec_env_get_cur_frame(exec_env);
    WASMFunctionInstance *function;
    Frame_Info *info;
    uint32 func_idx, frame_size, all_cell_num;
    FILE *fp;
    int i;

    fp = fopen("frame.img", "rb");

    info = malloc(sizeof(Frame_Info));

    while (!feof(fp)) {

        if ((fread(&func_idx, sizeof(uint32), 1, fp)) == 0) {
            break;
        }

        if (func_idx == -1) {
            // 初期フレームのスタックサイズをreadしてALLOC
            fread(&all_cell_num, sizeof(uint32), 1, fp);
            frame_size = wasm_interp_interp_frame_size(all_cell_num);
            frame = ALLOC_FRAME(exec_env, frame_size,
                                (WASMInterpFrame *)prev_frame);

            // 初期フレームをrestore
            frame->function = NULL;
            frame->ip = NULL;
            frame->sp = frame->lp + 0;

            if (!(frame->tsp = wasm_runtime_malloc((uint64)all_cell_num))) {
                exit(1);
            }
            frame->tsp_bottom = frame->tsp;
            frame->tsp_boundary = frame->tsp_bottom + all_cell_num;

            info->frame = prev_frame = frame;
            info->all_cell_num = all_cell_num;
        }
        else {
            // 関数からスタックサイズを計算し,ALLOC
            function = module_inst->functions + func_idx;
            printf("restore func_idx: %d\n", func_idx);

            all_cell_num = (uint64)function->param_cell_num
                           + (uint64)function->local_cell_num
                           + (uint64)function->u.func->max_stack_cell_num
                           + ((uint64)function->u.func->max_block_num)
                                 * sizeof(WASMBranchBlock) / 4;
            frame_size = wasm_interp_interp_frame_size(all_cell_num);
            frame = ALLOC_FRAME(exec_env, frame_size,
                                (WASMInterpFrame *)prev_frame);

            // フレームをrestore
            frame->function = function;
            restore_WASMInterpFrame(frame, exec_env, fp);

            info->frame = prev_frame = frame;
        }

        if (root_info == NULL) {
            root_info = tail_info = info;
        }
        else {
            tail_info->next = info;
            tail_info = tail_info->next;
        }
    }
    wasm_exec_env_set_cur_frame(exec_env, frame);
    wasm_dump_set_root_and_tail(root_info, tail_info);
    fclose(fp);
    return tail_info->frame;
}

static void
restore_WASMMemoryInstance(WASMMemoryInstance *memory, WASMExecEnv *exec_env,
                           FILE *fp)
{
    // /* Module type */
    // uint32 module_type;
    // /* Shared memory flag */
    // bool is_shared;
    // /* Number bytes per page */
    // uint32 num_bytes_per_page;
    // /* Current page count */
    // uint32 cur_page_count;
    // /* Maximum page count */
    // uint32 max_page_count;

    // /* Heap data base address */
    // uint8 *heap_data;
    // /* Heap data end address */
    // uint8 *heap_data_end;
    // /* The heap created */
    // void *heap_handle;

#if WASM_ENABLE_MULTI_MODULE != 0
    /* to indicate which module instance create it */
    // WASMModuleInstance *owner;
#endif

#if WASM_ENABLE_SHARED_MEMORY != 0
    /* mutex lock for the memory, used in atomic operation */
    // korp_mutex mem_lock;
#endif

    /* Memory data end address */
    // uint8 *memory_data_end;

    /* Memory data begin address, the layout is: memory data + heap data
       Note: when memory is re-allocated, the heap data and memory data
             must be copied to new memory also. */
    // uint8 *memory_data;
    fread(memory->memory_data, sizeof(uint8),
          memory->memory_data_end - memory->memory_data, fp);
}

static void
restore_WASMInterpFrame(WASMInterpFrame *frame, WASMExecEnv *exec_env, FILE *fp)
{
    int i;
    WASMModuleInstance *module_inst = exec_env->module_inst;
    WASMFunctionInstance *func = frame->function;

    // struct WASMInterpFrame *prev_frame;
    // struct WASMFunctionInstance *function;
    // uint8 *ip;
    uint32 ip_offset;
    fread(&ip_offset, sizeof(uint32), 1, fp);
    frame->ip = wasm_get_func_code(frame->function) + ip_offset;

    // uint32 *sp_bottom;
    // uint32 *sp_boundary;
    // uint32 *sp;
    frame->sp_bottom = frame->lp + func->param_cell_num + func->local_cell_num;
    frame->sp_boundary = frame->sp_bottom + func->u.func->max_stack_cell_num;
    uint32 sp_offset;
    fread(&sp_offset, sizeof(uint32), 1, fp);
    frame->sp = frame->sp_bottom + sp_offset;

    // uint8 *tsp_bottom;
    // uint8 *tsp_boundary;
    // uint8 *tsp;
    if (!(frame->tsp_bottom =
              wasm_runtime_malloc((uint64)func->u.func->max_stack_cell_num))) {
        printf("malloc error\n");
        exit(1);
    }
    frame->tsp_boundary = frame->tsp_bottom + func->u.func->max_stack_cell_num;
    uint32 tsp_offset;
    fread(&tsp_offset, sizeof(uint32), 1, fp);
    frame->tsp = frame->tsp_bottom + tsp_offset;

    // WASMBranchBlock *csp_bottom;
    // WASMBranchBlock *csp_boundary;
    // WASMBranchBlock *csp;
    frame->csp_bottom = frame->sp_boundary;
    frame->csp_boundary = frame->csp_bottom + func->u.func->max_block_num;
    uint32 csp_offset;
    fread(&csp_offset, sizeof(uint32), 1, fp);
    frame->csp = frame->csp_bottom + csp_offset;

    // =========================================================

    // uint32 lp[1];
    uint32 *lp = frame->lp;
    // VALUE_TYPE_I32
    // VALUE_TYPE_F32
    // VALUE_TYPE_I64
    // VALUE_TYPE_F64
    for (i = 0; i < func->param_count; i++) {
        switch (func->param_types[i]) {
            case VALUE_TYPE_I32:
            case VALUE_TYPE_F32:
                fread(lp, sizeof(uint32), 1, fp);
                lp++;
                break;
            case VALUE_TYPE_I64:
            case VALUE_TYPE_F64:
                fread(lp, sizeof(uint64), 1, fp);
                lp += 2;
                break;
            default:
                printf("TYPE NULL\n");
                break;
        }
    }
    for (i = 0; i < func->local_count; i++) {
        switch (func->local_types[i]) {
            case VALUE_TYPE_I32:
            case VALUE_TYPE_F32:
                fread(lp, sizeof(uint32), 1, fp);
                lp++;
                break;
            case VALUE_TYPE_I64:
            case VALUE_TYPE_F64:
                fread(lp, sizeof(uint64), 1, fp);
                lp += 2;
                break;
            default:
                printf("TYPE NULL\n");
                break;
        }
    }

    /*
    uint8 *tsp = frame->tsp_bottom;
    while (tsp != frame->tsp) {
        fread(tsp, sizeof(uint8), 1, fp);
        tsp++;
    }
    */
    fread(frame->tsp_bottom, sizeof(uint8), tsp_offset, fp);

    for (i = 0; i < sp_offset;) {
        switch (frame->tsp_bottom[i]) {
            case VALUE_TYPE_I32:
            case VALUE_TYPE_F32:
                fread(&frame->sp_bottom[i], sizeof(uint32), 1, fp);
                i++;
                break;
            case VALUE_TYPE_I64:
            case VALUE_TYPE_F64:
                fread(&frame->sp_bottom[i], sizeof(uint64), 1, fp);
                i += 2;
                break;
            default:
                printf("type error in wasm_restore.c\n");
                exit(1);
                break;
        }
    }

    WASMBranchBlock *csp = frame->csp_bottom;
    uint32 csp_num = frame->csp - frame->csp_bottom;

    for (i = 0; i < csp_num; i++, csp++) {
        // uint8 *begin_addr;
        uint64 addr;
        fread(&addr, sizeof(uint64), 1, fp);
        if (addr == -1) {
            csp->begin_addr = NULL;
        }
        else {
            csp->begin_addr = addr + wasm_get_func_code(frame->function);
        }

        // uint8 *target_addr;
        fread(&addr, sizeof(uint64), 1, fp);
        if (addr == -1) {
            csp->target_addr = NULL;
        }
        else {
            csp->target_addr = addr + wasm_get_func_code(frame->function);
        }

        // uint32 *frame_sp;
        fread(&addr, sizeof(uint64), 1, fp);
        if (addr == -1) {
            csp->frame_sp = NULL;
        }
        else {
            csp->frame_sp = addr + frame->sp_bottom;
        }

        // uint8 *frame_tsp;
        fread(&addr, sizeof(uint64), 1, fp);
        if (addr == -1) {
            csp->frame_tsp = NULL;
        }
        else {
            csp->frame_tsp = addr + frame->tsp_bottom;
        }

        // uint32 cell_num;
        fread(&csp->cell_num, sizeof(uint32), 1, fp);
    }
}
