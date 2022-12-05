#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define RT_NULL   (0)
#define rt_inline static inline

/**
 * @brief return the member address of ptr, if the type of ptr is the struct type.
 * 获取 type 结构体中, member 成员在这个结构体中的偏移, 从而得到 type 结构体的首地址, 后续可强转得到结构体的全部变量
 */
#define rt_container_of(ptr, type, member) ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#ifdef __cplusplus
}
#endif