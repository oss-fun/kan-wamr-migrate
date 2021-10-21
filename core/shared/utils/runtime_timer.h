/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef LIB_BASE_RUNTIME_TIMER_H_
#define LIB_BASE_RUNTIME_TIMER_H_

#include "bh_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64 bh_get_tick_ms();
uint32 bh_get_elpased_ms(uint32 *last_system_clock);


typedef struct _timer_ctx * timer_ctx_t;
typedef void (*timer_callback_f)(unsigned int id, unsigned int owner);
typedef void (*check_timer_expiry_f)(timer_ctx_t ctx);

typedef struct _app_timer {
    struct _app_timer * next;
    uint32 id;
    uint32 interval;
    uint64 expiry;
    bool is_periodic;
} app_timer_t;

struct _timer_ctx {
    app_timer_t *app_timers;
    app_timer_t *idle_timers;
    app_timer_t *free_timers;
    uint32 max_timer_id;
    int pre_allocated;
    uint32 owner;

    /* mutex and condition */
    korp_cond cond;
    korp_mutex mutex;

    timer_callback_f timer_callback;
    check_timer_expiry_f refresh_checker;
};

timer_ctx_t create_timer_ctx(timer_callback_f timer_handler,
                             check_timer_expiry_f, int prealloc_num,
                             unsigned int owner);
void destroy_timer_ctx(timer_ctx_t);
unsigned int timer_ctx_get_owner(timer_ctx_t ctx);

uint32 sys_create_timer(timer_ctx_t ctx, int interval, bool is_period,
                        bool auto_start);
bool sys_timer_destroy(timer_ctx_t ctx, uint32 timer_id);
bool sys_timer_cancel(timer_ctx_t ctx, uint32 timer_id);
bool sys_timer_restart(timer_ctx_t ctx, uint32 timer_id, int interval);
void cleanup_app_timers(timer_ctx_t ctx);
uint32 check_app_timers(timer_ctx_t ctx);
uint32 get_expiry_ms(timer_ctx_t ctx);

#ifdef __cplusplus
}
#endif
#endif /* LIB_BASE_RUNTIME_TIMER_H_ */
