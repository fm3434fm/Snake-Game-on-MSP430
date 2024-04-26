#include <msp430g2452.h>
#include <stdbool.h>
#include <msp430.h>
#include "ws2812.h"
#include "rand.h"

#define ONE_SEC 10600   // set to approximately 1 second
#define MAXLEN 70       // set max length


// game state logic
unsigned char state = 1;        // game state. 1 = starting screen, 0 = during game, 2 = intermediate state
char snake[MAXLEN][2];          // sets locations of the snake. from (index) to (index-snakelen) are valid
unsigned char snakelen = 1;     // length of snake
unsigned char index = 0;        // index of the head of the snake
unsigned char foodx = 0;        // food x location
unsigned char foody = 0;        // food y location
unsigned char board[16][16] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},   // 16 x 16 game board
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                             {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
                             };

unsigned char funnynum = 0; // iterator that generates the starting animation
unsigned int seed;          // for random number generation
unsigned char dir = 0;      // dir x 3 = clock position direction
unsigned char lastdir = 0;  // indicates most recent previous direction

unsigned char count = 0;    // game state updates every 4 iterations of count


void init_wdt(void){
    BCSCTL3 |= LFXT1S_2;     // ACLK = VLO
    WDTCTL = WDT_ADLY_16;    // WDT 16ms, ACLK, interval timer
    IE1 |= WDTIE;            // Enable WDT interrupt
}



void makefood() {
    // if d1 or d2 are 0, spawn location is valid
    bool d1 = 1;
    bool d2 = 1;

    // while the food spawn is invalid
    while (d1 & d2) {
        // rng a random place on the board
        foodx = ((seed >> 7) & 0x0F);
        foody = ((seed >> 12) & 0x0F);
        seed = prand(seed); // generate next pseudorandom value

        d1 = 0;
        d2 = 0;
        // if food spawned on snake, try again. check both (x,y) and (y,x) for collision
        unsigned char i;
        unsigned char wrapindex = index;
        for (i = snakelen; i > 0; i--) {        // iterates through the snake to check if food spawned on it
                                                // snakelen indices from index and below, circular, are valid locations of snake
            if (foodx == snake[wrapindex][0] && foody == snake[wrapindex][1]) {     // checking if (x, y) invalid
                d1 = 1;
            }
            if (foody == snake[wrapindex][0] && foodx == snake[wrapindex][1]) {     // checking if (y, x) invalid
                d2 = 1;
            }

            if (wrapindex == 0) {           // snake array is circular; go back to top once end is reached
                wrapindex = MAXLEN-1;
            } else {
                wrapindex--;
            }
        }
    }
    // if the (y,x) was valid, use it for food position
    if (d2 == 0) {
        unsigned char temp = foodx; // swap x and y
        foodx = foody;
        foody = temp;
    }
}

void startgame() {
    // reset game variables
    dir = 0;    // initializes snake direction to 0 (no movement)

    int i;
    snake[0][0] = 8;
    snake[0][1] = 8;
    for (i = 1; i < MAXLEN; i++) {  // reset snake to length 2
        snake[i][0] = 18;
        snake[i][1] = 18;
    }
    index = 0;      // reset index of head of snake
    snakelen = 1;   // snake to length 2 (snakelen+1 is real length)

    unsigned char x,y;
    for (x = 0; x < 16; x++) {
        for (y = 0; y < 16; y++) {
            board[x][y] = 0;        // clear game board
        }
    }

    makefood();     // spawn food
}

void printboard() {
    // place food and valid indices of snake onto board
    board[foodx][foody] = 86;
    unsigned char i;
    unsigned char wrapindex = index;
    for (i = snakelen; i > 0; i--) {    // iterate through snake; snakelen indices from index and below, circular, are valid locations of snake
        board[snake[wrapindex][0]][snake[wrapindex][1]] = snakelen-i+1; // set color of snake segments

        if (wrapindex == 0) {       // snake array is circular; go back to top once end is reached
            wrapindex = MAXLEN-1;
        } else {
            wrapindex--;
        }
    }

    showStrip(&board);  // call LED array driver to display the board


    // clear board for next print

    board[foodx][foody] = 0;            // remove food from board
    for (i = snakelen; i > 0; i--) {    // same loop as above, clear the board of snake for next print
        board[snake[wrapindex][0]][snake[wrapindex][1]] = 0;

        if (wrapindex == 0) {       // snake array is circular; go back to top once end is reached
            wrapindex = MAXLEN-1;
        } else {
            wrapindex--;
        }
    }
}

