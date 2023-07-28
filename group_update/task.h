#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define PERIOD_TASK_STOP (-1)

typedef uint32_t task_param_t;
typedef void (*task_fun)(task_param_t param);

typedef struct
{
    task_fun     fun;
    task_param_t param;
} task_t;

#define THREAD_TASK_INIT(_fun, _param)               \
    {                                                \
        .fun = _fun, .param = (task_param_t)(_param) \
    }

typedef void (*void_task_fun)(void);

typedef struct
{
    const void_task_fun task;
    const uint16_t      period;
    uint32_t            tick;
} period_task_t;

#define PERIOD_TASK_INIT(t, p)             \
    {                                      \
        .task = t, .period = p, .tick = 0, \
    }

typedef struct
{
    const task_t   task;
    const uint16_t period;
    uint32_t       tick;
} period_task_args_t;

#define PERIOD_TASK_ARGS_INIT(t, p, args)                          \
    {                                                              \
        .task = THREAD_TASK_INIT(t, args), .period = p, .tick = 0, \
    }

