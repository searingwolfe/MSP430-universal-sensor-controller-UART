#include <msp430g2553.h>
#include <stdio.h>
#include <string.h>

//Closed Loop: Temperature Sensor Fan Controller

/*** Initialize UART-Related Code ***/
volatile unsigned int current = 0; // store the currently read ADC10MEM at the beginning of the loop.
volatile unsigned int timer_count = 0;
static char data;
char buffer[32];
char buffer2[32];
void init_uart();
void init_timer();
void start_conversion();
void UARTSendArray(unsigned char *TxArray, unsigned char ArrayLength);

/*** Initialize Temperature Variables ***/
int error = 0; // subtract current from current to determine fan speed.
int SETtemperror = 370; //the default room temperature for making the error temperature values.

void (*uart_rx_isr_ptr)(unsigned char c);

void uart_set_rx_isr_ptr(void (*isr_ptr)(unsigned char c)) 
{
	uart_rx_isr_ptr = isr_ptr;
}

void uart_putc(unsigned char c)             // print to PC over TX one character at a time.
{
	while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  	UCA0TXBUF = c;                    		// TX
}

void uart_puts(const char *str)             // print any string message to PC with this function.
{
     while(*str) uart_putc(*str++);
}

void uart_rx_isr(unsigned char c) { // UART RX Commands go here.
	uart_putc(c);
	P1OUT ^= BIT0;		// toggle P1.0 (red led)
	
	switch(c) {
        case 'a': {
            SETtemperror = 460;
            uart_puts((char *)"\n\rDesired Temperature is now: 460\n\r");
        }
        case 'b': {
            SETtemperror = 660;
            uart_puts((char *)"\n\rDesired Temperature is now: 660\n\r");
        }
        case 'c': {
            SETtemperror = 860;
            uart_puts((char *)"\n\rDesired Temperature is now: 860\n\r");
        }
        break;
    }
}

void init_uart() 
{
	P1SEL  = BIT1 + BIT2;	// P1.1 = RXD, P1.2 = TXD
	P1SEL2 = BIT1 + BIT2;	// P1.1 = RXD, P1.2 = TXD
	UCA0CTL1 |= UCSSEL_2;	// SMCLK
	UCA0BR0 = 104;			// see baud rate divider above
	UCA0BR1 = 0;
	UCA0MCTL = UCBRS0;		// modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;	// ** initialize USCI state machine **
	IE2 |= UCA0RXIE;		// Enable USCI_A0 TX interrupt
}

void UARTSendArray(unsigned char *TxArray, unsigned char ArrayLength) 
{
    while(ArrayLength--) { // Loop until StringLength == 0 and post decrement
        while(!(IFG2 & UCA0TXIFG)); // Wait for TX buffer to be ready for new data
        
        UCA0TXBUF = *TxArray; //Write the character at the location specified py the pointer
        TxArray++; //Increment the TxString pointer to point to the next character
    }
}

void init_timer() 
{
	TA1CTL |= TACLR;		// reset timer
	TA1CTL  = TASSEL_2		// SMCLK
			+ ID_0			// input divider = 1
			+ MC_2;			// continuous mode, interrupt disabled
	TA1CCTL0 = OUTMOD_2		// compare mode
			+ CCIE			// interrupt enabled
			+ CCIFG;
			
	/*** Timer0_A Set-Up ***/
    DCOCTL = 0;							// Select lowest DCO settings
	BCSCTL1 = CALBC1_1MHZ;				// Set DCO to 1 MHz
	DCOCTL = CALDCO_1MHZ;
    TA0CTL |= TASSEL_2 | MC_1 | ID_3;
    TA0CCR0 |= 800;
    TA0CCTL1 |= OUTMOD_7;
    TA0CCR1 |= 0;
}

