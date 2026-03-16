#include "STD_TYPES.h"
#include "BIT_MATH.h"
#include <stdio.h>

int main(void)
{   //test STD_TYPES and BIT_MATH files
    u8 reg = 0x00U;//0000 0000

    SET_BIT(reg, 0);//0000 0001
    SET_BIT(reg, 3);//0000 1001
    printf("After setting bits: 0x%02X\n", reg);// must be 0x09

    TOGGLE_BIT(reg, 3);//0000 0001
    printf("After toggling bit 3: 0x%02X\n", reg);// must be 0x01

    u8 bit_val = READ_BIT(reg, 0);
    printf("Read bit 0: %u\n", bit_val);// 1

    return 0;
}