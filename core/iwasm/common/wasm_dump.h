#ifndef _WASM_CHECKPOINT_H
#define _WASM_CHECKPOINT_H

#include "wasm_exec_env.h"
#include "../interpreter/wasm_interp.h"

typedef struct Frame_Info {
    WASMInterpFrame *frame;
    WASMExecEnv *exec_env;
    uint32 size;
    struct Frame_Info *prev;
    struct Frame_Info *next;
} Frame_Info;

void
wasm_dump_alloc_frame(WASMInterpFrame *frame, uint32 size,
                      WASMExecEnv *exec_env);

void
wasm_dump_free_frame(void);

void
wasm_dump_frame(void);


#endif // _WASM_CHECKPOINT_H
