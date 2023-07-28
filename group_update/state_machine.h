#pragma once

#include "stdint.h"
#include "stdbool.h"
#include "rt_slist.h"

typedef enum
{
    STATE_MACHINE_EVENT_RUN   = 0,
    STATE_MACHINE_EVENT_ENTRY = 1,
} state_machine_event_e;

typedef struct state_machine_s state_machine_t;

typedef void (*state_machine_fun)(state_machine_t *self, state_machine_event_e event);

struct state_machine_s
{
    state_machine_fun func;

    rt_slist_t node;
    uint32_t   step;
    void      *args;
};

#define STATE_MACHINE_INIT(_func, _step, _args)     \
    {                                               \
        .func = _func, .step = _step, .args = _args \
    }

bool state_machine_append(rt_slist_t *root, uint32_t step, void *args, state_machine_fun func);
void state_machine_run(state_machine_t *handle);

state_machine_t *switch_state_machine(rt_slist_t *root, uint32_t step);