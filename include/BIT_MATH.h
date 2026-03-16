#ifndef BIT_MATH_H
#define BIT_MATH_H

// Single-bit operations
#define SET_BIT(REG, BIT)    ((REG) |= (1U << (BIT)))
#define CLEAR_BIT(REG, BIT)  ((REG) &= ~(1U << (BIT)))
#define TOGGLE_BIT(REG, BIT) ((REG) ^= (1U << (BIT)))
#define READ_BIT(REG, BIT)   (((REG) >> (BIT)) & 1U)

// Multi-bit operations with mask
#define SET_MASK(REG, MASK)   ((REG) |= (MASK))
#define CLEAR_MASK(REG, MASK) ((REG) &= ~(MASK))

// Bit-field write: clear region first, then set value
#define WRITE_BITS(REG, MASK, VAL)  ((REG) = (((REG) & ~(MASK)) | ((VAL) & (MASK))))

// Bit-field read
#define READ_BITS(REG, MASK) ((REG) & (MASK))

#endif