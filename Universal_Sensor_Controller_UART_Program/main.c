
#include "msp430g2553.h"
#include <stdio.h>

/*** Initialize UART-Related Code ***/
volatile unsigned int ph = 0; // store the currently read ADC10MEM at the beginning of the loop.
volatile unsigned int timer_count = 0;
static char data;

char buffer[32];
char buffera[32];
char bufferb[32];
char bufferron[32];
char bufferroff[32];
char buffercsec[32];
char buffercmin[32];
char bufferchour[32];
char bufferdsec[32];
char bufferdmin[32];
char bufferg[32];
char bufferh[32];

int printlowbound = 0;
int printhighbound = 0;
int printofftimerdisp = 0;
int printontimerdisp = 0;

int OFFtimersetmode = 0;
int ONtimersetmode = 0;
int LowBoundsetmode = 0;
int HighBoundsetmode = 0;

/***************************************************************/
/*** Initialize custom settings from UART commands ***/

int resetenable = 0;    //start the controller in OFF mode.
int phpause = 1;                                    /*** start the printing of ph values in PAUSE mode. Press p to unpause upon startup or reset. ***/
int printonce = 0;

int SETlowbound = 0;    //Low bound minimum begins at 0 pH, or 0V.
int SEThighbound = 1020; /*High bound maximum ends at 14 pH, or 3.0V.*/ /*** This puts the bounds at the maximum ends of the ADC range, forcing the user to adjust them before pumps can turn on.***/

int SETofftimer = 1;                                /*** Set the off-timer length in minutes only. Default is 1 minute; cannot go lower than 1 minute. ***/
int SETontimer = 1;                                 /*** Set the on-timer length in seconds only. Default is 1 second; cannot go lower than 1 second. ***/

int OFFcount = 0; //incrementer for OFFTimer
int ONcount = 0; //incrementer for ONTimer
int OFFsecond = 5500;
int ONsecond = 5500;
int OFFcountsec = 0;
int ONcountsec = 0;
int OFFsecondset = 10;
int OFFminuteset = 0;
int OFFhourset = 0;
int ONsecondset = 1;
int ONminuteset = 0;
int OFFbegin = 0, ONbegin = 0;
int OFFtimebegin = 0, ONtimebegin = 0;
int BOTHtimersset = 0, timersactive = 0;
int printdisplay = 0;
int OUTcount = 0;
int OUTcountsec = 0;

int Acidicsecond = 4000;
int Basicsecond = 4000;
int Acidiccount = 0;
int Basiccount = 0;
int Acidiccountsec = 0;
int Basiccountsec = 0;

int AcidicPumpOUT = 0;
int BasicPumpOUT = 0;
int LowbMUXOUT = 0;
int HighbMUXOUT = 0;
int ONtMUXOUT = 0;
int OFFtMUXOUT = 0;

int LowBound = 0;
int HighBound = 0;
int OFFTimer = 0;
int ONTimer = 0;
int Acidicincrement = 0;
int Basicincrement = 0;

/***************************************************************/

void init_gpio();
void init_uart();
void init_timer();
void init_adc();
void start_conversion();
void UARTSendArray(unsigned char *TxArray, unsigned char ArrayLength);

void Check_ALL_Outputs_LEDs();
void OFF_Timer_BeginCount();
void ON_Timer_BeginCount();

void OFF_Timer_BeginCount(void) {
    /***************************************************************/
    /*** set up the off-timer and its delays, define the time-length from a UART command, or go to manual-control mode with another UART command ***/
    
    
    if ((BOTHtimersset == 1) && (OFFbegin == 1)) { // Counts only once, then waits for final output to turn off OFFtMUXOUT again.
        while (OFFcount < OFFsecond) { //10 minute off-timer. DEFAULT: 3750 = 1 second.
            Acidiccount = 0; Acidiccountsec = 0; Basiccount = 0; Basiccountsec = 0; Acidicsecond = 4000; Basicsecond = 4000; 
            Basicincrement = 0; Acidicincrement = 0; OUTcount = 0; OUTcountsec = 0;
    	    OFFcount++;
            TA1CCR1 = 0; // Acidic Pump PWM
            TA1CCR2 = 0; // Basic Pump PWM
    	}
    	if (OFFcount == OFFsecond) { // this delay speed is the exact speed of the MSP430 LEDs (two counts per LED blink period).
    	    OFFcountsec++; OFFcount = 0;
    	    Check_ALL_Outputs_LEDs();
    	}
    	if (OFFcountsec >= SETofftimer) { //begin counting multiple seconds. 1 second has now passed
    	    OFFtMUXOUT = 1;
    	    Check_ALL_Outputs_LEDs();
        }
    }
    /***************************************************************/
}
void ON_Timer_BeginCount(void) {
    /***************************************************************/
    /*** set up the on-timer and its delays, define the time-length from a UART command, or go to manual-control mode with another UART command ***/
    
    
    if ((BOTHtimersset == 1) && (ONbegin == 1)) { // Counts endlessly as a square wave. One-half the period is defined by SETon	    while (ONcount < ONsecond) { //10 minute off-timer. DEFAULT: 3750 = 1 second.
        while (ONcount < ONsecond) {    
            ONtMUXOUT = 0; P2OUT &= ~BIT3; //keep MUX output off until off-time has passed.
            Acidiccount = 0; Acidiccountsec = 0; Basiccount = 0; Basiccountsec = 0; Acidicsecond = 4000; Basicsecond = 4000; 
            Basicincrement = 0; Acidicincrement = 0; OUTcount = 0; OUTcountsec = 0;
            ONcount++;
            TA1CCR1 = 0; // Acidic Pump PWM
            TA1CCR2 = 0; // Basic Pump PWM
	   	}
	   	if (ONcount == ONsecond) { // this delay speed is the exact speed of the MSP430 LEDs (two counts per LED blink period).
	   	    ONcountsec++; ONcount = 0;
	   	    Check_ALL_Outputs_LEDs();
	   	}
	   	if (ONcountsec >= SETontimer) { //begin counting multiple seconds. 1 second has now passed
	   	    ONtMUXOUT = 1;
	   	    Check_ALL_Outputs_LEDs();
	    }
    }
    /***************************************************************/
}
void Check_ALL_Outputs_LEDs(void) {
    /*** set up and define the lower and upper boundary ranges from UART after calibrating the pH probe by reading the voltage value and ADC value with a pH solution of a known pH value, and then calculating all other ADC values ***/
    if (ADC10MEM < SETlowbound) { LowbMUXOUT = 1; }
    if (ADC10MEM >= SETlowbound) { LowbMUXOUT = 0; }
    if (ADC10MEM < SEThighbound) { HighbMUXOUT = 0; }
    if (ADC10MEM >= SEThighbound) { HighbMUXOUT = 1; }
    /***************************************************************/
    
    //test the various mux outputs.
    if (LowbMUXOUT == 1) { LowBound = 1; P2OUT |= BIT0; }     // RED LED = LOW BOUND has now detected a pH value lower than its current setting.
    if (LowbMUXOUT == 0) { LowBound = 0; P2OUT &= ~BIT0; }    // RED LED = LOW BOUND has now detected a pH value lower than its current setting.
    if (HighbMUXOUT == 1) { HighBound = 1; P2OUT |= BIT1; }   // ORANGE LED = HIGH BOUND has now detected a pH value higher than its current setting.
    if (HighbMUXOUT == 0) { HighBound = 0; P2OUT &= ~BIT1; }  // ORANGE LED = HIGH BOUND has now detected a pH value higher than its current setting.
    
    if (AcidicPumpOUT == 1) {                                 // DARK PURPLE LED = Acidic Pump is now ON. These LEDs are set directly within the Pump output if-statement.
        P1OUT |= BIT4; timersactive = 0;
        if (Acidicincrement < 1) {
            Acidicincrement = 1;
        }
    }               
    if (AcidicPumpOUT == 0) {                                 // DARK PURPLE LED = Acidic Pump is now ON.
        P1OUT &= ~BIT4;
        if (Acidicincrement == 1) {
            timersactive = 1;
            Acidicincrement = 0;
        }
    }               
    if (BasicPumpOUT == 1) {                                  // LIGHT PURPLE LED = Basic Pump is now ON.
        P1OUT |= BIT5; timersactive = 0;
        if (Basicincrement < 1) {
            Basicincrement = 1;
        }
    }
        
    if (BasicPumpOUT == 0) {                                  // LIGHT PURPLE LED = Basic Pump is now ON.
        P1OUT &= ~BIT5; 
        if (Basicincrement == 1) {
            timersactive = 1;
            Basicincrement = 0;
        }
    }
    
    if (OFFtMUXOUT == 1) { 
        OFFTimer = 1; 
        P2OUT |= BIT5;                                        // GREEN LED = OFF-TIMER
    }     
    if (OFFtMUXOUT == 0) { OFFTimer = 0; P2OUT &= ~BIT5; }    // GREEN LED = OFF-TIMER
    if (ONtMUXOUT == 1) { 
        ONTimer = 1; 
        ONcount = 0; ONcountsec = 0;
        P2OUT |= BIT3;                                        // YELLOW LED = ON-TIMER 
    }       
    if (ONtMUXOUT == 0) { ONTimer = 0; P2OUT &= ~BIT3; }      // YELLOW LED = ON-TIMER
    if (timersactive == 1) { P1OUT |= BIT7; }                 // WHITE LED = Timers are now counting.
    if (timersactive == 0) { P1OUT &= ~BIT7; }                // WHITE LED = Timers are now counting.
}

