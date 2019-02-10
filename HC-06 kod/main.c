#include "io430.h"

#define TXD BIT2
#define RXD BIT1

int counter=0;
volatile unsigned int i; //Counter

char string[]={"a\0"}; //gönderilecek string

void send_string(char string[]);
void uart_init(void);

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  DCOCTL = 0; // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ; // Set DCO
  DCOCTL = CALDCO_1MHZ;

  uart_init();
  TACCR0=62500;//(62500*8)/1000000=0.5sn
  TACCTL0=CCIE;
  TACTL=MC_1+ID_3+TASSEL_2+TACLR;

  __bis_SR_register(GIE); // Enter LPM0 w/ int until Byte RXed

  while(1)
  {
    __bis_SR_register(LPM0_bits);
  }

 }

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
  UCA0TXBUF = string[++i];
   if(string[i]=='\0'){
      UC0IE &= ~UCA0TXIE; //tx intrerrupt disabled
   }
}
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void){
  counter++;
  if(counter==4){
    i=0;
    UC0IE |= UCA0TXIE; //tx intrerrupt activated
    UCA0TXBUF = string[i];
    counter=0;
  }
}
void uart_init(void){
  P1SEL |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
  P1SEL2 |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
  P1OUT &= 0x00;

  UCA0CTL1 |= UCSWRST;
  UCA0CTL1 |= UCSSEL_2; // SMCLK
  UCA0BR0 = 104;                        //BAUD RATE 9600
  UCA0BR1 = 0;                          //SMCLK/BAUDRATE=(UCAxBR0 + UCAxBR1 × 256)
  UCA0MCTL = UCBRS_1;                  //MODULATION VALUE  = 0 FROM DATASHEET
  UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
  UC0IE |= UCA0RXIE; // Enable USCI_A0 RX interrupt

}
void send_string(char string[]){
  i=0;
  UC0IE |= UCA0TXIE; //tx intrerrupt activated
  UCA0TXBUF = string[i];
}
