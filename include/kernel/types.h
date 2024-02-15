#ifndef _TYPES_H_
#define _TYPES_H_

#include <onix.h>

typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef unsigned int size_t;
typedef int int32_t;
typedef unsigned int uint32_t;

typedef long long int int164_t;
typedef unsigned long long int uint64_t;

typedef uint32_t idx_t;

#define EOF -1
#define EOS '\0' // end of string
#define NULL 0
#ifndef __cplusplus
#define bool _Bool
#define true 1
#define false 0
#endif

// 结构紧凑
#define _packed __attribute__((packed))
// 用于省略函数的栈帧
#define _ofp __attribute__((optimize("omit-frame-pointer")))

#endif