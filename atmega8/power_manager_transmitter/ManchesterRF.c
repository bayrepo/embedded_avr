#include <avr/io.h>
#include <avr/interrupt.h>

#include "ManchesterRF.h"
#include "options.h"

#include <avr/delay.h>

#define byte unsigned char
#define MAKE_BINARY(a,b,c,d,e,f,g,h) (((a)<<7)|((b)<<6)|((c)<<5)|((d)<<4)|((e)<<3)|((f)<<2)|((g<<1)|h))

uint8_t speedFactor;
uint16_t delay10;
uint16_t delay20;
uint16_t delay11;
uint16_t delay21;

static void sendZero(void);
static void sendOne(void);

#ifndef __DISABLE_TX__
static uint8_t directTxMask;
static uint8_t directTxPort;
#endif
static int8_t balanceFactor;

#define MAN_MESSAGE_SIZE MESSAGE_LEN

#define MAN_BUF_SIZE 4


#define MAN_IS_BUFF_EMPTY (man_rx_buff_end == man_rx_buff_start)
#define MAN_IS_BUFF_FULL ((man_rx_buff_end+1) % MAN_BUF_SIZE == man_rx_buff_start)

#ifndef __DISABLE_RX__
uint8_t man_rx_buff[MAN_BUF_SIZE][MAN_MESSAGE_SIZE+1];
volatile uint8_t man_rx_buff_start = 0;
volatile uint8_t man_rx_buff_end = 0;
#endif

#ifndef __DISABLE_TX__
uint8_t man_tx_buff[MAN_MESSAGE_SIZE+1];
#endif

#ifndef __DISABLE_RX__
static uint8_t directRxPort = 0x00;
static uint8_t directRxMask = 0x00;
static uint8_t rx_sample = 0;
static uint8_t rx_last_sample = 0;
static uint8_t rx_pulse_width = 0;
static uint8_t rx_pulse_width_inc = 8;
static uint8_t rx_sync_count = 0;
static uint8_t rx_mode = RX_MODE_IDLE;

static uint16_t rx_manBits = 0; //the received manchester 32 bits
static uint8_t rx_numMB = 0; //the number of received manchester bits
static uint8_t rx_curByte = 0;

#define MAX_RX_BUFFER 128

uint8_t sranie[MAX_RX_BUFFER];
uint8_t isranie = 0;
#endif

static void delay_us(int d)
{
	int i;
	for (i = 0; i < d; i++) {
		_delay_us(1);
	}
}

void ManchesterRF(uint8_t SF){
	speedFactor = SF;
}

/**************************  TRANSMIT INIT ****************************/

void ManchesterRF_setBalance(int8_t bf) {
  balanceFactor = bf;
}

#ifndef __DISABLE_TX__
static void TXInit() {

  //emprirically determined values to compensate for the time loss in overhead

  #if F_CPU < 8000000UL
    uint16_t compensationFactor = 48; //24;//40;
  #elif F_CPU < 16000000UL
    uint16_t compensationFactor = 6;
  #else //16000000Mhz
    uint16_t compensationFactor = 0;
  #endif

  #if F_CPU < 8000000UL
    uint16_t compensationFactor2 = 0;
  #elif F_CPU < 16000000UL
    uint16_t compensationFactor2 = 0;
  #else //16000000Mhz
    uint16_t compensationFactor2 = 0;
  #endif

  /*
  Base delay | speed factor
  3072 - 0
  1536 - 1
  768  - 2
  384  - 3
  192  - 4
  96   - 5
  48   - 6
  24   - 7
  12   - 8

  */

  //this must be signed int
  int temp10 = (HALF_BIT_INTERVAL >> speedFactor) - compensationFactor + balanceFactor;
  int temp11 = (HALF_BIT_INTERVAL >> speedFactor) - compensationFactor - balanceFactor;

  int temp20 = (HALF_BIT_INTERVAL >> speedFactor) - compensationFactor2 + balanceFactor;
  int temp21 = (HALF_BIT_INTERVAL >> speedFactor) - compensationFactor2 - balanceFactor;

  if (temp10 < 0) {
    temp21 += temp10; //borrow from second delay to maintain constant speed
    temp10 = 0;
  }

  if (temp11 < 0) {
    temp20 += temp11; //borrow from second delay to maintain constant speed
    temp11 = 0;
  }

  if (temp21 < 0) temp21 = 0; //too slow for such speed
  if (temp20 < 0) temp20 = 0; //too slow for such speed

  delay10 = temp10;
  delay11 = temp11;
  delay20 = temp20;
  delay21 = temp21;
}


