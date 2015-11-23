//
//  BitUtilities.h
//  hydra
//
//  Created by Kurt Arnlund on 11/14/15.
//
//

#ifndef BitUtilities_h
#define BitUtilities_h

#define HIGH_BYTE(val)      ((val >> 8) & 0xFF)
#define LOW_BYTE(val)       (val & 0xFF)

#define LSB_NIBBLE(val)     (val & 0x0F)
#define MSB_NIBBLE(val)     (val & 0xF0)

#define BIT(n)              (0x01 << (n - 1))

#define BIT_ON(value, n)    value |= BIT(n)

#endif /* BitUtilities_h */
