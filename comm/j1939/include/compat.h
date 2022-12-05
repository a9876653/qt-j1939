/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __COMPAT_H__
#define __COMPAT_H__

#include <stdlib.h>
#define htobe64(x) __builtin_bswap64(x)
#define htobe16(x) __builtin_bswap16(x)

#endif /* __COMPAT_H__ */