void ManchesterRF_TXInit(uint8_t port, uint8_t mask) {
  //set pin as output and low
  DDRC |= mask; 
  PORTC &= ~mask;
  directTxPort = port;
  directTxMask = mask;
  TXInit();
}
#endif

#ifndef __DISABLE_RX__
/***********************  RECEIVER INIT *************************/

static void RXInit() {

  //setup timers depending on the microcontroller used

    TCCR1A = _BV(WGM12); // reset counter on match
    TCCR1B =  _BV(CS11); // 1/8 prescaler
    #if F_CPU == 1000000UL
      OCR1A = (64 >> speedFactor) - 1;
    #elif F_CPU == 8000000UL
      OCR1A = (512 >> speedFactor) - 1;
    #elif F_CPU == 16000000UL
      OCR1A = (1024 >> speedFactor) - 1;
    #else
    #error "Manchester library only supports 1Mhz, 8mhz, 16mhz on ATMega8"
    #endif
    TIFR = _BV(OCF1A);  // clear interrupt flag
    TIMSK = _BV(OCIE1A); // Turn on interrupt
    TCNT1 = 0; // Set counter to 0

    rx_mode = RX_MODE_PRE;
}

void ManchesterRF_RXInit(uint8_t port, uint8_t mask) {
  DDRC &= ~mask; 
  PORTC &= ~mask; break;
  directRxPort = port;
  directRxMask = mask;
  RXInit();
}

#endif

#ifndef __DISABLE_TX__
static void sendZero(void) {
	if (directTxPort && directTxMask) { //use direct port manipulation (much faster)
		delay_us(delay11);
		//go HIGH
        PORTC |= directTxMask;
		delay_us(delay20);

		//go LOW
		PORTC &= ~directTxMask;
	}
}//end of send a zero


static void sendOne(void) {
	if (directTxMask) { //use direct port manipulation (much faster)
		delay_us(delay10);
		//go LOW
		PORTC &= ~directTxMask;


		delay_us(delay21);
		//go HIGH
		PORTC |= directTxMask;
	}
}//end of send one

uint8_t ManchesterRF_transmitArray(uint8_t size, uint8_t *data) {
  if (size == 0) return 0;

  #if F_CPU < 88000000UL
    char cSREG;
    cSREG = SREG; /* store SREG value */
    cli();
  #endif

  //send preamble
  for( uint8_t i = 0; i < 14; i++) {
    sendZero();
    //match the overhead in the data cycle, I have to do it this way, so optimizer won't mess with it
    #if F_CPU < 88000000UL
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
      asm volatile ("mov r0, r0"); //wait one cycle
    #endif
	}

  // Send a single 1
  sendOne(); //start data pulse


  //match the overhead in the data cycle, I have to do it this way, so optimizer won't mess with it
  #if F_CPU < 88000000UL
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
    asm volatile ("mov r0, r0"); //wait one cycle
  #endif

  // Send the user data

  for (uint8_t i = 0; i < size; i++) {
    uint8_t mask = 0x01; //mask to send bits
    for (uint8_t j = 0; j < 8; j++) {
      if (((data[i] ^ DECOUPLING_MASK) & mask) == 0)
        sendZero();
      else
        sendOne();
      mask <<= 1; //get next bit
    }//end of byte
  }//end of data

  // Send terminating 0 to correctly terminate the previous bit and to turn the transmitter off
  sendZero();
  sendZero();

  #if F_CPU < 88000000UL
    SREG = cSREG;
  #endif
  return size;
}//end of send the data
#endif

#ifndef __DISABLE_RX__
uint8_t ManchesterRF_receiveArray(uint8_t *size, uint8_t **data) {
  if (MAN_IS_BUFF_EMPTY) return 0;
  *size = man_rx_buff[man_rx_buff_start][0];
  *data = &man_rx_buff[man_rx_buff_start][1];
  man_rx_buff_start = (man_rx_buff_start + 1) % MAN_BUF_SIZE; //remove message from the buffer
  return 1;
}
#endif

#ifndef __DISABLE_TX__
uint8_t ManchesterRF_transmitPacket(uint8_t size, uint8_t from, uint8_t to, uint8_t meta, uint8_t *payload) {
  man_tx_buff[0] = from;
  man_tx_buff[1] = to;
  man_tx_buff[2] = meta;
  for (uint8_t i = 0; i < size && i < 10; i++) {
    man_tx_buff[i + 3] = payload[i];
  }

  //calculate the modification of a Fletcher checksum
  uint8_t c0 = size + 4;
  uint8_t c1 = size + 4;
  for(int i = 0; i < size + 3; i++) { //zeroth element is the size, it's not part of the packet, but we will add it as an extra check; sizeth element is the checksum we are calculating
    c0 += man_tx_buff[i];
    c1 += c0;
  }
  c1 ^= c0;
  man_tx_buff[size + 3] = c1;

  return ManchesterRF_transmitArray(size + 4, man_tx_buff);
}
#endif

