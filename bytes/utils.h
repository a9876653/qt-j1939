#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))
#define ARRAYEND(x) (&(x)[ARRAYLEN(x)])

// 4bytes、32bit数据大小端转化
#define L2B32(Little) (((Little & 0xff) << 24) | (((Little)&0xff00) << 8) | (((Little)&0xff0000) >> 8) | ((Little >> 24) & 0xff))

// 2bytes、16bit数据大小端转化
#define L2B16(Little) (((Little & 0xff) << 8) | ((Little >> 8) & 0xff))

/**
 * \brief           Get larger value out of 2 different inputs
 * \param[in]       x: First input
 * \param[in]       y: Second input
 * \return          Larger of both inputs
 */
#define LW_MAX(x, y) ((x) > (y) ? (x) : (y))

/**
 * \brief           Get smaller value out of 2 different inputs
 * \param[in]       x: First input
 * \param[in]       y: Second input
 * \return          Smaller of both inputs
 */
#define LW_MIN(x, y) ((x) < (y) ? (x) : (y))

#ifndef BIT
#define BIT(x) ((uint32_t)((uint32_t)0x01U << (x)))
#endif

#ifndef BITS
#define BITS(start, end) ((0xFFFFFFFFUL << (start)) & (0xFFFFFFFFUL >> (31U - (uint32_t)(end))))
#endif

#ifndef GET_BITS
#define GET_BITS(regval, start, end) (((regval)&BITS((start), (end))) >> (start))
#endif

#ifndef GET_BIT
#define GET_BIT(regval, bit) GET_BITS(regval, bit, bit)
#endif

#define PERIOD_TASK_STOP (-1)

typedef void (*period_task_fun)(void);

typedef struct
{
    const period_task_fun task;
    const uint16_t        period;
    uint32_t              tick;
} period_task_t;

#define PERIOD_TASK_INIT(t, p)             \
    {                                      \
        .task = t, .period = p, .tick = 0, \
    }

typedef void (*period_task_args_fun)(void *args);

typedef struct
{
    const period_task_args_fun task;
    const uint16_t             period;
    uint32_t                   tick;
} period_task_args_t;

#define PERIOD_TASK_ARGS_INIT(t, p)        \
    {                                      \
        .task = t, .period = p, .tick = 0, \
    }
