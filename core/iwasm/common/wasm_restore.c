#include <stdlib.h>

#include "../interpreter/wasm_runtime.h"
#include "wasm_restore.h"

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

    // struct WASMInterpFrame *prev_frame;
    /*
        skip
    */

    // struct WASMFunctionInstance *function;
    uint32 func_idx;
    fread(&func_idx, sizeof(uint32), 1, fp);
    if (func_idx == -1) {
        return;
    }

    frame->function = module_inst->functions + func_idx;
    WASMFunctionInstance *func = frame->function;

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
                fread(lp, sizeof(int32), 1, fp);
                lp++;
                break;
            case VALUE_TYPE_I64:
            case VALUE_TYPE_F64:
                fread(lp, sizeof(int64), 1, fp);
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
                fwrite(lp, sizeof(int32), 1, fp);
                lp++;
                break;
            case VALUE_TYPE_I64:
            case VALUE_TYPE_F64:
                fwrite(lp, sizeof(int64), 1, fp);
                lp += 2;
                break;
            default:
            printf("TYPE NULL\n");
                break;
        }
    }

    uint8 *tsp = frame->tsp_bottom;
    while (tsp != frame->tsp) {
        fread(tsp, sizeof(uint8), 1, fp);
        tsp++;
    }

    tsp = frame->tsp_bottom;
    uint32 *sp = frame->sp_bottom;
    while (sp != frame->sp) {
        switch (*tsp) {
            case VALUE_TYPE_I32:
            case VALUE_TYPE_F32:
                fread(sp, sizeof(uint32), 1, fp);
                sp++;
                tsp++;
                break;
            case VALUE_TYPE_I64:
            case VALUE_TYPE_F64:
                fread(sp, sizeof(uint64), 1, fp);
                sp += 2;
                tsp += 2;
                break;
            default:
                printf("TYPE NULL\n");
                break;
        }
    }

    WASMBranchBlock *csp = frame->csp_bottom;
    uint32 csp_num = frame->csp - frame->csp_bottom;

     for (i = 0; i < csp_num; i++,csp++) {
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

        // uint32 cell_num;
        fread(&csp->cell_num, sizeof(uint32), 1, fp);
    }
}
