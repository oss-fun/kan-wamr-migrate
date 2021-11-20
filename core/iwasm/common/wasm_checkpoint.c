#include <stdlib.h>

#include "../interpreter/wasm_runtime.h"
#include "wasm_checkpoint.h"

static Frame_Info *root_info = NULL, *tail_info = NULL;

void
wasm_checkpoint_alloc_frame(WASMInterpFrame *frame, uint32 size,
                            WASMExecEnv *exec_env)
{
    Frame_Info *info;
    info = malloc(sizeof(Frame_Info));
    info->frame = frame;
    info->size = size;
    info->exec_env = exec_env;
    info->prev = tail_info;
    info->next = NULL;

    if (root_info == NULL) {
        root_info = tail_info = info;
    }
    else {
        tail_info->next = info;
        tail_info = tail_info->next;
    }
}

void
wasm_checkpoint_free_frame(void)
{
    tail_info = tail_info->prev;
    free(tail_info->next);
    tail_info->next = NULL;
}

void
wasm_checkpoint_dump_frame(void)
{
    Frame_Info *info;
    FILE *fp;
    if (!root_info) {
        printf("dump failed\n");
        exit(1);
    }
    fp = fopen("frame.img", "w");
    info = root_info;
    while (info) {
        dump_WASMInterpFrame(info->frame, info->exec_env, fp);
    }
}

static void
dump_WASMInterpFrame(WASMInterpFrame *frame, WASMExecEnv *exec_env, FILE *fp)
{
    int i;
    WASMModuleInstance *module_inst = exec_env->module_inst;

// struct WASMInterpFrame *prev_frame;
/*
    skip
*/

// struct WASMFunctionInstance *function;
#ifdef METHOD_A
    for (i = 0; i < module_inst->function_count; i++) {
        if (frame->function == &module_inst->functions[i]) {
            fwrite(&i, sizeof(int), 1, fp);
            break;
        }
    }
#else
    uint32 func_idx = frame->function - module_inst->functions;
    fwrite(&func_idx, sizeof(uint32), 1, fp);
#endif

    // uint8 *ip;
    uint32 ip = frame->ip - wasm_get_func_code(frame->function);
    fwrite(&ip, sizeof(uint32), 1, fp);

    // uint32 *sp_bottom;
    /*
        skip
    */
    // uint32 *sp_boundary;
    /*
        skip
    */

    // uint32 *sp;
    uint32 sp = frame->sp - frame->sp_bottom;
    fwrite(&sp, sizeof(uint32), 1, fp);

    // WASMBranchBlock *csp_bottom;
    /*
        skip
    */
    // WASMBranchBlock *csp_boundary;
    /*
        skip
    */
    // WASMBranchBlock *csp;
    uint32 csp = frame->csp - frame->csp_bottom;
    fwrite(&csp, sizeof(uint32), 1, fp);

    // uint32 lp[1];
#ifdef METHOD_A

#else
    fwrite(frame->lp, sizeof(uint32),
           frame->function->param_cell_num + frame->function->local_cell_num,
           fp);
    fwrite(frame->sp, sizeof(uint32),
           frame->function->u.func->max_stack_cell_num, fp);

#endif

    WASMBranchBlock *bb = frame->csp_bottom;
    uint32 csp_num = frame->csp - frame->csp_bottom;

    for (int i = 0; i < csp_num; i++, bb++) {
        // uint8 *begin_addr;
        uint64 addr;
        if (bb->begin_addr == NULL) {
            addr = -1;
            fwrite(&addr, sizeof(uint64), 1, fp);
        }
        else {
            addr = bb->begin_addr - wasm_get_func_code(frame->function);
            fwrite(&addr, sizeof(uint64), 1, fp);
        }

        // uint8 *target_addr;
        if (bb->target_addr == NULL) {
            addr = -1;
            fwrite(&addr, sizeof(uint64), 1, fp);
        }
        else {
            addr = bb->target_addr - wasm_get_func_code(frame->function);
            fwrite(&addr, sizeof(uint64), 1, fp);
        }

        // uint32 *frame_sp;
        if (bb->frame_sp == NULL) {
            addr = -1;
            fwrite(&addr, sizeof(uint64), 1, fp);
        }
        else {
            sp = bb->frame_sp - frame->sp_bottom;
            fwrite(&sp, sizeof(uint64), 1, fp);
        }
        // uint32 cell_num;
        fwrite(&bb->cell_num, sizeof(uint32), 1, fp);
    }
}
