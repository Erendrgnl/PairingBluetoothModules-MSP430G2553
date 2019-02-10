#include "io430.h"

#define TXD BIT2
#define RXD BIT1
#define SM_clk 1100000
#define servo_freq 50

char recieved_data='b';
int counter=0;

void uart_init(void);
void PWM_init(void);
void servo_control(void);

int main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  DCOCTL = 0; // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ; // Set DCO
  DCOCTL = CALDCO_1MHZ;

  uart_init();
  PWM_init();

  TACCR0=62500;//(62500*8)/1000000=0.5sn
  TACCTL0=CCIE;
  TACTL=MC_1+ID_3+TASSEL_2+TACLR;

  while(1){
  servo_control();
  __bis_SR_register(GIE+LPM0_bits);
  }

}
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  counter=0;
  recieved_data=UCA0RXBUF;
  __bic_SR_register_on_exit(CPUOFF);//bu kod olmadiginda lpm modunda takili kaliyor!

}
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void){
  counter++;
  if(counter==10){
    counter=0;
    recieved_data='b';
  }
  __bic_SR_register_on_exit(CPUOFF);
}
void servo_control(void){
  if(recieved_data=='a'){
    for(;TA1CCR1>0;TA1CCR1-=10){__delay_cycles(5000);}
  }
  if(recieved_data=='b'){
    for(;TA1CCR1<1700;TA1CCR1+=10){__delay_cycles(5000);}
  }
}
void uart_init(void){
  P1SEL |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
  P1SEL2 |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
  P1OUT &= 0x00;

  UCA0CTL1 |= UCSWRST;
  UCA0CTL1 |= UCSSEL_2; // SMCLK
  UCA0BR0 = 104;                        //BAUD RATE 38400
  UCA0BR1 = 0;                          //SMCLK/BAUDRATE=(UCAxBR0 + UCAxBR1 × 256)
  UCA0MCTL = UCBRS_1;
  UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
  UC0IE |= UCA0RXIE; // Enable USCI_A0 RX interrupt
}
void PWM_init(void)
{
  int PWM_period = SM_clk /servo_freq;
  P2DIR |= BIT2; //direction is set
  P2SEL |= BIT2;// port 2 function is set.
  TA1CCR0 = PWM_period-1; // pwm period
  TA1CCR1 = 1700; //duty cycle = TA0CCR0/ TA0CCR1 1700 yaptim
  TA1CCTL1 = OUTMOD_7;
  TA1CTL=TASSEL_2+MC_1; // starting timer by setting clock source SMCLK of timer and UP mode

}