void main(void)	
{
    WDTCTL 	 = WDTPW + WDTHOLD;	// stop the WDT
	BCSCTL1  = CALBC1_1MHZ;	 	// calibration for basic clock system
	DCOCTL 	 = CALDCO_1MHZ;	 	// calibration for digitally controlled oscillator

	// initiate peripherals
	init_gpio();
	init_uart();
	init_timer();
	init_adc();

	// enable interrupts and put the CPU to sleep
	_bis_SR_register(GIE+LPM0_bits);
	
	UARTSendArray("\n\r", 2); 
	UARTSendArray("\n\r", 2); 
	UARTSendArray("Controller is OFF: Press 'r' to initialize.", 43);
    UARTSendArray("\n\r", 2);
	
	for (;;)
	{
	    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
        __bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR will force exit
        //ph = ADC10MEM;
        
        /***************************************************************/
        /*** pH Pump Controller if-statement code is below ***/
       
        if (resetenable == 0)
        {
            LowbMUXOUT = 0; HighbMUXOUT = 0; OFFtMUXOUT = 0; ONtMUXOUT = 0;
            SETlowbound = 0; SEThighbound = 1020; SETofftimer = 1; SETontimer = 1;
            OFFsecondset = 10; OFFminuteset = 0; OFFhourset = 0; ONsecondset = 1; ONminuteset = 0;
            OFFcount = 0; ONcount = 0; OFFcountsec = 0; ONcountsec = 0; OFFsecond = 5500; ONsecond = 5500; OUTcount = 0; OUTcountsec = 0;
            phpause = 1; printdisplay = 0; OFFbegin = 0; ONbegin = 0; OFFtimebegin = 0; ONtimebegin = 0;
            BOTHtimersset = 0; timersactive = 0;
            Acidicsecond = 4000; Basicsecond = 4000; Acidiccount = 0; Basiccount = 0; Acidiccountsec = 0; Basiccountsec = 0;
            AcidicPumpOUT = 0; BasicPumpOUT = 0;
    
            Acidicincrement = 0; Basicincrement = 0;
            OFFtimersetmode = 0; ONtimersetmode = 0; LowBoundsetmode = 0; HighBoundsetmode = 0;
            LowBound = 0; HighBound = 0; OFFTimer = 0; ONTimer = 0;
            P1OUT &= ~(BIT0 + BIT4 + BIT5 + BIT6 + BIT7);
	        P2OUT &= ~(BIT0 + BIT1 + BIT3 + BIT5);
        }
        if (resetenable == 1)
        {
            Check_ALL_Outputs_LEDs();
            
            /*** Set up the logic of outputs for the pump controller. If selected bound reading inputs a 1, and when the selected off-time ***/
            /*** has passed and outputs a 1, output a 1 to one of two pumps once and only for the time-length of the selected on-time, until the ***/
            /*** next off-time cycle has passed and outputs a new 1 value. Low bound enables the acidic pump, high bound enables the basic pump outputs. ***/
            if ((LowBound == 1) && (OFFTimer == 1) && (ONTimer == 1)) { //once all conditions are met, acidic pump turns on.
                TA1CCR1 = 0; // Acidic Pump PWM
                TA1CCR2 = 0; // Basic Pump PWM
                
                while (Acidiccount < Acidicsecond) { //10 minute off-timer. DEFAULT: 3750 = 1 second.
                    timersactive = 0; Acidicincrement = 2; P1OUT &= ~BIT7;
		    	    TA1CCR1 = 90; // Acidic Pump PWM
                    TA1CCR2 = 0; // Basic Pump PWM
                    AcidicPumpOUT = 1; //P1OUT |= BIT4;//keep MUX output off until off-time has passed.
		    	    Acidiccount++;
		    	    Check_ALL_Outputs_LEDs();
		    	    
		    	    if (Acidiccount >= Acidicsecond) {
		    	        Acidiccountsec++; Acidiccount = 0;
		    	        TA1CCR1 = 90; // Acidic Pump PWM
                        TA1CCR2 = 0; // Basic Pump PWM
		    	    }
		    	    if (Acidiccountsec >= SETontimer) { //begin counting multiple seconds. 1 second has now passed
		    	        timersactive = 1; Acidicincrement = 1; P1OUT |= BIT7;
		    	        Acidiccount = Acidicsecond;
		                OFFcount = 0; ONcount = 0; OFFcountsec = 0; ONcountsec = 0;
		                OFFtMUXOUT = 0; ONtMUXOUT = 0; OFFTimer = 0; ONTimer = 0;
		                
		    	        TA1CCR1 = 0; // Acidic Pump PWM
                        TA1CCR2 = 0; // Basic Pump PWM
		    	        AcidicPumpOUT = 0; //P1OUT &= ~BIT4;
		    	        Check_ALL_Outputs_LEDs();
		            }
		    	}
            }
            if ((HighBound == 1) && (OFFTimer == 1) && (ONTimer == 1)) { //once all conditions are met, basic pump turns on.
                TA1CCR1 = 0; // Acidic Pump PWM
                TA1CCR2 = 0; // Basic Pump PWM
                
                while (Basiccount < Basicsecond) { //10 minute off-timer. DEFAULT: 3750 = 1 second.
                    timersactive = 0; Basicincrement = 2; P1OUT &= ~BIT7;
		    	    TA1CCR1 = 0; // Acidic Pump PWM
                    TA1CCR2 = 90; // Basic Pump PWM
                    BasicPumpOUT = 1; //P1OUT |= BIT5; //keep MUX output off until off-time has passed.
		    	    Basiccount++;
		    	    Check_ALL_Outputs_LEDs();
		    	    
		    	    if (Basiccount >= Basicsecond) { // this delay speed is the exact speed of the MSP430 LEDs (two counts per LED blink period).
		    	        Basiccountsec++; Basiccount = 0;
		    	        TA1CCR1 = 0; // Acidic Pump PWM
                        TA1CCR2 = 90; // Basic Pump PWM
		    	    }
		    	    if (Basiccountsec >= SETontimer) { //begin counting multiple seconds. 1 second has now passed
		                timersactive = 1; Basicincrement = 1; P1OUT |= BIT7;
		                Basiccount = Basicsecond;
		                OFFcount = 0; ONcount = 0; OFFcountsec = 0; ONcountsec = 0;
		                OFFtMUXOUT = 0; ONtMUXOUT = 0; OFFTimer = 0; ONTimer = 0;
		                
		    	        TA1CCR1 = 0; // Acidic Pump PWM
                        TA1CCR2 = 0; // Basic Pump PWM
		    	        BasicPumpOUT = 0; //P1OUT &= ~BIT5;
		    	        Check_ALL_Outputs_LEDs();
		            }
                }
            }
            
            ON_Timer_BeginCount();
            Check_ALL_Outputs_LEDs();
            
            OFF_Timer_BeginCount();
            Check_ALL_Outputs_LEDs();
        }
	}
}

