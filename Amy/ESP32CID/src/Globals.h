#ifndef Globals_H
#define Globals_H


/*
   Debug Options:
   0: LCD only. no debug, no printing. no softwareSerial declaration.
   1: Pure bit dump
   2: error and some information messages if outside the time critical loop. Standard debug mode.
   3: as 2 plus some Info during time critical sections
   4: as 3 plus some customisable statements.

   Be aware that too much printing can slow the parser to the extent that the demodulator
    output is dropped partially, leading to unpredicatable errors.
*/
#include "Arduino.h"
#define swSerial Serial




// set in main - run status/diagnostic variables
extern byte debugMode;
extern unsigned long callsThisRun;
extern uint32_t successCountLoop;



// #define ILI9341_BLACK       0x0000      /*   0,   0,   0 */
// #define ILI9341_NAVY        0x000F      /*   0,   0, 128 */
// #define ILI9341_DARKGREEN   0x03E0      /*   0, 128,   0 */
// #define ILI9341_DARKCYAN    0x03EF      /*   0, 128, 128 */
// #define ILI9341_MAROON      0x7800      /* 128,   0,   0 */
// #define ILI9341_PURPLE      0x780F      /* 128,   0, 128 */
// #define ILI9341_OLIVE       0x7BE0      /* 128, 128,   0 */
// #define ILI9341_LIGHTGREY   0xC618      /* 192, 192, 192 */
// #define ILI9341_DARKGREY    0x7BEF      /* 128, 128, 128 */
// #define ILI9341_BLUE        0x001F      /*   0,   0, 255 */
// #define ILI9341_GREEN       0x07E0      /*   0, 255,   0 */
// #define ILI9341_CYAN        0x07FF      /*   0, 255, 255 */
// #define ILI9341_RED         0xF800      /* 255,   0,   0 */
// #define ILI9341_MAGENTA     0xF81F      /* 255,   0, 255 */
// #define ILI9341_YELLOW      0xFFE0      /* 255, 255,   0 */
// #define ILI9341_WHITE       0xFFFF      /* 255, 255, 255 */
// #define ILI9341_ORANGE      0xFD20      /* 255, 165,   0 */
// #define ILI9341_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
// #define ILI9341_PINK        0xF81F




#endif
