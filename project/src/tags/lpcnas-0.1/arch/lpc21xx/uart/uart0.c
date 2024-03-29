#include <math.h>
#include "uart0.h"

uint32_t uart0IsTxBufferEmpty() {
    return 1;
}

void uart0Init() {
//    uint32_t divisor;

    U0LCR = BIT1 | BIT0 /* 8-bit words                            */
        | BIT7;             /* enable programming of divisor latches  */

    {
        uint32_t divaddval, mulval, divisor;
        double exact_rate, exact_divisor;
        double error, min_exact_rate, min_error = 1e300;
        uint32_t min_divaddval, min_mulval, min_divisor;

        for (divaddval=0; divaddval <= 15; divaddval++) {
            for (mulval=1; mulval <= 15; mulval++) {
                exact_divisor = (double) CLOCKS_PCLK / ( (double) UART0_BAUD_RATE
                        * 16.0 * ( 1.0 + ( (double) divaddval / (double) mulval ) ) );
                divisor = round(exact_divisor);
                exact_rate = (double) CLOCKS_PCLK / ( (double) divisor * 16.0 * ( 1.0
                            + ( (double) divaddval / (double) mulval ) ) );
                error = fabs(exact_rate - (double) UART0_BAUD_RATE );
                if (error < min_error) {
                    min_error = error;
                    min_exact_rate = exact_rate;
                    min_divaddval = divaddval;
                    min_mulval = mulval;
                    min_divisor = divisor;
                }
            }
        }
        U0DLM = (min_divisor & 0x0000FF00) >> 8;
        U0DLL = (min_divisor & 0x000000FF);
        U0FDR = min_divaddval | (min_mulval << 4);
    }

    U0LCR &= ~BIT7; /* disable programming of divisor latches */

    U0FCR = BIT0 | BIT1 | BIT2; /* FIFO control: enable & reset    */

    PINSEL0 &= ~(BIT1 | BIT3); /* select UART function               */
    PINSEL0 |= BIT0 | BIT2; /* for pins P0.0 and P0.1             */
}

void uart0SendByte(uint8_t c) {
    while (!(U0LSR & BIT5));
    U0THR = c;
}

/* Returns: character on success, -1 if no character is available */
int uart0GetByte(void)
{
    if (U0LSR & BIT0)                 // check if character is available
        return U0RBR;                     // return character

    return -1;
}

/* Returns: character on success, waits */
int uart0GetByteWait(void)
{
    while ( !(U0LSR & BIT0) ); // wait for character

    return U0RBR;              // return character
}