// HELPER FUNCTIONS
void init_gpio() 
{
    // LEDs
	P1DIR = BIT0 + BIT4 + BIT5 + BIT6 + BIT7;
	P2DIR = BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5;
	P1OUT &= ~(BIT0 + BIT4 + BIT5 + BIT6 + BIT7);
	P2OUT &= ~(BIT0 + BIT1 + BIT3 + BIT5);
	
	// Dual PWM Outputs
    P2SEL = BIT2 + BIT4;
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
	TA0CTL |= TACLR;		// reset timer
	TA0CTL  = TASSEL_2		// SMCLK
			+ ID_0			// input divider = 1
			+ MC_2;			// continuous mode, interrupt disabled
	TA0CCTL0 = OUTMOD_2		// compare mode
			+ CCIE			// interrupt enabled
			+ CCIFG;
	
	/***************************************************************/
	/*** define the Timer peripherals for the dual PWM outputs ***/		
	TA1CTL |= TASSEL_2 | MC_1 | ID_3 | TACLR;
	TA1CCTL0 = OUTMOD_2 + CCIFG; //compare mode
    TA1CCR0 = 100;                      //current PWM is set to 100Hz with a 90% duty cycle when turned on, still needs to be calibrated with actual pumps.
    TA1CCTL1 |= OUTMOD_7; //P2.2        //PWM duty cycles begins with 0% duty cycle. Make sure that the pumps are not still on with 0% duty cycle!
    TA1CCTL2 |= OUTMOD_7; //P2.4
    TA1CCR1 = 0; // Acidic Pump PWM
    TA1CCR2 = 0; // Basic Pump PWM
    /***************************************************************/
}

void init_adc()
{
	ADC10CTL1 = INCH_3		// photoresistor input to ADC
			+ SHS_0			// use ADC10SC bit to trigger sampling
			+ ADC10DIV_3	// clock divider = 4
			+ ADC10SSEL_3	// clock source = SMCLK
			+ CONSEQ_0;		// single channel, single conversion
	ADC10DTC1 = 1;			// one block per transfer
    //ADC10AE0 = 0x08;      // enable A4 analog input
	ADC10CTL0 = SREF_0		// reference voltages are Vss and Vcc. 0 allows code to work.
			+ ADC10SHT_3	// 64 ADC10 clocks for sample and hold time (slowest)
			+ REFON			// reference generator on
			+ ADC10ON		// turn on ADC10
			+ ADC10IE
			+ ENC;			// enable (but not yet start) conversions
}

void start_conversion() 
{
	if ((ADC10CTL1 & ADC10BUSY) == 0) {	// if not already converting
		//P1OUT ^= BIT0; //red led
		ADC10CTL0 |= ADC10SC;
		ADC10SA = (unsigned) &ph; // store latest ADC value into address
	}
}

#pragma vector=ADC10_VECTOR // ADC10 interrupt service routine
__interrupt void ADC10_ISR(void)
{
    __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

// INTERRUPT HANDLERS
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void) 
{
	timer_count++;
	if (timer_count > 13) { //default is 13 for 1 second refresh rate. set to 16 for slower data transfer
		timer_count = 0; 
		start_conversion();
		IE2 |= UCA0TXIE; // activate TX interrupt
		IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
	}
}

