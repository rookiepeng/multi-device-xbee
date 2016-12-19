/**
* Multiple radars for motion detection with XBee
* Sampling rate: 20 Hz
* By: Zhengyu Peng
* Dec. 18, 2016
*/

#include "io430.h"

/** configure it before using */
char radarID = 0x03; // change ID for different devices

/** initial parameters */
char queryMode;
char continuousMode;
char isQuery = 1;
char isContinuous = 0;

int Code0 = 0x0040;
int Code1 = 0x80C0;

int I = 0; // value for DAC1
int Q = 0; // value for DAC2

char startADC = 0;

char saveTo1 = 1;
char cache1ready = 0;
char cache2ready = 0;
int saveIndex = 0;

int ccr = 1638; // sampling rate = 32768/ccr

/** cache 1 */
char cacheIHigh[200] = {0}; // 20 Hz 10s data
char cacheILow[200] = {0};
char cacheQHigh[200] = {0};
char cacheQLow[200] = {0};

/** cache 2 */
char cacheIIHigh[200] = {0};
char cacheIILow[200] = {0};
char cacheQQHigh[200] = {0};
char cacheQQLow[200] = {0};

//char sendCache1=0;
//char sendCache2=0;

void sendCache(char cache);
void clockConfig();

void main(void)
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  queryMode = 0x10 + radarID;      // query mode, send out data when query signal received
  continuousMode = 0x20 + radarID; // continous mode, send out data continuously

  clockConfig();

  /** ADC configuration */
  P6SEL = 0x03;
  ADC12CTL0 = SHT0_2 + REF2_5V + REFON + MSC + ADC12ON;
  ADC12CTL1 = SHP + CONSEQ_3;
  ADC12MCTL0 = INCH_0;
  ADC12MCTL1 = INCH_1 + EOS;
  ADC12IE = 0x02;       // Enable ADC12IFG.1
  ADC12CTL0 |= ENC;     // Enable conversions
  ADC12CTL0 |= ADC12SC; // Start convn - software trigger

  /** UART configuration */
  P3OUT &= ~(BIT4 + BIT5);
  P3SEL = 0x30;               // P3.4,5 = USCI_A0 TXD/RXD
  UCA0CTL1 |= UCSSEL_1;       // CLK = ACLK
  UCA0BR0 = 0x03;             // 32kHz/9600 = 3.41
  UCA0BR1 = 0x00;             //
  UCA0MCTL = UCBRS1 + UCBRS0; // Modulation UCBRSx = 3
  UCA0CTL1 &= ~UCSWRST;       // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;            // Enable USCI_A0 RX interrupt

  /** Timer configuration */
  TACCTL0 = CCIE; // CCR0 interrupt enabled
  CCR0 = 1;
  TACTL = TASSEL_1 + MC_1; //

  /** Enter LPM3, interrupts enabled */
  __bis_SR_register(LPM3_bits + GIE);
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12ISR(void)
{
  I = ADC12MEM0;
  Q = ADC12MEM1;
}

