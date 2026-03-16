#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Fixed-width integer types
typedef uint8_t  u8;
typedef int8_t   s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;

// Boolean type
typedef bool boolean;

// Standard function return type
typedef enum
{
    SUCCESS = 0U,
    FAILURE = 1U
} Std_ReturnType;

// Generic pointer type
typedef void* Ptr;

#endif