#pragma vector = USCIAB0TX_VECTOR // Note: too much code in here will cause microprocessor to run out of memory.
__interrupt void USCI0TX_ISR(void) 
{
	P1OUT |= BIT6; //green led
	
	/*******************************************************************************/
    /*** Print the newly configured setting once, only when a specific character is entered. ***/
	if (printlowbound == 1) {
	    unsigned int a = 0;					// iterator pointers
	    sprintf(buffera, "Low pH Bound is now:   %d \n\r", (int)(SETlowbound)); //output text to PC

	    while (buffera[a] != '\0') {
		    while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		    UCA0TXBUF = buffera[a++];
	    }
	    printlowbound = 0;
	}
	if (printhighbound == 1) {
	    unsigned int b = 0;					// iterator pointers
	    sprintf(bufferb, "High pH Bound is now:   %d \n\r", (int)(SEThighbound)); //output text to PC

	    while (bufferb[b] != '\0') {
		    while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		    UCA0TXBUF = bufferb[b++];
	    }
	    printhighbound = 0;
	}
	
	if (printofftimerdisp == 1) {
	    SETofftimer = (OFFsecondset + (OFFminuteset * 60) + (OFFhourset * 3600));
	    
	    UARTSendArray("Current OFF-Timer duration is:   ", 30);
	    unsigned int csec = 0;					// iterator pointers
	    sprintf(buffercsec, "   Seconds : %d   ", (int)(OFFsecondset)); //output text to PC

	    while (buffercsec[csec] != '\0') {
		    while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		    UCA0TXBUF = buffercsec[csec++];
	    }
	    unsigned int cmin = 0;					// iterator pointers
	    sprintf(buffercmin, "Minutes : %d   ", (int)(OFFminuteset)); //output text to PC

	    while (buffercmin[cmin] != '\0') {
		    while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		    UCA0TXBUF = buffercmin[cmin++];
	    }
	    unsigned int chour = 0;					// iterator pointers
	    sprintf(bufferchour, "Hours : %d  \n\r", (int)(OFFhourset)); //output text to PC

	    while (bufferchour[chour] != '\0') {
		    while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		    UCA0TXBUF = bufferchour[chour++];
	    }
	    printofftimerdisp = 0; 
	}
	if (printontimerdisp == 1) {
	    SETontimer = (ONsecondset + (ONminuteset * 60));
	    
	    UARTSendArray("Current ON-Timer duration is:   ", 29);
	    unsigned int dsec = 0;					// iterator pointers
	    sprintf(bufferdsec, "   Seconds : %d   ", (int)(ONsecondset)); //output text to PC

	    while (bufferdsec[dsec] != '\0') {
		    while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		    UCA0TXBUF = bufferdsec[dsec++];
	    }
	    unsigned int dmin = 0;					// iterator pointers
	    sprintf(bufferdmin, "Minutes : %d  \n\r", (int)(ONminuteset)); //output text to PC

	    while (bufferdmin[dmin] != '\0') {
		    while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		    UCA0TXBUF = bufferdmin[dmin++];
	    }
	    printontimerdisp = 0; 
	}
	
    /*******************************************************************************/
    /*** Continuously print the current pH reading to PC only when controller is ON and when unpaused by UART command ***/
    /*** If a UART command is pressed, change the variable(s) that are being printed. ***/
    if ((resetenable == 1) && (phpause == 0)) {
        
        if (printdisplay == 0) {
	        unsigned int i = 0;					// iterator pointers
	        sprintf(buffer, "Current ADC: %d \n\r", (int)(ph)); //output text to PC

	        while (buffer[i] != '\0') {
		        while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		        UCA0TXBUF = buffer[i++];
	        }
	        printdisplay = 0;
        }
        if (printdisplay == 1) {
	        unsigned int g = 0;					// iterator pointers
	        sprintf(bufferg, "OFF-Timer count: %d     ", (int)(OFFcountsec)); //output text to PC

	        while (bufferg[g] != '\0') {
		        while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		        UCA0TXBUF = bufferg[g++];
	        }
	        unsigned int h = 0;					// iterator pointers
	        sprintf(bufferh, "ON-Timer count: %d \n\r", (int)(ONcountsec)); //output text to PC

	        while (bufferh[h] != '\0') {
		        while (!(IFG2 & UCA0TXIFG));	// USCI_A0 TX buffer ready?
		        UCA0TXBUF = bufferh[h++];
	        }
	        printdisplay = 1;
        }
        if (printonce == 0) {
            phpause = 1;
        }
    }
	
	/*******************************************************************************/
	// Also found in case statement UART commands. This is placed here to reconfirm the setting for BOTHtimersset.
	if ((OFFbegin == 1) && (ONbegin == 1)) {               /*** Once both timers' settings have been set, then turn on both timers and begin counting. ***/
	    BOTHtimersset = 1;
	    timersactive = 1;
	    AcidicPumpOUT = 0; BasicPumpOUT = 0;
	}
	if ((OFFbegin == 0) || (ONbegin == 0)) {
	    BOTHtimersset = 0;
	    timersactive = 0;
	    TA1CCR1 = 0; // Acidic Pump PWM
        TA1CCR2 = 0; // Basic Pump PWM
	}
    
	P1OUT &= ~BIT6; // green led
	IE2 &= ~UCA0TXIFG; // reset interrupt flag
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    data = UCA0RXBUF;
    UARTSendArray("Received command: ", 18); //printing to PC
    UARTSendArray(&data, 1);  //one character input
    UARTSendArray("\n\r", 2); //essentially an enter key
    UARTSendArray("\n\r", 2); //essentially an enter key
    P1OUT |= BIT0; //red led

    if (resetenable == 1) {
    switch(data){
        case 'Set pH = 475': { //does not work. will make this possible in the future, the number inputted will be converted into an integer for the variable below.
            //SETpHerror = 475;
            UARTSendArray("Desired pH is now: 350", 27);
            UARTSendArray("\n\r", 2);
        }
        break;
        
        /*******************************************************************************/
        /*** Commands can be customized here. Single-character commands only, for now. ***/
        
        /*** Reset-Enable for resetting entire controller. Pause for pausing the printing of pH values. ***/
        case 'r': {
            UARTSendArray("Controller is already ON. Press 'R' to hard-reset the controller.", 65);
            UARTSendArray("\n\r", 2);
        }
        break;
        case 'R': {
            UARTSendArray("\n\r", 2);
            UARTSendArray("\n\r", 2);
            UARTSendArray("Controller is OFF: Press 'r' to initialize.", 43);
            UARTSendArray("\n\r", 2);
            UARTSendArray("All timers have reset. pH bounds and time-length settings have reset.", 69);
            UARTSendArray("\n\r", 2);
            resetenable = 0; 
        }
        break;
        case 'e': {
            UARTSendArray("Printing once the current pH value.", 35);
            UARTSendArray("\n\r", 2);
            phpause = 0; 
            printonce = 0;
        }
        break;
        case 'E': {
            UARTSendArray("Continuously printing the current pH value. Press 'e' to stop printing. \n\r", 73);
            UARTSendArray("WARNING: Outputs will not work while continuously printing!", 59);
            UARTSendArray("\n\r", 2);
            phpause = 0;
            printonce = 1;
        }
        break;
        case 'w': {
            UARTSendArray("Changing printed value to:   pH.", 32);
            UARTSendArray("\n\r", 2);
            printdisplay = 0; 
        }
        break;
        case 'W': {
            UARTSendArray("Changing printed values to:   OFFTimer count    ONTimer count.", 62);
            UARTSendArray("\n\r", 2);
            printdisplay = 1;
        }
        break;
        case '/': {
            UARTSendArray("Provided below is a few common pH values, converted from the ADC values printed after 'e' is pressed.", 101);
            UARTSendArray("\n\r", 2);
            UARTSendArray("To convert a pH value, look at the multipliers below.", 53);
            UARTSendArray("\n\r", 2);
            UARTSendArray("'pH value' * 73.1428571429 = 'ADC value'.   'ADC value' * 0.013671875 = 'pH value'.", 83);
            UARTSendArray("\n\r", 2);
            printdisplay = 1;
        }
        break;
        /*******************************************************************************/
        
        /*** Low pH Boundary adjust mode, and three different adjust-incrementing commands. ***/
        case 'L': {
            if (LowBoundsetmode == 1) {
                UARTSendArray("Cannot use this command yet. Press 'l' to exit Low pH Bound Adjust Mode first.", 78);
                UARTSendArray("\n\r", 2);
            }
            if ((LowBoundsetmode == 0) && (HighBoundsetmode == 0) && (OFFtimersetmode == 0) && (ONtimersetmode == 0)) {
                UARTSendArray("Low pH Bound Adjust Mode is ON: Adjust commands are now enabled. See new commands below.", 88);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("Low pH Bound Adjust commands:   'i' & 'I' = adjust by 1.   'o' & 'O' = adjust by 10.   'p' & 'P' = adjust by 50.", 112);
                UARTSendArray("\n\r", 2);
                UARTSendArray("Other commands:   'l' = Turn off Low pH Bound Adjust Mode, and disable adjust commands.", 87);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                LowBoundsetmode = 1; HighBoundsetmode = 0;
            }
        }
        break;
        case 'l': {
            if (LowBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'L' to enter Low pH Bound Adjust Mode first.", 79);
                UARTSendArray("\n\r", 2);
            }
            if (LowBoundsetmode == 1) {
                UARTSendArray("Low pH Bound Adjust Mode is OFF: Adjust commands are now disabled.", 66);
                UARTSendArray("\n\r", 2);
                UARTSendArray("Press 'L' to adjust Low pH Bound value again.", 45);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("The new Low pH Bound setting is below: ", 38);
                UARTSendArray("\n\r", 2);
                printlowbound = 1;
                LowBoundsetmode = 0; HighBoundsetmode = 0;
            }
        }
        break;
        case 'i': {
            if (LowBoundsetmode == 1) {
                UARTSendArray("Decreasing by 1 Low pH Bound value.", 35);
                UARTSendArray("\n\r", 2);
                printlowbound = 1;
                SETlowbound = SETlowbound - 1; // input this character to decrement the low boundary ADC value by 1 (represents pH from 0 to 7)
                if (SETlowbound < 0) {
                    UARTSendArray("Low pH Bound cannot be lower than 0, or 0 pH.", 45);
                    UARTSendArray("\n\r", 2);
                    SETlowbound = 0;
                }
            }
            if (LowBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'L' to enter Low Bound Adjust Mode first.", 76);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'I': {
            if (LowBoundsetmode == 1) {
                UARTSendArray("Increasing by 1 Low pH Bound value.", 35);
                UARTSendArray("\n\r", 2);
                printlowbound = 1;
                SETlowbound = SETlowbound + 1; // input this character to increment the low boundary ADC value by 1 (represents pH from 0 to 7)
                if (SETlowbound > 512) {
                    UARTSendArray("Low pH Bound cannot be higher than 512, or 7 pH.", 48);
                    UARTSendArray("\n\r", 2);
                    SETlowbound = 512;
                }
            }
            if (LowBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'L' to enter Low Bound Adjust Mode first.", 76);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'o': {
            if (LowBoundsetmode == 1) {
                UARTSendArray("Decreasing by 10 Low pH Bound values.", 37);
                UARTSendArray("\n\r", 2);
                printlowbound = 1;
                SETlowbound = SETlowbound - 10; // input this character to decrement the low boundary ADC value by 10 (represents pH from 0 to 7)
                if (SETlowbound < 0) {
                    UARTSendArray("Low pH Bound cannot be lower than 0, or 0 pH.", 45);
                    UARTSendArray("\n\r", 2);
                    SETlowbound = 0;
                }
            }
            if (LowBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'L' to enter Low Bound Adjust Mode first.", 76);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'O': {
            if (LowBoundsetmode == 1) {
                UARTSendArray("Increasing by 10 Low pH Bound values.", 37);
                UARTSendArray("\n\r", 2);
                printlowbound = 1;
                SETlowbound = SETlowbound + 10; // input this character to increment the low boundary ADC value by 10 (represents pH from 0 to 7)
                if (SETlowbound > 512) {
                    UARTSendArray("Low pH Bound cannot be higher than 512, or 7 pH.", 48);
                    UARTSendArray("\n\r", 2);
                    SETlowbound = 512;
                }
            }
            if (LowBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'L' to enter Low Bound Adjust Mode first.", 76);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'p': {
            if (LowBoundsetmode == 1) {
                UARTSendArray("Decreasing by 50 Low pH Bound values.", 37);
                UARTSendArray("\n\r", 2);
                printlowbound = 1;
                SETlowbound = SETlowbound - 50; // input this character to decrement the low boundary ADC value by 50 (represents pH from 0 to 7)
                if (SETlowbound < 0) {
                    UARTSendArray("Low pH Bound cannot be lower than 0, or 0 pH.", 45);
                    UARTSendArray("\n\r", 2);
                    SETlowbound = 0;
                }
            }
            if (LowBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'L' to enter Low Bound Adjust Mode first.", 76);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'P': {
            if (LowBoundsetmode == 1) {
                UARTSendArray("Increasing by 50 Low pH Bound values.", 37);
                UARTSendArray("\n\r", 2);
                printlowbound = 1;
                SETlowbound = SETlowbound + 50; // input this character to increment the low boundary ADC value by 50 (represents pH from 0 to 7)
                if (SETlowbound > 512) {
                    UARTSendArray("Low pH Bound cannot be higher than 512, or 7 pH.", 48);
                    UARTSendArray("\n\r", 2);
                    SETlowbound = 512;
                }
            }
            if (LowBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'L' to enter Low Bound Adjust Mode first.", 76);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        /*******************************************************************************/
        
        /*** High pH Boundary adjust mode, and three different adjust-incrementing commands. ***/
        case 'H': {
            if (HighBoundsetmode == 1) {
                UARTSendArray("Cannot use this command yet. Press 'h' to exit High pH Bound Adjust Mode first.", 79);
                UARTSendArray("\n\r", 2);
            }
            if ((LowBoundsetmode == 0) && (HighBoundsetmode == 0) && (OFFtimersetmode == 0) && (ONtimersetmode == 0)) {
                UARTSendArray("High pH Bound Adjust Mode is ON: Adjust commands are now enabled. See new commands below.", 89);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("High pH Bound Adjust commands:   't' & 'T' = adjust by 1.   'y' & 'Y' = adjust by 10.   'u' & 'U' = adjust by 50.", 113);
                UARTSendArray("\n\r", 2);
                UARTSendArray("Other commands:   'h' = Turn off High pH Bound Adjust Mode, and disable adjust commands.", 88);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                HighBoundsetmode = 1; LowBoundsetmode = 0;
            }
        }
        break;
        case 'h': {
            if (HighBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'H' to enter High pH Bound Adjust Mode first.", 80);
                UARTSendArray("\n\r", 2);
            }
            if (HighBoundsetmode == 1) {
                UARTSendArray("High pH Bound Adjust Mode is OFF: Adjust commands are now disabled.", 67);
                UARTSendArray("\n\r", 2);
                UARTSendArray("Press 'H' to adjust High pH Bound value again.", 46);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("The new High pH Bound setting is below: ", 39);
                UARTSendArray("\n\r", 2);
                printhighbound = 1;
                HighBoundsetmode = 0; LowBoundsetmode = 0;
            }
        }
        break;
        case 't': {
            if (HighBoundsetmode == 1) {
                UARTSendArray("Decreasing by 1 High pH Bound value.", 36);
                UARTSendArray("\n\r", 2);
                printhighbound = 1;
                SEThighbound = SEThighbound - 1; // input this character to decrement the high boundary ADC value by 1 (represents pH from 7 to 14)
                if (SEThighbound < 512) {
                    UARTSendArray("High pH Bound cannot be lower than 512, or 7 pH.", 48);
                    UARTSendArray("\n\r", 2);
                    SEThighbound = 512;
                }
            }
            if (HighBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'H' to enter High Bound Adjust Mode first.", 77);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'T': {
            if (HighBoundsetmode == 1) {
                UARTSendArray("Increasing by 1 High pH Bound value.", 36);
                UARTSendArray("\n\r", 2);
                printhighbound = 1;
                SEThighbound = SEThighbound + 1; // input this character to increment the high boundary ADC value by 1 (represents pH from 7 to 14)
                if (SEThighbound > 1020) {
                    UARTSendArray("High pH Bound cannot be higher than 1020, or 14 pH.", 51);
                    UARTSendArray("\n\r", 2);
                    SEThighbound = 1020;
                }
            }
            if (HighBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'H' to enter High Bound Adjust Mode first.", 77);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'y': {
            if (HighBoundsetmode == 1) {
                UARTSendArray("Decreasing by 10 High pH Bound values.", 37);
                UARTSendArray("\n\r", 2);
                printhighbound = 1;
                SEThighbound = SEThighbound - 10; // input this character to decrement the high boundary ADC value by 10 (represents pH from 7 to 14)
                if (SEThighbound < 512) {
                    UARTSendArray("High pH Bound cannot be lower than 512, or 7 pH.", 48);
                    UARTSendArray("\n\r", 2);
                    SEThighbound = 512;
                }
            }
            if (HighBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'H' to enter High Bound Adjust Mode first.", 77);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'Y': {
            if (HighBoundsetmode == 1) {
                UARTSendArray("Increasing by 10 High pH Bound values.", 37);
                UARTSendArray("\n\r", 2);
                printhighbound = 1;
                SEThighbound = SEThighbound + 10; // input this character to increment the high boundary ADC value by 10 (represents pH from 7 to 14)
                if (SEThighbound > 1020) {
                    UARTSendArray("High pH Bound cannot be higher than 1020, or 14 pH.", 51);
                    UARTSendArray("\n\r", 2);
                    SEThighbound = 1020;
                }
            }
            if (HighBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'H' to enter High Bound Adjust Mode first.", 77);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'u': {
            if (HighBoundsetmode == 1) {
                UARTSendArray("Decreasing by 50 High pH Bound values.", 37);
                UARTSendArray("\n\r", 2);
                printhighbound = 1;
                SEThighbound = SEThighbound - 50; // input this character to decrement the high boundary ADC value by 50 (represents pH from 7 to 14)
                if (SEThighbound < 512) {
                    UARTSendArray("High pH Bound cannot be lower than 512, or 7 pH.", 48);
                    UARTSendArray("\n\r", 2);
                    SEThighbound = 512;
                }
            }
            if (HighBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'H' to enter High Bound Adjust Mode first.", 77);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'U': {
            if (HighBoundsetmode == 1) {
                UARTSendArray("Increasing by 50 High pH Bound values.", 37);
                UARTSendArray("\n\r", 2);
                printhighbound = 1;
                SEThighbound = SEThighbound + 50; // input this character to increment the high boundary ADC value by 50 (represents pH from 7 to 14)
                if (SEThighbound > 1020) {
                    UARTSendArray("High pH Bound cannot be higher than 1020, or 14 pH.", 51);
                    UARTSendArray("\n\r", 2);
                    SEThighbound = 1020;
                }
            }
            if (HighBoundsetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'H' to enter High Bound Adjust Mode first.", 77);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        /*******************************************************************************/
        
        /*** OFF-Timer Set Mode and off-time adjusts. ***/
        case 'z': {
            if (OFFtimersetmode == 1) {
                UARTSendArray("Cannot use this command yet. Press 'Z' to exit OFF-Timer Set Mode first.", 72);
                UARTSendArray("\n\r", 2);
            }
            if ((LowBoundsetmode == 0) && (HighBoundsetmode == 0) && (OFFtimersetmode == 0) && (ONtimersetmode == 0)) {
                UARTSendArray("OFF-Timer Set Mode is ON: See new commands below.", 49);
                UARTSendArray("\n\r", 2);
                UARTSendArray("OFF-Timer has stopped counting. Both timers will restart when OFF-Timer Set Mode is OFF,", 88);
                UARTSendArray("\n\r", 2);
                UARTSendArray("only after both OFF-Timer and ON-Timer duration settings have been set.", 71);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("OFF-Timer Set commands:   'x' & 'X' = OFF-seconds adjust.   'c' & 'C' = OFF-minutes adjust.   'v' & 'V' = OFF-hours adjust.", 123);
                UARTSendArray("\n\r", 2);
                UARTSendArray("Other commands: 'Z' = Turn off OFF-Timer Set Mode, and display current OFF-Timer settings.", 90);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                OFFtimersetmode = 1; ONtimersetmode = 0;
                OFFbegin = 0; // stop and restart both timers.
                BOTHtimersset = 0; timersactive = 0;
            }
        }
        break;
        case 'Z': {
            if (OFFtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'z' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
            if (OFFtimersetmode == 1) {
                UARTSendArray("OFF-Timer Set Mode is OFF. Both timers will start counting only after,", 70);
                UARTSendArray("\n\r", 2);
                UARTSendArray("both OFF-Timer and ON-Timer duration settings have been set.", 60);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                
                OFFbegin = 1;
                if ((OFFbegin == 1) && (ONbegin == 1)) { BOTHtimersset = 1;}
                if ((OFFbegin == 0) || (ONbegin == 0)) { BOTHtimersset = 0; }
                if (BOTHtimersset == 0) {
                    UARTSendArray("Are the duration settings for both timers set yet?     Answer: No.", 66);
                    UARTSendArray("\n\r", 2);
                    UARTSendArray("Please set the ON-Timer duration by pressing 'a' before both timers can begin counting.", 87);
                    UARTSendArray("\n\r", 2);
                }
                if (BOTHtimersset == 1) {
                    UARTSendArray("Are the duration settings for both timers set yet?     Answer: Yes.", 67);
                    UARTSendArray("\n\r", 2);
                    UARTSendArray("The pH Pump Controller is ready to go! Both timers will now BEGIN counting.", 75);
                    UARTSendArray("\n\r", 2);
                }
                
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                printofftimerdisp = 1;
                OFFtimersetmode = 0; ONtimersetmode = 0;
            }
        }
        break;
        case 'x': { // maximum time is 18 hours, limited by unsigned int going only up to 65535.
            if (OFFtimersetmode == 1) {
                UARTSendArray("Decreasing by 1 second length of OFF-timer.", 43);
                UARTSendArray("\n\r", 2);
                printofftimerdisp = 1;
                OFFsecondset = OFFsecondset - 1;
                if (OFFsecondset < 0) {
                    UARTSendArray("OFF-Timer second set cannot be lower than 0 seconds.", 52);
                    UARTSendArray("\n\r", 2);
                    OFFsecondset = 0;
                }
                if ((OFFsecondset < 10) && (OFFminuteset <= 0 ) && (OFFhourset <= 0)) {
                    UARTSendArray("OFF-Timer duration cannot be shorter than 10 seconds.", 53);
                    UARTSendArray("\n\r", 2);
                    OFFsecondset = 10;
                }
            }
            if (OFFtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'z' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'X': { // maximum time is 18 hours, limited by unsigned int going only up to 65535.
            if (OFFtimersetmode == 1) {
                UARTSendArray("Increasing by 1 second length of OFF-timer.", 43);
                UARTSendArray("\n\r", 2);
                printofftimerdisp = 1;
                OFFsecondset = OFFsecondset + 1;
                if (OFFsecondset > 60) {
                    UARTSendArray("OFF-Timer second set cannot be higher than 60 seconds.", 54);
                    UARTSendArray("\n\r", 2);
                    OFFsecondset = 60;
                }
                if ((OFFhourset >= 8) && ((OFFminuteset > 0) || (OFFsecondset > 0))) {
                    UARTSendArray("OFF-Timer hour set cannot be higher than 8 hours. Limited to 16-bit integers.", 77);
                    UARTSendArray("\n\r", 2);
                    OFFhourset = 8; OFFminuteset = 0; OFFsecondset = 0;
                }
            }
            if (OFFtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'z' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'c': {
            if (OFFtimersetmode == 1) {
                UARTSendArray("Decreasing by 1 minute length of OFF-timer.", 43);
                UARTSendArray("\n\r", 2);
                printofftimerdisp = 1;
                OFFminuteset = OFFminuteset - 1;
                if (OFFminuteset < 0) {
                    UARTSendArray("OFF-Timer minute set cannot be lower than 0 minutes.", 52);
                    UARTSendArray("\n\r", 2);
                    OFFminuteset = 0;
                }
                if ((OFFsecondset < 10) && (OFFminuteset <= 0 ) && (OFFhourset <= 0)) {
                    UARTSendArray("OFF-Timer duration cannot be shorter than 10 seconds.", 53);
                    UARTSendArray("\n\r", 2);
                    OFFsecondset = 10;
                }
            }
            if (OFFtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'z' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'C': {
            if (OFFtimersetmode == 1) {
                UARTSendArray("Increasing by 1 minute length of OFF-timer.", 43);
                UARTSendArray("\n\r", 2);
                printofftimerdisp = 1;
                OFFminuteset = OFFminuteset + 1;
                if (OFFminuteset > 60) {
                    UARTSendArray("OFF-Timer minute set cannot be higher than 60 minutes.", 54);
                    UARTSendArray("\n\r", 2);
                    OFFminuteset = 60;
                }
                if ((OFFhourset >= 8) && ((OFFminuteset > 0) || (OFFsecondset > 0))) {
                    UARTSendArray("OFF-Timer hour set cannot be higher than 8 hours. Limited to 16-bit integers.", 77);
                    UARTSendArray("\n\r", 2);
                    OFFhourset = 8; OFFminuteset = 0; OFFsecondset = 0;
                }
            }
            if (OFFtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'z' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'v': {
            if (OFFtimersetmode == 1) {
                UARTSendArray("Decreasing by 1 hour length of OFF-timer.", 41);
                UARTSendArray("\n\r", 2);
                printofftimerdisp = 1;
                OFFhourset = OFFhourset - 1;
                if (OFFhourset < 0) {
                    UARTSendArray("OFF-Timer hour set cannot be lower than 0 hours.", 48);
                    UARTSendArray("\n\r", 2);
                    OFFhourset = 0;
                }
                if ((OFFsecondset < 10) && (OFFminuteset <= 0 ) && (OFFhourset <= 0)) {
                    UARTSendArray("OFF-Timer duration cannot be shorter than 10 seconds.", 53);
                    UARTSendArray("\n\r", 2);
                    OFFsecondset = 10;
                }
            }
            if (OFFtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'z' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'V': {
            if (OFFtimersetmode == 1) {
                UARTSendArray("Increasing by 1 hour length of OFF-timer.", 41);
                UARTSendArray("\n\r", 2);
                printofftimerdisp = 1;
                OFFhourset = OFFhourset + 1;
                if (OFFhourset > 8) {
                    UARTSendArray("OFF-Timer hour set cannot be higher than 8 hours. Limited to 16-bit integers.", 77);
                    UARTSendArray("\n\r", 2);
                    OFFhourset = 8;
                }
                if ((OFFhourset >= 8) && ((OFFminuteset > 0) || (OFFsecondset > 0))) {
                    UARTSendArray("OFF-Timer hour set cannot be higher than 8 hours. Limited to 16-bit integers.", 77);
                    UARTSendArray("\n\r", 2);
                    OFFhourset = 8; OFFminuteset = 0; OFFsecondset = 0;
                }
            }
            if (OFFtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'z' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        /*******************************************************************************/
        
        /*** ON-Timer Set Mode and on-time adjusts. ***/
        case 'a': {
            if (ONtimersetmode == 1) {
                UARTSendArray("Cannot use this command yet. Press 'A' to exit ON-Timer Set Mode first.", 72);
                UARTSendArray("\n\r", 2);
            }
            if ((LowBoundsetmode == 0) && (HighBoundsetmode == 0) && (OFFtimersetmode == 0) && (ONtimersetmode == 0)) {
                UARTSendArray("ON-Timer Set Mode is ON: See new commands below.", 48);
                UARTSendArray("\n\r", 2);
                UARTSendArray("ON-Timer has stopped counting. Both timers will restart when ON-Timer Set Mode is OFF,", 86);
                UARTSendArray("\n\r", 2);
                UARTSendArray("only after both ON-Timer and ON-Timer duration settings have been set.", 70);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("ON-Timer Set commands:   's' & 'S' = ON-seconds adjust.   'd' & 'D' = ON-minutes adjust.", 88);
                UARTSendArray("\n\r", 2);
                UARTSendArray("Other commands: 'A' = Turn off ON-Timer Set Mode, and display current ON-Timer settings.", 88);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                ONtimersetmode = 1; OFFtimersetmode = 0;
                ONbegin = 0;  // stop and restart both timers.
                BOTHtimersset = 0; timersactive = 0;
            }
        }
        break;
        case 'A': {
            if (ONtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'a' to enter ON-Timer Set Mode first.", 72);
                UARTSendArray("\n\r", 2);
            }
            if (ONtimersetmode == 1) {
                UARTSendArray("ON-Timer Set Mode is OFF. Both timers will start counting only after,", 69);
                UARTSendArray("\n\r", 2);
                UARTSendArray("both OFF-Timer and ON-Timer duration settings have been set.", 60);
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                UARTSendArray("\n\r", 2);
                
	            ONbegin = 1;
                if ((OFFbegin == 1) && (ONbegin == 1)) { BOTHtimersset = 1;}
                if ((OFFbegin == 0) || (ONbegin == 0)) { BOTHtimersset = 0; }
                if (BOTHtimersset == 0) {
                    UARTSendArray("Are the duration settings for both timers set yet?     Answer: No.", 66);
                    UARTSendArray("\n\r", 2);
                    UARTSendArray("Please set the OFF-Timer duration by pressing 'z' before both timers can begin counting.", 88);
                    UARTSendArray("\n\r", 2);
                }
                if (BOTHtimersset == 1) {
                    UARTSendArray("Are the duration settings for both timers set yet?     Answer: Yes.", 67);
                    UARTSendArray("\n\r", 2);
                    UARTSendArray("The pH Pump Controller is ready to go! Both timers will now BEGIN counting.", 75);
                    UARTSendArray("\n\r", 2);
                }
                
                UARTSendArray("\n\r", 2);
                UARTSendArray("****************************************************************************************************", 100);
                UARTSendArray("\n\r", 2);
                printontimerdisp = 1;
                ONtimersetmode = 0; OFFtimersetmode = 0;
            }
        }
        break;
        case 's': { // maximum time is 18 hours, limited by unsigned int going only up to 65535.
            if (ONtimersetmode == 1) {
                UARTSendArray("Decreasing by 1 second length of ON-timer.", 42);
                UARTSendArray("\n\r", 2);
                printontimerdisp = 1;
                ONsecondset = ONsecondset - 1;
                if (ONsecondset < 0) {
                    UARTSendArray("ON-Timer second set cannot be lower than 0 seconds.", 51);
                    UARTSendArray("\n\r", 2);
                    ONsecondset = 0;
                }
                if ((ONsecondset < 1) && (ONminuteset <= 0 )) {
                    UARTSendArray("ON-Timer duration cannot be shorter than 1 second.", 50);
                    UARTSendArray("\n\r", 2);
                    ONsecondset = 1;
                }
            }
            if (ONtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'a' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'S': { // maximum time is 18 hours, limited by unsigned int going only up to 65535.
            if (ONtimersetmode == 1) {
                UARTSendArray("Increasing by 1 second length of ON-timer.", 42);
                UARTSendArray("\n\r", 2);
                printontimerdisp = 1;
                ONsecondset = ONsecondset + 1;
                if (ONsecondset > 60) {
                    UARTSendArray("ON-Timer second set cannot be higher than 60 seconds.", 53);
                    UARTSendArray("\n\r", 2);
                    ONsecondset = 60;
                }
                if ((ONminuteset >= 5) && (ONsecondset > 0)) {
                    UARTSendArray("ON-Timer minute set cannot be higher than 5 minutes. Pumps should not stay on for too long.", 91);
                    UARTSendArray("\n\r", 2);
                    ONminuteset = 5; ONsecondset = 0;
                }
            }
            if (ONtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'a' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'd': {
            if (ONtimersetmode == 1) {
                UARTSendArray("Decreasing by 1 minute length of ON-timer.", 42);
                UARTSendArray("\n\r", 2);
                printontimerdisp = 1;
                ONminuteset = ONminuteset - 1;
                if (ONminuteset < 0) {
                    UARTSendArray("ON-Timer minute set cannot be lower than 0 minutes.", 51);
                    UARTSendArray("\n\r", 2);
                    ONminuteset = 0;
                }
                if ((ONsecondset < 1) && (ONminuteset <= 0 )) {
                    UARTSendArray("ON-Timer duration cannot be shorter than 1 second.", 50);
                    UARTSendArray("\n\r", 2);
                    ONsecondset = 1;
                }
            }
            if (ONtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'a' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        case 'D': {
            if (ONtimersetmode == 1) {
                UARTSendArray("Increasing by 1 minute length of ON-timer.", 42);
                UARTSendArray("\n\r", 2);
                printontimerdisp = 1;
                ONminuteset = ONminuteset + 1;
                if (ONminuteset > 5) {
                    UARTSendArray("ON-Timer minute set cannot be higher than 5 minutes. Pumps should not stay on for too long.", 91);
                    UARTSendArray("\n\r", 2);
                    ONminuteset = 5;
                }
                if ((ONminuteset >= 5) && (ONsecondset > 0)) {
                    UARTSendArray("ON-Timer minute set cannot be higher than 5 minutes. Pumps should not stay on for too long.", 91);
                    UARTSendArray("\n\r", 2);
                    ONminuteset = 5; ONsecondset = 0;
                }
            }
            if (ONtimersetmode == 0) {
                UARTSendArray("Cannot use this command yet. Press 'a' to enter OFF-Timer Set Mode first.", 73);
                UARTSendArray("\n\r", 2);
            }
        }
        break;
        /*******************************************************************************/
        
        default: {
            UARTSendArray("Unknown Command: ", 17); // print this if the inputting character is not recognized by the case-statement
            UARTSendArray(&data, 1);
            UARTSendArray("\n\r", 2);
        }
        break;
    } //close the switch-statement
    } //close the resetenable if-statement
    
    if (resetenable == 0) {
    switch(data) {
        case 'r': {
            UARTSendArray("Controller is ON: pH is now being measured.", 43);
            UARTSendArray("\n\r", 2);
            UARTSendArray("****************************************************************************************************", 100);
            UARTSendArray("\n\r", 2);
            UARTSendArray("Press 'e' to print once the current pH value.", 45);
            UARTSendArray("\n\r", 2);
            UARTSendArray("\n\r", 2);
            UARTSendArray("****************************************************************************************************", 100);
            UARTSendArray("\n\r", 2);
            UARTSendArray("Timers must be defined before they start. Define their times first,", 67);
            UARTSendArray("\n\r", 2);
            UARTSendArray("then turn off Off/ON-Timer Set Mode to activate that timer. Both timers must be on for controller to function properly.", 119);
            UARTSendArray("\n\r", 2);
            UARTSendArray("pH Pump Controller single-character commands are listed below:", 62);
            UARTSendArray("\n\r", 2);
            UARTSendArray("****************************************************************************************************", 100);
            UARTSendArray("\n\r", 2);
            UARTSendArray("\n\r", 2);
            UARTSendArray("****************************************************************************************************", 100);
            UARTSendArray("\n\r", 2);
            UARTSendArray("Main commands:   'L' = Low Bound adjust mode.   'H' = High Bound adjust mode.   'z' = OFF-Timer set mode.   'a' = ON-Timer set mode.", 132);
            UARTSendArray("\n\r", 2);
            UARTSendArray("\n\r", 2);
            UARTSendArray("\n\r", 2);
            UARTSendArray("Other commands 1:   'w' = Print values of measured pH.   'W' = Print values of current OFFTimer and ONTimer counter integers.", 125);
            UARTSendArray("\n\r", 2);
            UARTSendArray("Other commands 2:   'e' & 'E' = Print once/Continuous printing of pH values.   '/' = Open Help Menu to help convert ADC values into pH values.", 142);
            UARTSendArray("\n\r", 2);
            UARTSendArray("Other commands 3:   'R' = Turn off controller, reset all timers and settings.", 77);
            UARTSendArray("\n\r", 2);
            UARTSendArray("****************************************************************************************************", 100);
            UARTSendArray("\n\r", 2);
            UARTSendArray("\n\r", 2);
            resetenable = 1;
        }
        break;
        default: {
            UARTSendArray("\n\r", 2); 
	        UARTSendArray("\n\r", 2); 
	        UARTSendArray("Controller is OFF: Press 'r' to initialize.", 43);
            UARTSendArray("\n\r", 2);
        }
    }
    }
    P1OUT &= ~BIT0; //red led
}