#ifndef __DISABLE_RX__
uint8_t ManchesterRF_receivePacket(uint8_t *size, uint8_t *from, uint8_t *to, uint8_t *meta, uint8_t **payload) {
  if (MAN_IS_BUFF_EMPTY) return 0;
  if (man_rx_buff[man_rx_buff_start][0] < 4) { //not enough bytes for a valid packet
    man_rx_buff_start = (man_rx_buff_start + 1) % MAN_BUF_SIZE; //remove message from the buffer
    return 0;
  }
  uint8_t msize = man_rx_buff[man_rx_buff_start][0];
  *size = msize - 4; //payload size
  *from = man_rx_buff[man_rx_buff_start][1];
  *to   = man_rx_buff[man_rx_buff_start][2];
  *meta = man_rx_buff[man_rx_buff_start][3];
  *payload = &man_rx_buff[man_rx_buff_start][4];
  uint8_t ch = man_rx_buff[man_rx_buff_start][msize];

  //calculate the modification of a Fletcher checksum
  uint8_t c0 = 0;
  uint8_t c1 = 0;
  for(int i = 0; i < msize; i++) { //zeroth element is the size, it's not part of the packet, but we will add it as an extra check; sizeth element is the checksum we are calculating
    c0 += man_rx_buff[man_rx_buff_start][i];
    c1 += c0;
  }
  c1 ^= c0;
  man_rx_buff_start = (man_rx_buff_start + 1) % MAN_BUF_SIZE; //remove message from the buffer
  if (ch == c1) return 1;
  return 0;
}
#endif

#ifndef __DISABLE_TX__
uint8_t ManchesterRF_transmitByte(uint8_t data) {
  return ManchesterRF_transmitArray(1, &data);
}
#endif

#ifndef __DISABLE_RX__
uint8_t receiveByte(uint8_t *data) {
  if (MAN_IS_BUFF_EMPTY) return 0;
  if (man_rx_buff[man_rx_buff_start][0] < 1) { //not enough bytes
    man_rx_buff_start = (man_rx_buff_start + 1) % MAN_BUF_SIZE; //remove message from the buffer
    return 0;
  }
  *data = man_rx_buff[man_rx_buff_start][1];
  man_rx_buff_start = (man_rx_buff_start + 1) % MAN_BUF_SIZE; //remove message from the buffer
  return 1;
}
#endif

#ifndef __DISABLE_TX__
uint8_t transmitWord(uint16_t data) {
  return ManchesterRF_transmitArray(2, (uint8_t*)&data);
}
#endif

#ifndef __DISABLE_RX__
uint8_t receiveWord(uint16_t *data) {
  if (MAN_IS_BUFF_EMPTY) return 0;
  if (man_rx_buff[man_rx_buff_start][0] < 2) { //not enough bytes for a valid word
    man_rx_buff_start = (man_rx_buff_start + 1) % MAN_BUF_SIZE; //remove message from the buffer
    return 0;
  }
  *data = (man_rx_buff[man_rx_buff_start][1] << 8 ) + man_rx_buff[man_rx_buff_start][2];
  man_rx_buff_start = (man_rx_buff_start + 1) % MAN_BUF_SIZE; //remove message from the buffer
  return 1;
}

int ManchesterRF_available() {
  return !MAN_IS_BUFF_EMPTY;
}

void MANRX_BeginReceive(void) {
	rx_mode = RX_MODE_PRE;
}

void MANRX_BeginReceiveBytes(uint8_t maxBytes, uint8_t *data) {
	rx_mode = RX_MODE_PRE;
}

void MANRX_StopReceive(void) {
	rx_mode = RX_MODE_IDLE;
}

void ManchesterRF_beginReceive(void) {
  MANRX_BeginReceive();
}

void ManchesterRF_stopReceive(void) {
  MANRX_StopReceive();
}

