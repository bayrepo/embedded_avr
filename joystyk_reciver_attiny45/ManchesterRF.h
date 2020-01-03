#ifndef MANCHESTERRF_h
#define MANCHESTERRF_h

#define F_CPU 8000000
#define __AVR_ATtinyX5__ 1
#define __DISABLE_TX__
//#define DEBUG_S

#define _BV(bit) (1 << (bit)) 

//timer scaling factors for different transmission speeds
#define MAN_300 0
#define MAN_600 1
#define MAN_1200 2
#define MAN_2400 3
#define MAN_4800 4
#define MAN_9600 5
#define MAN_19200 6
#define MAN_38400 7

//setup timing for receiver
#define MinCount 52//pulse lower count limit on capture
#define MaxCount 78//pulse higher count limit on capture
#define MinLongCount 116//pulse lower count on double pulse
#define MaxLongCount 152//pulse higher count on double pulse

//setup timing for transmitter
#define HALF_BIT_INTERVAL 5120U //(=48 * 1024 * 1000000 / 8000000Hz) microseconds for speed factor 0 (300baud)

//it's common to zero terminate a string or to transmit small numbers involving a lot of leading zeroes
//those zeroes may be mistaken for training pattern, confusing the receiver and resulting high packet lost, 
//therefore we xor the data with random decoupling mask
#define DECOUPLING_MASK 0b11001010 

#define RX_MODE_PRE 0
#define RX_MODE_SYNC 1
#define RX_MODE_DATA 2
#define RX_MODE_MSG 3
#define RX_MODE_IDLE 4

#define TimeOutDefault -1 //the timeout in msec default blocks

void ManchesterRF(uint8_t SF);
    
void ManchesterRF_setBalance(int8_t bf);

void ManchesterRF_TXInit(uint8_t port, uint8_t mask); //set port and mask
void ManchesterRF_RXInit(uint8_t port, uint8_t mask); //set port and mask


uint8_t ManchesterRF_transmitArray(uint8_t size, uint8_t *data); // transmit array of bytes
uint8_t ManchesterRF_receiveArray(uint8_t *size, uint8_t **data); // receive array of bytes
    
uint8_t ManchesterRF_transmitPacket(uint8_t size, uint8_t from, uint8_t to, uint8_t meta, uint8_t *payload); //decode receive buffer into a packet, return 1 if valid packet received, otherwise 0
uint8_t ManchesterRF_receivePacket(uint8_t *size, uint8_t *from, uint8_t *to, uint8_t *meta, uint8_t **payload); //decode receive buffer into a packet, return 1 if valid packet received, otherwise 0
    
uint8_t ManchesterRF_transmitByte(uint8_t data);
uint8_t ManchesterRF_receiveByte(uint8_t *data);

uint8_t ManchesterRF_transmitWord(uint16_t data);
uint8_t ManchesterRF_receiveWord(uint16_t *data);

void ManchesterRF_beginReceive(void);
void ManchesterRF_stopReceive(void);
int ManchesterRF_available();

void ManchesterRF_setFunction(void (*f_ptr)());

#endif
