#include <msp430g2553.h>
#include <stdio.h>
#include <stdint.h>

static char data;
int loop = 1, delay = 0;
typedef union {
  struct {
    uint8_t hh;
    uint8_t hl;
    uint8_t th;
    uint8_t tl;
    uint8_t crc;
  } val;
  uint8_t bytes[5];
} dht22data;

void dht_start_read();
int dht_get_temp();
int dht_get_rh();
char txbuf[256];

void uart_init() {
  UCA0CTL0 = 0;
  UCA0CTL1 |= UCSWRST; // Put USCI in reset mode
  UCA0CTL1 = UCSSEL_2 | UCSWRST; // Use SMCLK, still reset
  UCA0MCTL = UCBRF_0 | UCBRS_6;
  UCA0BR0 = 131; // 9600 bauds
  UCA0BR1 = 6;
  UCA0CTL1 &= ~UCSWRST; // Normal mode
	P1SEL2 |= (BIT1 + BIT2); // Set pins for USCI
	P1SEL |= (BIT1 + BIT2);
  IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
}

void uart_send(int len) {
  int i;
  for(i = 0; i < len; i++) {
    while (UCA0STAT & UCBUSY);
    UCA0TXBUF = txbuf[i];
  }
}

enum {
  DHT_IDLE = 0,
  DHT_TRIGGERING,
  DHT_WAITING_ACK,
  DHT_ACK_LOW,
  DHT_ACK_HIGH,
  DHT_IN_BIT_LOW,
  DHT_IN_BIT_HIGH,
} dht_current_state;

dht22data dht_data;

uint8_t dht_data_byte, dht_data_bit;

int dht_get_temp() {
  uint16_t temp_temp;
  //while (dht_current_state != DHT_IDLE); // Comment out to continuously check temperature and humidity
  temp_temp = (((dht_data.val.th&0x7f)<<8)+dht_data.val.tl);
  return ((-1)*((dht_data.val.th&0x80)>>7)+temp_temp);
}

int dht_get_rh() {
  uint16_t temp_rh;
  //while (dht_current_state != DHT_IDLE); // Comment out to continuously check temperature and humidity
  temp_rh = (dht_data.val.hh<<8)+dht_data.val.hl;
  return temp_rh;
}

void dht_start_read() {
  // First, low pulse of 1ms
  P2OUT &= ~BIT0;
  P2SEL &= ~BIT0;
  P2DIR |= BIT0;

  TA1CCTL0 &= ~CCIFG;
  TA1CCR0 = 16000u;
  TA1CCTL0 = CCIE;
  TA1CTL = TACLR;
  TA1CTL = TASSEL_2 | ID_0 | MC_1;

  dht_current_state = DHT_TRIGGERING;
}

void __attribute__((interrupt (TIMER1_A0_VECTOR))) timer1_a0_isr() {
  TA1CCTL0 &= ~CCIFG;
  // This handles only TA1CCR0 interrupts
  switch (dht_current_state) {
  case DHT_IDLE:
    break; // Shouldn't be here
  case DHT_TRIGGERING:
    // 1ms has passed since setting the pin low
    // Let P2.0 go high and set Compare input on T1
    P2DIR &= ~BIT0; // input
    P2SEL |= BIT0; // Timer1_A3.CCI0A input
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_2 | CCIS_0 | CAP | CCIE; // Capture on falling edge
    dht_current_state = DHT_WAITING_ACK;
    break;
  case DHT_WAITING_ACK:
    // I don't care about timings here...
    P2DIR &= ~BIT0; // input
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_1 | CCIS_0 | CAP | CCIE; // Capture on rising edge
    dht_current_state = DHT_ACK_LOW;
    break;
  case DHT_ACK_LOW:
    // I don't care about timings here either...
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_2 | CCIS_0 | CAP | CCIE; // Capture on falling edge
    dht_current_state = DHT_ACK_HIGH;
    dht_data_byte = dht_data_bit = 0;
    break;
  case DHT_ACK_HIGH:
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_1 | CCIS_0 | CAP | CCIE; // Capture on rising edge
    dht_current_state = DHT_IN_BIT_LOW;
    break;
  case DHT_IN_BIT_LOW:
    TA1CTL = TACLR;
    TA1CTL = TASSEL_2 | ID_0 | MC_2;
    TA1CCTL0 = CM_2 | CCIS_0 | CAP | CCIE; // Capture on falling edge
    dht_current_state = DHT_IN_BIT_HIGH;
    break;
  case DHT_IN_BIT_HIGH:
    // OK now I need to measure the time since last time
    dht_data.bytes[dht_data_byte] <<= 1;
    if (TA1CCR0 > 750) {
      // Long pulse: 1
      dht_data.bytes[dht_data_byte] |= 1;
    }
    if (++dht_data_bit >= 8) {
      dht_data_bit = 0;
      dht_data_byte++;
    }
    if (dht_data_byte >= 5) {
      // I'm done, bye
      // TODO: check CRC
      TA1CTL = TACLR;
      dht_current_state = DHT_IDLE;
    } else {
      TA1CTL = TACLR;
      TA1CTL = TASSEL_2 | ID_0 | MC_2;
      TA1CCTL0 = CM_1 | CCIS_0 | CAP | CCIE; // Capture on rising edge
      dht_current_state = DHT_IN_BIT_LOW;
    }
    break;
  }
}

int main() {
  WDTCTL = WDTPW | WDTHOLD;
  DCOCTL = 0;
  BCSCTL1 = CALBC1_16MHZ;
  DCOCTL = CALDCO_16MHZ;

  uart_init();
  _BIS_SR(GIE);
  dht_start_read();
  int t = dht_get_temp();
  int h = dht_get_rh();
  
  uart_send(sprintf(txbuf, "Program initialized. \r\n \r\n"));
  uart_send(sprintf(txbuf, "Welcome to the DHT22 sensor reader! \r\n \r\n"));
  uart_send(sprintf(txbuf, "Press 'space' or 'p' to print once the air humidity and air temperature readings. \r\n \r\n"));
  
  while(1) {
    if (loop > 1) {
        dht_start_read();
        int t = dht_get_temp();
        int h = dht_get_rh();
        uart_send(sprintf(txbuf, "Temperature (C) = %d.%d \r\n \r\n", t/10, t%10));
        uart_send(sprintf(txbuf, "Humidity (%%) = %d.%d \r\n \r\n", h/10, h%10));
        loop--;
    }
  }
  return 0;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    data = UCA0RXBUF;
    switch(data){
        case 'p': {
            uart_send(sprintf(txbuf, "Received command: 'p' \r\n \r\n"));
            uart_send(sprintf(txbuf, "Printing the values: \r\n \r\n \r\n \r\n"));
            loop = 2;
        }
        break;
        case 'P': {
            uart_send(sprintf(txbuf, "Received command: 'P' \r\n \r\n"));
            uart_send(sprintf(txbuf, "Printing the values: \r\n \r\n \r\n \r\n"));
            loop = 2;
        }
        break;
        case ' ': {
            uart_send(sprintf(txbuf, "Received command: 'space' \r\n \r\n"));
            uart_send(sprintf(txbuf, "Printing the values: \r\n \r\n \r\n \r\n"));
            loop = 2;
        }
        break;
        default: {
            uart_send(sprintf(txbuf, "Unknown command: %d \r\n \r\n", data));
            uart_send(sprintf(txbuf, "Press 'space' or 'p' to print once the air humidity and air temperature readings. \r\n \r\n"));
        }
        break;
    }
}