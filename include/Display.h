#include <Arduino.h>

const uint8_t SEG_PAY[] = {
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,         // P
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G, // A
    SEG_B | SEG_C | SEG_D | SEG_G | SEG_F ,       // n
    0x0                                            //
};

const uint8_t SEG_DONE[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,         // d
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G          // E
};
const uint8_t SEG_PAn1[] = {
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,         // P
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G, // A
    SEG_C | SEG_E | SEG_G,                         // n
    SEG_C | SEG_B                                  // 1
};
const uint8_t SEG_PAn2[] = {
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,         // P
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G, // A
    SEG_C | SEG_E | SEG_G,                         // n
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G          // 2
};
const uint8_t SEG_PAn3[] = {
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,         // P
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G, // A
    SEG_C | SEG_E | SEG_G,                         // n
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G          // 3
};

const uint8_t SEG_P[] = {
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G, // P
    0x0,                                   // r
    0x0,                                   // O
    0x0                                    // g
};

const uint8_t SEG_on[] = {
    0x0,                                           //
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    0x0                                            //
};

const uint8_t SEG_PROG[] = {
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,         // P
    SEG_E | SEG_G,                                 // r
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G  // g
};
const uint8_t SEG_DOOR[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,         // d
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_E | SEG_G                                  // R
};
const uint8_t SEG_E2[] = {
    SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G, // 2
    SEG_G};
const uint8_t SEG_E1[] = {
    SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
    SEG_B | SEG_C,                         // 1
    SEG_G};
const uint8_t SEG_E3[] = {
    SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G, // 3
    SEG_G};
const uint8_t SEG_E4[] = {
    SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
    SEG_B | SEG_C | SEG_F | SEG_G,         // 4
    SEG_G};
const uint8_t SEG_E5[] = {
    SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G, // 5
    SEG_G};
const uint8_t SEG_E6[] = {
    SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,         // E
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // 6
    SEG_G};
const uint8_t SEG_c1[] = {
    SEG_G,
    SEG_D | SEG_E | SEG_G, // c
    SEG_B | SEG_C,         // 1
    SEG_G};
const uint8_t SEG_c2[] = {
    SEG_G,
    SEG_D | SEG_E | SEG_G,                 // c
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G, // 2
    SEG_G};
const uint8_t SEG_c3[] = {
    SEG_G,
    SEG_D | SEG_E | SEG_G,                 // c
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G, // 3
    SEG_G};

const uint8_t SEG_c4[] = {
    SEG_G,
    SEG_D | SEG_E | SEG_G,         // c
    SEG_B | SEG_C | SEG_F | SEG_G, // 4
    SEG_G};

const uint8_t SEG_t1[] = {
    SEG_G,
    SEG_D | SEG_E | SEG_F | SEG_G, // t
    SEG_B | SEG_C,                 // 1
    SEG_G};
const uint8_t SEG_t2[] = {
    SEG_G,
    SEG_D | SEG_E | SEG_F | SEG_G,         // t
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G, // 2
    SEG_G};
const uint8_t SEG_t3[] = {
    SEG_G,
    SEG_D | SEG_E | SEG_F | SEG_G,         // t
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G, // 3
    SEG_G};
const uint8_t SEG_n1[] = {
    SEG_G,
    SEG_C | SEG_E | SEG_G, // n
    SEG_B | SEG_C,         // 1
    SEG_G};
const uint8_t SEG_n2[] = {
    SEG_G,
    SEG_C | SEG_E | SEG_G,                 // n
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G, // 2
    SEG_G};
const uint8_t SEG_n3[] = {
    SEG_G,
    SEG_C | SEG_E | SEG_G,                 // n
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G, // 3
    SEG_G};

const uint8_t SEG_SEG[] = {
    SEG_G,
    SEG_G, // c
    SEG_G, // 1
    SEG_G};