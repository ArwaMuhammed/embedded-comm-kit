#include "MemScanner.h"
#include <stdio.h>

/* ------------------------------------------------------------------ */
/*  Internal helper: cast any pointer to a byte pointer               */
/* ------------------------------------------------------------------ */
/* We define a local macro so every function casts the same safe way. */
/* This is the ONLY place the cast appears; all arithmetic is in u8*. */
#define TO_BYTE_PTR(p) ((u8 *)(p))

/* ================================================================== */
/*  1. Read Byte                                                       */
/* ================================================================== */
u8 MemScanner_ReadByte(Ptr base, u32 offset)
{
    /*
     * Cast the generic pointer to u8* so we can do byte-level arithmetic,
     * then return the byte sitting at (base + offset).
     */
    return TO_BYTE_PTR(base)[offset];
}

/* ================================================================== */
/*  2. Read Half-Word (16-bit, Little-Endian)                         */
/* ================================================================== */
u16 MemScanner_ReadHalfWord(Ptr base, u32 offset)
{
    /*
     * Little-Endian layout in memory:
     *   [offset + 0] = low byte  (bits  7:0)
     *   [offset + 1] = high byte (bits 15:8)
     *
     * We read both bytes individually and combine them manually.
     * This avoids any alignment issues that a direct (u16*) cast could cause
     * on strict-alignment architectures.
     */
    u8  low_byte  = TO_BYTE_PTR(base)[offset];
    u8  high_byte = TO_BYTE_PTR(base)[offset + 1U];

    return (u16)(((u16)high_byte << 8U) | (u16)low_byte);
}

/* ================================================================== */
/*  3. Read Word (32-bit, Little-Endian)                              */
/* ================================================================== */
u32 MemScanner_ReadWord(Ptr base, u32 offset)
{
    /*
     * Little-Endian layout in memory:
     *   [offset + 0] = bits  7:0  (lowest byte)
     *   [offset + 1] = bits 15:8
     *   [offset + 2] = bits 23:16
     *   [offset + 3] = bits 31:24 (highest byte)
     *
     * Same manual byte-by-byte reconstruction to stay alignment-safe.
     */
    u8 b0 = TO_BYTE_PTR(base)[offset];
    u8 b1 = TO_BYTE_PTR(base)[offset + 1U];
    u8 b2 = TO_BYTE_PTR(base)[offset + 2U];
    u8 b3 = TO_BYTE_PTR(base)[offset + 3U];

    return ((u32)b3 << 24U) |
           ((u32)b2 << 16U) |
           ((u32)b1 <<  8U) |
            (u32)b0;
}

/* ================================================================== */
/*  4. Write Byte                                                      */
/* ================================================================== */
void MemScanner_WriteByte(Ptr base, u32 offset, u8 value)
{
    /*
     * Cast to u8* and write the value directly at (base + offset).
     */
    TO_BYTE_PTR(base)[offset] = value;
}

/* ================================================================== */
/*  5. Hex Dump                                                        */
/* ================================================================== */
void MemScanner_HexDump(Ptr base, u32 size)
{
    /*
     * Walk through every byte in the region and print it as a two-digit
     * uppercase hex value separated by spaces.
     * Format:  [HexDump] 0A 1B 2C 3D
     */
    u32 i;

    printf("[HexDump] ");

    for (i = 0U; i < size; i++)
    {
        printf("%02X", (u32)TO_BYTE_PTR(base)[i]);

        /* Print a space after every byte except the last one */
        if (i < (size - 1U))
        {
            printf(" ");
        }
    }

    printf("\n");
}

/* ================================================================== */
/*  6. Memory Fill                                                     */
/* ================================================================== */
void MemScanner_MemFill(Ptr base, u32 size, u8 value)
{
    /*
     * Iterate over every byte in the region and overwrite it with value.
     * We do this manually (not using memset) so we only depend on our own
     * types and the project does not silently use raw C library functions.
     */
    u32 i;

    for (i = 0U; i < size; i++)
    {
        TO_BYTE_PTR(base)[i] = value;
    }
}

/* ================================================================== */
/*  7. Memory Compare                                                  */
/* ================================================================== */
s32 MemScanner_MemCompare(Ptr base1, Ptr base2, u32 size)
{
    /*
     * Compare the two regions byte-by-byte.
     *
     * Return value:
     *   0   → regions are identical
     *   N   → 1-indexed offset of the first byte that differs
     *          e.g. if byte at index 2 (0-indexed) differs, return 3
     */
    u32 i;

    for (i = 0U; i < size; i++)
    {
        if (TO_BYTE_PTR(base1)[i] != TO_BYTE_PTR(base2)[i])
        {
            /* Return 1-indexed position of the first mismatch */
            return (s32)(i + 1U);
        }
    }

    return 0;  /* All bytes matched */
}

/* ================================================================== */
/*  8. Find Byte                                                       */
/* ================================================================== */
s32 MemScanner_FindByte(Ptr base, u32 size, u8 value)
{
    /*
     * Scan the region for the first byte equal to value.
     *
     * Return value:
     *   offset (0-indexed) of the first match
     *   -1 if value is not found anywhere in the region
     */
    u32 i;

    for (i = 0U; i < size; i++)
    {
        if (TO_BYTE_PTR(base)[i] == value)
        {
            return (s32)i;  /* 0-indexed offset */
        }
    }

    return -1;  /* Not found */
}