#pragma vector = TIMERA0_VECTOR
__interrupt void Timer_A(void)
{
  CCR0 = ccr; // sampling rate = 32768/CCR0

  /**
  * I channel
  * |0|0| | | | | | | Higher bits
  * |0|1| | | | | | | Lower bits
  *
  * Q channel
  * |1|0| | | | | | | Higher bits
  * |1|1| | | | | | | Lower bits
  */

  Code0 = 0x0040;
  Code0 |= (I >> 6) << 8;
  Code0 |= (I & 0x003f);

  Code1 = 0x80C0;
  Code1 |= (Q >> 6) << 8;
  Code1 |= (Q & 0x003f);

  if (startADC)
  {
    if (isContinuous)
    {
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = (Code0 & 0xff00) >> 8;
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = (Code0 & 0x00ff);
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = (Code1 & 0xff00) >> 8;
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = (Code1 & 0x00ff);
    }
    else if (isQuery)
    {
      if (cache1ready == 0 && saveTo1)
      {
        cacheIHigh[saveIndex] = (Code0 & 0xff00) >> 8;
        cacheILow[saveIndex] = (Code0 & 0x00ff);
        cacheQHigh[saveIndex] = (Code1 & 0xff00) >> 8;
        cacheQLow[saveIndex] = (Code1 & 0x00ff);
        if (saveIndex < 199)
        {
          saveIndex++;
        }
        else
        {
          saveIndex = 0;
          saveTo1 = 0;
          cache1ready = 1;
          //P2OUT^=0x10;
          //P2OUT |= 0x10;
        }
      }
      else if (cache2ready == 0 && saveTo1 == 0)
      {
        cacheIIHigh[saveIndex] = (Code0 & 0xff00) >> 8;
        cacheIILow[saveIndex] = (Code0 & 0x00ff);
        cacheQQHigh[saveIndex] = (Code1 & 0xff00) >> 8;
        cacheQQLow[saveIndex] = (Code1 & 0x00ff);
        if (saveIndex < 199)
        {
          saveIndex++;
        }
        else
        {
          saveIndex = 0;
          saveTo1 = 1;
          cache2ready = 1;
          //P2OUT^=0x10;
          //P2OUT &= 0xEF;
        }
      }
    }
  }
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  startADC = 1;             // start to save ADC data as soon as a UART byte is received
  if (UCA0RXBUF == radarID) // query radar in queryMode
  {
    if (cache1ready != 0 || cache2ready != 0)
    {
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = 0x01; // response to the computer, one of the caches is ready
      if (cache1ready)  // send cache 1
      {
        sendCache(1);
        //sendCache1=1;
      }
      else if (cache2ready)
      {
        sendCache(2);
        //sendCache2=1;
      }
    }
    else // send cache 2
    {
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = 0x00; // caches are not ready
    }
  }
  else if (UCA0RXBUF == queryMode)
  {
    ccr = 1638; // 20 Hz
    isQuery = 1;
    isContinuous = 0;
    //oneChannel = 0;
  }
  else if (UCA0RXBUF == continuousMode)
  {
    ccr = 410; // 80 Hz
    isQuery = 0;
    isContinuous = 1;
  }
}

void sendCache(char cache)
{
  if (cache == 1)
  {
    for (int i = 0; i < 200; i++)
    {
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = cacheIHigh[i];
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = cacheILow[i];

      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = cacheQHigh[i];
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = cacheQLow[i];
    }
    cache1ready = 0;
  }
  else if (cache == 2)
  {
    for (int i = 0; i < 200; i++)
    {
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = cacheIIHigh[i];
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = cacheIILow[i];

      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = cacheQQHigh[i];
      while (!(IFG2 & UCA0TXIFG))
        ;
      UCA0TXBUF = cacheQQLow[i];
    }
    cache2ready = 0;
  }
}

void clockConfig()
{
  if (CALBC1_8MHZ == 0xFF) // If calibration constant erased
  {
    while (1)
      ; // do not load, trap CPU!!
  }
  DCOCTL = 0;            // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_8MHZ; // Set DCO to 8MHz
  DCOCTL = CALDCO_8MHZ;

  // Select 32kHz Crystal for ACLK
  // BCSCTL1 &= (~XTS);	// ACLK = LFXT1CLK
  // BCSCTL2 &= ~(BIT4|BIT5);	// 32768Hz crystal on LFXT1

  // Clock output
  //            MSP430F261x/241x
  //             -----------------
  //         /|\|              XIN|-
  //          | |                 | 32kHz
  //          --|RST          XOUT|-
  //            |                 |
  //            |             P5.6|-->ACLK = 32kHz
  //            |             P5.5|-->SMCLK = 8MHz
  //            |             P5.4|-->MCLK = DCO
  //            |             P5.3|-->MCLK/10
  // P5DIR |= 0x78;
  // P5SEL |= 0x70;
}