void start_conversion() 
{
	if ((ADC10CTL1 & ADC10BUSY) == 0) {	// if not already converting
		//P1OUT ^= 0x40; //green led
		ADC10CTL0 |= ADC10SC;
		ADC10SA = (unsigned) &current; // store latest ADC value into address
	}
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    init_uart();
    init_timer();

    /*** GPIO Set-Up ***/
    P1DIR |= BIT6; // PWM output
    P1SEL |= BIT6;
    P2DIR |= BIT0 + BIT1 + BIT3 + BIT4;

    /*** ADC10 Set-Up ***/
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE + REFON + ENC; // ADC10ON, interrupt enabled. enable (but not yet start) conversions
    ADC10DTC1 = 1;			// one block per transfer
    ADC10CTL1 = INCH_3 + SHS_0 + ADC10DIV_3 + ADC10SSEL_3 + CONSEQ_0;	// clock source = SMCLK
    //ADC10AE0 |= BIT0;                         // PA.1 ADC option select
    
    // register ISR called when data was received
    uart_set_rx_isr_ptr(uart_rx_isr);

    // enable interrupts and put the CPU to sleep
	_bis_SR_register(GIE+LPM0_bits);

	unsigned char c = UCA0RXBUF;
	uart_putc(c);       // initialize retrieving of characters, initialize printing of characters.
	
	uart_puts((char *)"\n\rpH Pump Controller: ON and READY\n\r");  // signal that the main block has been initialized

    unsigned int a = 0, delay = 50000, sec = 0;
    for (;;)
    {
        current = ADC10MEM;
        error = current - SETtemperror;   // Calculate error difference from preset temperature. abs() removed.
        
        //a = 0; sec = 0;                         // Delays to remove LED flickering
        //while (a < delay) { a++; }
        //while ((a >= delay) && (sec < 10)) { a = 0; sec++; }
        
        if (error < 10) {
            P2OUT &= ~BIT4; P2OUT &= ~BIT3; P2OUT &= ~BIT1; P2OUT &= ~BIT0; // 0000
            TA0CCR1 = 0; // TA0CCR1 = ADC10MEM
        }
        if ((error >= 10) && (error < 15)) {
            P2OUT &= ~BIT4; P2OUT &= ~BIT3; P2OUT &= ~BIT1; P2OUT |= BIT0; // 0001
            TA0CCR1 = 100; // TA0CCR1 = ADC10MEM
        }
        if((error >= 15) && (error < 20)) {
            P2OUT &= ~BIT4; P2OUT &= ~BIT3; P2OUT |= BIT1; P2OUT &= ~BIT0; // 0010
            TA0CCR1 = 150; // TA0CCR1 = ADC10MEM
        }
        if((error >= 20) && (error < 25)) {
            P2OUT &= ~BIT4; P2OUT &= ~BIT3; P2OUT |= BIT1; P2OUT |= BIT0; // 0011
            TA0CCR1 = 200; // TA0CCR1 = ADC10MEM
        }
        if((error >= 25) && (error < 30)) {
            P2OUT &= ~BIT4; P2OUT |= BIT3; P2OUT &= ~BIT1; P2OUT &= ~BIT0; // 0100
            TA0CCR1 = 250; // TA0CCR1 = ADC10MEM
        }
        if((error >= 30) && (error < 35)) {
            P2OUT &= ~BIT4; P2OUT |= BIT3; P2OUT &= ~BIT1; P2OUT |= BIT0; // 0101
            TA0CCR1 = 300; // TA0CCR1 = ADC10MEM
        }
        if((error >= 35) && (error < 40)) {
            P2OUT &= ~BIT4; P2OUT |= BIT3; P2OUT |= BIT1; P2OUT &= ~BIT0; // 0110
            TA0CCR1 = 350; // TA0CCR1 = ADC10MEM
        }
        if((error >= 40) && (error < 45)) {
            P2OUT &= ~BIT4; P2OUT |= BIT3; P2OUT |= BIT1; P2OUT |= BIT0; // 0111
            TA0CCR1 = 400; // TA0CCR1 = ADC10MEM
        }
        if((error >= 45) && (error < 50)) {
            P2OUT |= BIT4; P2OUT &= ~BIT3; P2OUT &= ~BIT1; P2OUT &= ~BIT0; // 1000
            TA0CCR1 = 450; // TA0CCR1 = ADC10MEM
        }
        if((error >= 50) && (error < 55)) {
            P2OUT |= BIT4; P2OUT &= ~BIT3; P2OUT &= ~BIT1; P2OUT |= BIT0; // 1001
            TA0CCR1 = 500; // TA0CCR1 = ADC10MEM
        }
        if((error >= 55) && (error < 60)) {
            P2OUT |= BIT4; P2OUT &= ~BIT3; P2OUT |= BIT1; P2OUT &= ~BIT0; // 1010
            TA0CCR1 = 550; // TA0CCR1 = ADC10MEM
        }
        if((error >= 60) && (error < 65)) {
            P2OUT |= BIT4; P2OUT &= ~BIT3; P2OUT |= BIT1; P2OUT |= BIT0; // 1011
            TA0CCR1 = 600; // TA0CCR1 = ADC10MEM
        }
        if((error >= 65) && (error < 70)) {
            P2OUT |= BIT4; P2OUT |= BIT3; P2OUT &= ~BIT1; P2OUT &= ~BIT0; // 1100
            TA0CCR1 = 650; // TA0CCR1 = ADC10MEM
        }
        if((error >= 70) && (error < 75)) {
            P2OUT |= BIT4; P2OUT |= BIT3; P2OUT &= ~BIT1; P2OUT |= BIT0; // 1101
            TA0CCR1 = 700; // TA0CCR1 = ADC10MEM
        }
        if((error >= 75) && (error < 80)) {
            P2OUT |= BIT4; P2OUT |= BIT3; P2OUT |= BIT1; P2OUT &= ~BIT0; // 1110
            TA0CCR1 = 750; // TA0CCR1 = ADC10MEM
        }
        if(error >= 80) {
            P2OUT |= BIT4; P2OUT |= BIT3; P2OUT |= BIT1; P2OUT |= BIT0; // 1111
            TA0CCR1 = 800; // TA0CCR1 = ADC10MEM
        }
    }
}

#pragma vector=ADC10_VECTOR // ADC10 interrupt service routine
__interrupt void ADC10_ISR(void)
{
    __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

// INTERRUPT HANDLERS
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0(void) 
{
	timer_count++;
	if (timer_count > 16) { //default is 16 for 1 second refresh rate. set to 16 for slower data transfer
		timer_count = 0;
		start_conversion();

		IE2 |= UCA0TXIE; // activate TX interrupt
        IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
	}
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) 
{
    //P1DIR = 0x01;
	//P1OUT ^= 0x01; //red led
	
	unsigned int i = 0;					// iterator pointers
	sprintf(buffer, "pH measured: %d \n\r", (int)(current)); //output text to PC
	//uart_puts((char *)"MSP430 harduart\n\r");
	while (buffer[i] != '\0') {
		while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		UCA0TXBUF = buffer[i++];
	}
	
	unsigned int j = 0;					// iterator pointers
	sprintf(buffer2, "Error diff: %d \n\r", (int)(error)); //output text to PC
	while (buffer2[j] != '\0') {
		while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		UCA0TXBUF = buffer2[j++];
	}
	IE2 &= ~UCA0TXIFG; // reset interrupt flag
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	if(uart_rx_isr_ptr != 0L) {         // if there are more characters to send to RX buffer, keep sending characters.
		(uart_rx_isr_ptr)(UCA0RXBUF);
	}
}