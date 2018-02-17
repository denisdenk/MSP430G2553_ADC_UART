#include <msp430.h> 
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//typedef unsigned char     uint8_t;
//typedef unsigned short    uint16_t;
//typedef unsigned int      uint32_t;
//typedef unsigned long int uint64_t;

#define RXD	BIT1
#define TXD	BIT2

void UART_TX(char * tx_data);
void ConfigureADC(void);
void ConfigureUART(void);

uint16_t ADC_value = 0;
char Buf[100];

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;         	// Stop Watch dog timer

	BCSCTL1 = CALBC1_1MHZ;            	// Set DCO to 1 MHz
	DCOCTL = CALDCO_1MHZ;
	BCSCTL2 &= ~(DIVS_3);				// SMCLK = DCO = 1MHz
	P1SEL |= BIT3;						// Вход АЦП P1.3

	ConfigureUART();
	ConfigureADC();

  while(1)                          	// While 1 is equal to 1 (forever)
  {
	  	  __delay_cycles(10000);				// Wait for ADC Ref to settle
	  	  ADC10CTL0 |= ENC + ADC10SC;
	  	  __bis_SR_register(CPUOFF + GIE);
	  	  ADC_value = ADC10MEM;

	  	  sprintf(Buf, "ADC: %d\r\n", ADC_value);
          UART_TX(Buf);
  }

}
// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	__bic_SR_register_on_exit(CPUOFF);        // Return to active mode
}

void UART_TX(char * tx_data) 			// Define a function which accepts a character pointer to an array
{
    unsigned int i=0;
    while(tx_data[i]) 					// Increment through array, look for null pointer (0) at end of string
    {
        while ((UCA0STAT & UCBUSY)); 	// Wait if line TX/RX module is busy with data
        UCA0TXBUF = tx_data[i]; 		// Send out element i of tx_data array on UART bus
        i++; 							// Increment variable for array address
    }
}

void ConfigureADC(void)
{
	ADC10CTL1 = INCH_3 + ADC10DIV_3 ; // Channel 3, ADC10CLK/3
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE; // Vcc & Vss as reference, Sample and hold for 64 Clock cycles, ADC on, ADC interrupt enable
	ADC10AE0 |= BIT3;
}

void ConfigureUART(void)
{
	P1SEL = RXD + TXD ;                	// Select TX and RX functionality for P1.1 & P1.2
	P1SEL2 = RXD + TXD ;              	//

	UCA0CTL1 |= UCSSEL_2;             	// Have USCI use System Master Clock: AKA core clk 1MHz

	UCA0BR0 = 104;                    	// 1MHz 9600, see user manual
	UCA0BR1 = 0;                      	//

	UCA0MCTL = UCBRS0;                	// Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;               // Start USCI state machine
}
