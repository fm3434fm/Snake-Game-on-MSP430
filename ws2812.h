// Configuration - SET THESE!
#define OUTPUT_PIN  (0x80)  // Set P1.7 as output

// Useful typedefs
typedef unsigned char u_char;	// 8 bit
typedef unsigned int u_int;     // 16 bit

// Transmit codes
#define HIGH_CODE   (0xF0)      // b11110000
#define LOW_CODE    (0xC0)      // b11000000

// Configure processor to output to data strip
void initStrip(void);

// Send colors to the strip and show them. Disables interrupts while processing.
void showStrip(unsigned char (*array)[16][16]);
