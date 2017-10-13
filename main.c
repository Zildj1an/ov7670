#define F_CPU 16000000UL
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "ov7670.h"

static inline void serialWrB(uint8_t dat)
{
    while (!(UCSR0A & (1 << UDRE0)))
        ; //wait for byte to transmit
    UDR0 = dat;
    while (!(UCSR0A & (1 << UDRE0)))
        ; //wait for byte to transmit
}

static void StringPgm(const char *str)
{
    do
    {
        serialWrB(pgm_read_byte_near(str));
    } while (pgm_read_byte_near(++str));
}

static void captureImg(uint16_t wg, uint16_t hg)
{
    uint16_t wg2, hg2, x, y;
    uint16_t buf[20 * 15];
    uint16_t *b;
    uint8_t pix = 0;

    memset(buf, 0, sizeof(buf));

    //Wait for vsync it is on pin 3 (counting from 0) portD
    while (!(PIND & 8))
        ; //wait for high
    while ((PIND & 8))
        ; //wait for low

    hg2 = hg;
    while (hg2--)
    {
        b = buf + ((hg - hg2) / 16 * 20);
        wg2 = wg;
        while (wg2--)
        {
            // Get Y (luminance)
            while ((PIND & 4))
                ; //wait for low
            // b[(wg - wg2) / 64] += (PINC & 15) | (PIND & 240);
            // *b = (PINC & 15) | (PIND & 240);
            pix = (PINC & 15) | (PIND & 240);
            // *b += (PINC & 15) | (PIND & 240);
            // b += !(wg2 & 0x1F);
            // UDR0 = ((*b2++) >> (int)2) + 32;
            while (!(PIND & 4))
                ; //wait for high

            if (hg2 % 4 == 0)
                UDR0 = (pix >> 3) + '0';

            // Ignore UV (Chrominance)
            while ((PIND & 4))
                ; //wait for low
            while (!(PIND & 4))
                ; //wait for high
        }
        while (!(UCSR0A & (1 << UDRE0)))
            ; //wait for byte to transmit
        if (hg2 % 4 == 0)
        {
            UDR0 = '\n';
            while (!(UCSR0A & (1 << UDRE0)))
                ; //wait for byte to transmit
        }
    }

    UDR0 = '\n';
    while (!(UCSR0A & (1 << UDRE0)))
        ; //wait for byte to transmit
    UDR0 = '\n';

    // for (y = 0; y < 15; y++)
    // {
    //     for (x = 0; x < 20; x++)
    //     {
    //         UDR0 = (uint8_t)(buf[y * 20 + x] >> 0) + ' ';
    //         while (!(UCSR0A & (1 << UDRE0)))
    //             ; //wait for byte to transmit
    //     }
    //     UDR0 = '\n';
    //     while (!(UCSR0A & (1 << UDRE0)))
    //         ; //wait for byte to transmit
    // }
}

int main(void)
{
    cli(); //disable interrupts
    /* Setup the 8mhz PWM clock 
	 * This will be on pin 11*/
    DDRB |= (1 << 3); //pin 11
    ASSR &= ~(_BV(EXCLK) | _BV(AS2));
    TCCR2A = (1 << COM2A0) | (1 << WGM21) | (1 << WGM20);
    TCCR2B = (1 << WGM22) | (1 << CS20);
    OCR2A = 0;    //(F_CPU)/(2*(X+1))
    DDRC &= ~15;  //low d0-d3 camera
    DDRD &= ~252; //d7-d4 and interrupt pins
    _delay_ms(300);
    //set up twi for 100khz
    TWSR &= ~3; //disable prescaler for TWI
    TWBR = 72;  //set to 100khz
    //enable serial
    UBRR0H = 0;
    UBRR0L = 1;                           //0 = 2M baud rate. 1 = 1M baud. 3 = 0.5M. 7 = 250k 207 is 9600 baud rate.
    UCSR0A |= 2;                          //double speed aysnc
    UCSR0B = (1 << RXEN0) | (1 << TXEN0); //Enable receiver and transmitter
    UCSR0C = 6;                           //async 1 stop bit 8bit char no parity bits
    camInit();
    setRes(QVGA);
    setColorSpace(YUV422);
    wrReg(0x11, 3);
    /* If you are not sure what value to use here for the divider (register 0x11)
	 * Values I have found to work raw vga 25 qqvga yuv422 12 qvga yuv422 21
	 * run the commented out test below and pick the smallest value that gets a correct image */
    while (1)
    {
        /* captureImg operates in bytes not pixels in some cases pixels are two bytes per pixel
		 * So for the width (if you were reading 640x480) you would put 1280 if you are reading yuv422 or rgb565 */
        // uint8_t x = 33; //Uncomment this block to test divider settings note the other line you need to uncomment
        // do
        // {
        // wrReg(0x11, x);
        // _delay_ms(1000);
        captureImg(320, 240);
        // } while (--x); //Uncomment this line to test divider settings
    }
}