//rx_manBits, &rx_numMB, &rx_curByte
void AddManBit(uint8_t bit) {
  rx_manBits <<= 1;
  if (bit) rx_manBits |= 0x01;
  (rx_numMB)++;
  if (rx_numMB == 16) {
    uint8_t newData = 0;
    for (int8_t i = 0; i < 8; i++) {
      // rx_manBits holds 16 bits of manchester data
      // 1 = LO,HI
      // 0 = HI,LO
      // We can decode each bit by looking at the bottom bit of each pair.
      newData <<= 1;
      newData |= (rx_manBits & 1); // store the one
      rx_manBits = rx_manBits >> 2; //get next data bit
    }

    man_rx_buff[man_rx_buff_end][(rx_curByte) +1] = newData ^ DECOUPLING_MASK;
    man_rx_buff[man_rx_buff_end][0] = (rx_curByte)+1;

    //data[rx_curByte] = newData ^ DECOUPLING_MASK;
    (rx_curByte)++;
    rx_numMB = 0;
  }
}

void MAN_RX_INTERRUPT_HANDLER() {
  if (rx_mode < RX_MODE_MSG) {//receiving something

    // Increment counter
    rx_pulse_width += rx_pulse_width_inc;


    // Check receive pin
    if (directRxPort && directRxMask) { //use direct port manipulation (much faster)

	    rx_sample = PINC & directRxMask; 
        default: rx_sample = 0;
      }

    }


	if (rx_sample != rx_last_sample) { //pin has changed
      isranie++;
	  if (isranie>=MAX_RX_BUFFER){
		  isranie = 0;
	  }
      sranie[isranie] = rx_pulse_width;
			if (rx_mode == RX_MODE_PRE) {
				if (rx_sample) { // Wait for first transition to HIGH
					rx_pulse_width = 0;
					rx_sync_count = 0;
					rx_mode = RX_MODE_SYNC;
					isranie = 0;
					
				}
			} else if (rx_mode == RX_MODE_SYNC) { // Initial sync block
				if (((rx_sync_count < 26) || (rx_last_sample)) && ((rx_pulse_width < MinCount) || (rx_pulse_width > MaxCount))) {
					// First 20 bits and all 1 bits are expected to be regular
					// Transition was too slow/fast
					rx_mode = RX_MODE_PRE;
					isranie = 0;
				} else if ((!rx_last_sample) && ((rx_pulse_width < MinCount) || (rx_pulse_width > MaxLongCount))) {
					// 0 bits after the 20th bit are allowed to be a double bit
					// Transition was too slow/fast
					rx_mode = RX_MODE_PRE;
					isranie = 0;
				} else {
					rx_sync_count++;
					if ((!rx_last_sample) && (rx_sync_count >= 26) && (rx_pulse_width >= MinLongCount)) {
						// We have seen at least 10 regular transitions
						// Lock sequence ends with unencoded bits 01
						// This is encoded and TX as HI,LO,LO,HI
						// We have seen a long low - we are now locked!

  					    rx_mode = RX_MODE_DATA;
						rx_manBits = 0;
						rx_numMB = 0;
						rx_curByte = 0;
					} else if (rx_sync_count >= 64) { //preamble too long
						rx_mode = RX_MODE_PRE; //TODO do we really need to drop the packet?
						isranie = 0;
					}
					rx_pulse_width = 0;
				}
			} else if (rx_mode == RX_MODE_DATA) { // Receive data

				if ((rx_pulse_width < MinCount) || (rx_pulse_width > MaxLongCount)) {// wrong pulse length
					rx_mode = RX_MODE_PRE;
					if (rx_curByte > 0) { //if received something, save what we have
						if (!MAN_IS_BUFF_FULL) man_rx_buff_end = (man_rx_buff_end+1) % MAN_BUF_SIZE;
					}
					isranie = 0;
				} else {
					if (rx_pulse_width >= MinLongCount) {// was the previous bit a double bit?
						AddManBit(rx_last_sample);
					}
					if ((rx_sample) && (rx_curByte >= MAN_MESSAGE_SIZE)) { //message too long, truncate it and start over
//						rx_mode = RX_MODE_MSG;
						isranie = 0;
						rx_mode = RX_MODE_PRE;
						if (!MAN_IS_BUFF_FULL) man_rx_buff_end = (man_rx_buff_end+1) % MAN_BUF_SIZE;
					} else {
						// Add the current bit
						AddManBit(rx_sample);
						rx_pulse_width = 0;
					}
				}
			}
			// Get ready for next loop
			rx_last_sample = rx_sample;
		}
  }
} //MAN_RX_INTERRUPT_HANDLER

ISR(TIMER1_COMPA_vect)
{
	MAN_RX_INTERRUPT_HANDLER();
}

ISR(TIMER1_OVF_vect)
{
	MAN_RX_INTERRUPT_HANDLER();
}

//end
#endif