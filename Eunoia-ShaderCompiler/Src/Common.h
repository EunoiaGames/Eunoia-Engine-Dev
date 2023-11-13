#pragma once

#define EU_NULL 0
#define EU_PERSISTENT static

//Values copied from <stdint.h>
#define EU_S8_MIN         (-127i8 - 1)
#define EU_S16_MIN        (-32767i16 - 1)
#define EU_S32_MIN        (-2147483647i32 - 1)
#define EU_S64_MIN        (-9223372036854775807i64 - 1)
#define EU_S8_MAX         127i8
#define EU_S16_MAX        32767i16
#define EU_S32_MAX        2147483647i32
#define EU_S64_MAX        9223372036854775807i64
#define EU_U8_MAX        0xffui8
#define EU_U16_MAX       0xffffui16
#define EU_U32_MAX       0xffffffffui32
#define EU_U64_MAX       0xffffffffffffffffui64
///////////////////////////////////////////////

#define EU_CONST_INT_TO_STRING_FINAL(X) (#X)
#define EU_CONST_INT_TO_STRING(X) EU_CONST_INT_TO_STRING_FINAL(X)

#define EU_HAS_FLAG(Flags, Bit) (((Flags) & (Bit)) == (Bit))

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long int s64;
typedef u32 b32;
typedef float r32;
typedef double r64;

typedef u64 mem_size;