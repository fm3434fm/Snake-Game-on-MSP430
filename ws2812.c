#include <msp430.h>
#include "ws2812.h"

// WS2812 takes GRB format


static const unsigned char colors[87][3] = {{0,0,0},    // define gradient where indices 0 and 86 are special colors (black and green)
                                                        // gradient taken from matplotlib colormap plasma
{13, 8, 135},
{22, 7, 139},
{29, 6, 142},
{36, 6, 145},
{42, 5, 148},
{48, 5, 150},
{53, 4, 153},
{58, 4, 155},
{63, 4, 157},
{69, 3, 159},
{73, 3, 161},
{78, 2, 162},
{83, 2, 164},
{88, 1, 165},
{93, 1, 166},
{98, 0, 167},
{102, 0, 168},
{107, 0, 169},
{112, 0, 169},
{116, 1, 169},
{121, 1, 169},
{125, 3, 169},
{130, 4, 168},
{134, 6, 167},
{138, 9, 166},
{143, 12, 165},
{147, 15, 163},
{151, 19, 161},
{155, 22, 159},
{159, 25, 157},
{163, 29, 155},
{167, 32, 152},
{170, 35, 150},
{174, 39, 147},
{177, 42, 145},
{181, 46, 142},
{184, 49, 139},
{187, 52, 136},
{191, 56, 133},
{194, 59, 130},
{197, 63, 128},
{200, 66, 125},
{202, 69, 122},
{205, 73, 119},
{208, 76, 117},
{211, 80, 114},
{213, 83, 111},
{216, 87, 109},
{218, 90, 106},
{221, 93, 103},
{223, 97, 101},
{226, 100, 98},
{228, 104, 96},
{230, 108, 93},
{232, 111, 90},
{234, 115, 88},
{236, 119, 85},
{238, 122, 83},
{240, 126, 80},
{242, 130, 77},
{244, 134, 75},
{245, 138, 72},
{247, 142, 70},
{248, 146, 67},
{249, 150, 64},
{250, 154, 62},
{251, 158, 59},
{252, 163, 57},
{253, 167, 54},
{254, 171, 52},
{254, 176, 49},
{254, 180, 47},
{255, 185, 45},
{255, 190, 43},
{254, 194, 41},
{254, 199, 39},
{254, 204, 38},
{253, 209, 37},
{252, 214, 36},
{251, 219, 36},
{249, 224, 37},
{248, 229, 38},
{246, 234, 39},
{244, 239, 39},
{242, 245, 38},
{0,255,0}
};

// Initializes everything needed to use this library. This clears the strip.
void initStrip() {
    P1SEL |= OUTPUT_PIN;    // configure output pin as SPI output
    P1SEL2 |= OUTPUT_PIN;
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC; // 3-pin, MSB, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2;   // SMCLK source (16 MHz)
    UCB0BR0 = 3;            // 16 MHz / 3 = .1875 us per bit
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;   // Initialize USCI state machine
//    clearStrip();           // clear the strip
}


// Send colors to the strip and show them. Disables interrupts while processing.
void showStrip(unsigned char (*array)[16][16]) {
    __bic_SR_register(GIE);  // disable interrupts

    // send RGB color for every LED
    unsigned char x, y;
    for (x = 0; x < 16; x++) {
        for (y = 0; y < 16; y++) {
            unsigned char ind;
            if (x & 0x01 == 1) {
                ind = (*array)[x][y];
            } else {
                ind = (*array)[x][15-y];
            }

            u_char g = (u_char) colors[ind][1] >> 2; // get G color for this LED
            u_char r = (u_char) colors[ind][0] >> 2; // get R color for this LED
            u_char b = (u_char) colors[ind][2] >> 2; // get B color for this LED

            // send green, then red, then blue
            u_char mask = 0x80;    // b1000000
            // check each of the 8 bits
            while (mask != 0) {
                while (!(IFG2 & UCB0TXIFG));    // wait to transmit
                if (g & mask) {        // most significant bit first
                    UCB0TXBUF = HIGH_CODE;  // send 1
                } else {
                    UCB0TXBUF = LOW_CODE;   // send 0
                }

                mask >>= 1;  // check next bit
            }

            mask = 0x80;    // b1000000
            // check each of the 8 bits
            while (mask != 0) {
                while (!(IFG2 & UCB0TXIFG));    // wait to transmit
                if (r & mask) {        // most significant bit first
                    UCB0TXBUF = HIGH_CODE;  // send 1
                } else {
                    UCB0TXBUF = LOW_CODE;   // send 0
                }

                mask >>= 1;  // check next bit
            }

            mask = 0x80;    // b1000000
            // check each of the 8 bits
            while (mask != 0) {
                while (!(IFG2 & UCB0TXIFG));    // wait to transmit
                if (b & mask) {        // most significant bit first
                    UCB0TXBUF = HIGH_CODE;  // send 1
                } else {
                    UCB0TXBUF = LOW_CODE;   // send 0
                }

                mask >>= 1;  // check next bit
            }
        }
    }

    // send RES code for at least 50 us (800 cycles at 16 MHz)
    _delay_cycles(800);

    __bis_SR_register(GIE);    // enable interrupts
}