void main(void)
{
    seed = rand();      // generate true random seed

    WDTCTL = WDTPW + WDTHOLD;   // Stop WDT

    BCSCTL3 |= LFXT1S_2;        // ACLK = VLO

    // INITIALIZE RGB
    // configure clock to 16 MHz
    BCSCTL1 = CALBC1_16MHZ;    // DCO = 16 MHz
    DCOCTL = CALDCO_16MHZ;

    // initialize LED strip
    initStrip();

    // show the board to begin with
    showStrip(&board);

    // CONFIGURE PERIODIC TIMERS
    TA0CTL = TASSEL_1 + MC_2;   // ACLK, upmode, counter interrupt enable
    TA0CCR0 = ONE_SEC >> 4;     // Register 0 counter value to trigger interrupt
    TA0CCTL0 = CCIE;            // CCR0 interrupt enabled


    // CONFIGURE BUTTONS
    // set 2.0, 2.2, 2.3, 2.4 as button input
    P2DIR &= ~(BIT0 + BIT2 + BIT3 + BIT4);      // configure pins as inputs
    P2IES |= (BIT0 + BIT2 + BIT3 + BIT4);       // set to negedge of button

    P2REN |= (BIT0 + BIT2 + BIT3 + BIT4);       // sets pulldown resistor
    P2IE |= (BIT0 + BIT2 + BIT3 + BIT4);        // interrupts enabled



    // SETUP GAME
    startgame();    // initialize game
    state = 1;      // go to starting screen

    while(1) {
        count++;
        if (count > 3) {
            count = 0;
        }

        if (count == 3) {       // every 1/4 second, update game board
            if (state == 1) {
                // if you press a button, start the game
                if (dir != 0) {
                    unsigned char x,y;
                    for (x = 0; x < 16; x++) {
                        for (y = 0; y < 16; y++) {
                            board[x][y] = 0;  // clear game board for next time
                        }
                    }
                    state = 0;

                } else { // else, play starting animation
                    unsigned char x, y;
                    for (x = 0; x < 16; x++) {
                        for (y = 0; y < 16; y++) {
                            board[x][y] = (x*y *2 + funnynum) % 85 + 1; // generate a wave pattern that repeats
                        }
                    }

                    showStrip(&board);  // show the pattern
                    funnynum += 5;      // iterator that helps generate wave
                    if (funnynum == 85) {
                        funnynum = 0;
                    }
                }

            // INPUT MODE
            } else if (state == 0) {

                // button input logic
                unsigned char next = index + 1; // index of next segment of snake
                if (next == MAXLEN) {           // snake array is circular, if snake reaches the end, use front of array again
                    next = 0;
                }

                switch (dir) {
                    case 1:
                        // snake moves right

                        // out of bounds
                        if (snake[index][0] == 0) {
                            state = 2;      // if snake is already at the right side of board, end the game
                            break;
                        }
                        snake[next][0] = snake[index][0] - 1;   // else, move snake to the right
                        snake[next][1] = snake[index][1];       // y position doesn't change

                        // set the direction you came from and can't move there
                        lastdir = 3;

                        break;
                    case 2:
                        // snake moves down

                        // out of bounds
                        if (snake[index][1] == 15) {
                            state = 2;      // if snake is already at the bottom side of board, end the game
                            break;
                        }
                        snake[next][0] = snake[index][0];       // x position doesn't change
                        snake[next][1] = snake[index][1] + 1;   // else, move snake to down

                        // set the direction you came from and can't move there
                        lastdir = 4;

                        break;
                    case 3:
                        // snake moves left

                        // out of bounds
                        if (snake[index][0] == 15) {
                            state = 2;      // if snake is already at the left side of board, end the game
                            break;
                        }
                        snake[next][0] = snake[index][0] + 1;   // else, move snake to the left
                        snake[next][1] = snake[index][1];       // y position doesn't change

                        // set the direction you came from and can't move there
                        lastdir = 1;

                        break;
                    case 4:
                        // snake moves up

                        // out of bounds
                        if (snake[index][1] == 0) {
                            state = 2;      // if snake is already at the top side of board, end the game
                            break;
                        }
                        snake[next][0] = snake[index][0];       // x position doesn't change
                        snake[next][1] = snake[index][1] - 1;   // else, move snake up

                        // set the direction you came from and can't move there
                        lastdir = 2;

                        break;
                }

                // scan the length of the snake-1 to see if collision occurs
                unsigned char i;
                unsigned char wrapindex = index;
                for (i = snakelen; i > 1; i--) {
                    if (snake[next][0] == snake[wrapindex][0] && snake[next][1] == snake[wrapindex][1]) {
                        state = 2;  // if the next location of the snake's head is the same location one of its body segments, end the game
                    }
                    if (wrapindex == 0) {       // snake array is circular; go back to top once end is reached
                        wrapindex = MAXLEN-1;
                    } else {
                        wrapindex--;
                    }
                }

                index = next;
                // if snake ate food, increase length
                if (snake[next][0] == foodx && snake[next][1] == foody) {
                    snakelen++;
                    makefood();     // respawn food elsewhere
                }

                printboard();       // print the board after updating game state

            // GAME OVER, BACK TO START SCREEN
            } else {
                startgame();        // reset the game for next round
                printboard();
                state = 1;          // go to starting animation
            }


        }

        // refresh LEDs after each round of updates


        __low_power_mode_1(); // Enter LPM1
    }
}


// Timer 0 interrupt service routine for register 0
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A0 (void)
#else
#error Compiler not supported!
#endif
{

    TA0CCR0 += ONE_SEC>>4;                      // wake processor 16 times per second to perform logic
    __bic_SR_register_on_exit(LPM1_bits);       // Clear LPM1 bits from 0(SR)

}


// BUTTON HANDLING
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    // 2.0 is right
    if ((P2IFG & BIT0)) {
        if (lastdir != 1) dir = 1;      // flag that right button was pressed
    }
    // 2.2 is down
    if ((P2IFG & BIT2)) {
        if (lastdir != 2) dir = 2;      // flag that down button was pressed
    }

    // 2.3 is up
    if ((P2IFG & BIT3)) {
        if (lastdir != 4) dir = 4;      // flag that up button was pressed
    }
    // 2.4 is left
    if ((P2IFG & BIT4)) {
        if (lastdir != 3) dir = 3;      // flag that left button was pressed
    }

    P2IFG &= ~(BIT0 + BIT2 + BIT3 + BIT4); // clear interrupt
}

