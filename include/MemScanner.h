#ifndef MEMSCANNER_H
#define MEMSCANNER_H

#include "STD_TYPES.h"

/*
 * MemScanner.h
 * Memory inspection and manipulation module.
 * All functions accept a generic pointer (Ptr / void*) and work on raw bytes.
 */

/*
 * MemScanner_ReadByte
 * Read a single byte from (base + offset).
 * Parameters:
 *   base   - generic pointer to the start of the memory region
 *   offset - byte offset from base
 * Returns: the byte value at that location
 */
u8 MemScanner_ReadByte(Ptr base, u32 offset);

/*
 * MemScanner_ReadHalfWord
 * Read a 16-bit little-endian value from (base + offset).
 * Parameters:
 *   base   - generic pointer to the start of the memory region
 *   offset - byte offset from base (points to the low byte)
 * Returns: the 16-bit value
 */
u16 MemScanner_ReadHalfWord(Ptr base, u32 offset);

/*
 * MemScanner_ReadWord
 * Read a 32-bit little-endian value from (base + offset).
 * Parameters:
 *   base   - generic pointer to the start of the memory region
 *   offset - byte offset from base (points to the lowest byte)
 * Returns: the 32-bit value
 */
u32 MemScanner_ReadWord(Ptr base, u32 offset);

/*
 * MemScanner_WriteByte
 * Write a single byte value into memory at (base + offset).
 * Parameters:
 *   base   - generic pointer to the start of the memory region
 *   offset - byte offset from base
 *   value  - byte value to write
 */
void MemScanner_WriteByte(Ptr base, u32 offset, u8 value);

/*
 * MemScanner_HexDump
 * Print 'size' bytes starting at base, formatted as hex pairs.
 * Example output:  [HexDump] 0A 1B 2C 3D 4E 5F
 * Parameters:
 *   base - generic pointer to the start of the memory region
 *   size - number of bytes to dump
 */
void MemScanner_HexDump(Ptr base, u32 size);

/*
 * MemScanner_MemFill
 * Fill 'size' bytes starting at base with the given byte value.
 * Parameters:
 *   base  - generic pointer to the start of the memory region
 *   size  - number of bytes to fill
 *   value - byte value to write into every location
 */
void MemScanner_MemFill(Ptr base, u32 size, u8 value);

/*
 * MemScanner_MemCompare
 * Compare two memory regions byte-by-byte.
 * Parameters:
 *   base1 - generic pointer to the first region
 *   base2 - generic pointer to the second region
 *   size  - number of bytes to compare
 * Returns:
 *   0              if both regions are identical
 *   N (1-indexed)  the offset of the first differing byte (starts at 1)
 */
s32 MemScanner_MemCompare(Ptr base1, Ptr base2, u32 size);

/*
 * MemScanner_FindByte
 * Search 'size' bytes starting at base for the first occurrence of value.
 * Parameters:
 *   base  - generic pointer to the start of the memory region
 *   size  - number of bytes to search
 *   value - byte value to look for
 * Returns:
 *   offset (0-indexed) of the first match, or -1 if not found
 */
s32 MemScanner_FindByte(Ptr base, u32 size, u8 value);

#endif /* MEMSCANNER_H */