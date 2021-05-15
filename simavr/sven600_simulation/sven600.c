/*
* sven_vrv600_avr.c
*
* Created: 18.01.2021 23:56:08
* Author : Alexey Berezhok
*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>

#define SetBit(x,y) (x|=(1<<y))
#define ClrBit(x,y) (x&=~(1<<y))
#define ToggleBit(x,y) (x^=(1<<y))
#define FlipBit(x,y) (x^=(1<<y)) // Same as ToggleBit.
#define TestBit(x,y) (x&(1<<y))

#define _BV(bit) (1 << (bit))

//Pins
#define RELE3 PB4
#define RELE2 PB5
#define RELE1 PB6
#define RED_LIGHT PA4
#define YELLOW_LIGHT PA5

//k=1024/5*((x*sqrt(2))/(470000*2+9700))*9700
#define V180 531
#define V205 607
#define V235 693
#define V260 770
#define V100 100 //295

#define V10 15 //V100/20


#define TEMP_WARN 500

#define ADCTR 1
#define fON 1
#define fOFF 0

//debug
#define PROTOCOL_MAX_LEN 6
#define PROTOCOL_MAX_NAME_LEN 3

#ifndef byte
#define byte unsigned char
#endif

//#define DEBUG_S 1

#ifdef DEBUG_S
//ATtiny45 8000000
void ddelay_ms(int ms){
    int ms1=F_CPU/1000 - 1600;
    unsigned char bt = 0;
    int ms2 = 0;
    for(bt=0; bt<ms; bt++){
        for(ms2=0;ms2<(ms1>>2);ms2++){
            asm volatile ("mov r0, r0");
        }
    }
}

void ddelay_us(int us){
    int ms1=F_CPU/1000000 - 4;
    unsigned int bt = 0;
    int ms2 = 0;
    if (us<10) return;
    for(bt=0; bt<us; bt++){
        for(ms2=0;ms2<(ms1>>1);ms2++){
            asm volatile ("mov r0, r0");
        }
    }
}
#endif

typedef enum _protocol_data_type
{
    PROTO_UNDEF = 0,
    PROTO_UNSBYTE,
    PROTO_BYTE_HEX,
    PROTO_INT,
    PROTO_STRING
} protocol_data_type;

/*
* type: //0 - undefined, 1 - unsigned byte/char decimal, 2 - byte/char hex, 3 - int, 4 - string
*/

typedef struct __attribute__ ((packed)) __item_info{
    byte type;
    char name[PROTOCOL_MAX_NAME_LEN];
    union Data {
        byte data1;
        int data2;
        char data4[PROTOCOL_MAX_LEN];
    } data;
} item_info;

void protocol_prepare_send_buffer(item_info *buffer, byte length){
    memset(buffer, 0, sizeof(item_info) * length);
}

void *protocol_associate_data(item_info *buffer, byte index, void *data, protocol_data_type type, char *name){
    item_info *ptr_buffer = (item_info *)((char *)buffer + sizeof(item_info) * index);//sizeof(item_info) * index);
    ptr_buffer->type = (byte)type;
    strncpy(ptr_buffer->name, name, sizeof(ptr_buffer->name));
    switch(type){
        case PROTO_UNSBYTE:
        case PROTO_BYTE_HEX:
        {
            byte *buf = (byte *)data;
            ptr_buffer->data.data1 = *buf;
            return &(ptr_buffer->data.data1);
        }
        case PROTO_INT:
        {
            int *buf = (int *)data;
            ptr_buffer->data.data2 = *buf;
            return &(ptr_buffer->data.data2);
        }
        case PROTO_STRING:
        {
            strncpy((char *)&(ptr_buffer->data.data4), (char *)data, sizeof(ptr_buffer->data.data4));
            return &(ptr_buffer->data.data4);
        }
        default:{
            return NULL;
        }
    }
}

//USI
//adopted from https://github.com/JDat/AtTiny-I2C-master-slave-USI/blob/master/usi_i2c_slave.c
#define DDR_USI             DDRB
#define PORT_USI            PORTB
#define PIN_USI             PINB
#define PORT_USI_SDA        PB0
#define PORT_USI_SCL        PB2
#define PIN_USI_SDA         PINB0
#define PIN_USI_SCL         PINB2

char usi_i2c_slave_internal_address;
char usi_i2c_slave_address;
char usi_i2c_mode;

#define USI_SLAVE_REGISTER_COUNT 4

unsigned char USI_Slave_register_buffer[(sizeof(item_info) * USI_SLAVE_REGISTER_COUNT)+2] = { 0 };
unsigned char USI_walk = 0;

enum
{
    USI_SLAVE_CHECK_ADDRESS,
    USI_SLAVE_SEND_DATA,
    USI_SLAVE_SEND_DATA_ACK_WAIT,
    USI_SLAVE_SEND_DATA_ACK_CHECK,
    USI_SLAVE_RECV_DATA_WAIT,
    USI_SLAVE_RECV_DATA_ACK_SEND
} USI_I2C_Slave_State;

#define USI_SLAVE_COUNT_ACK_USISR			0b01110000 | (0x0E << USICNT0)	//Counts one clock (ACK)
#define USI_SLAVE_COUNT_BYTE_USISR			0b01110000 | (0x00 << USICNT0)	//Counts 8 clocks (BYTE)
#define USI_SLAVE_CLEAR_START_USISR			0b11110000 | (0x00 << USICNT0)  //Clears START flag
#define USI_SLAVE_SET_START_COND_USISR		0b01110000 | (0x00 << USICNT0)
#define USI_SLAVE_SET_START_COND_USICR		0b10101000
#define USI_SLAVE_STOP_DID_OCCUR_USICR		0b10111000
#define USI_SLAVE_STOP_NOT_OCCUR_USICR		0b11101000

#define USI_SET_SDA_OUTPUT()	{ DDR_USI |=  (1 << PORT_USI_SDA); }
#define USI_SET_SDA_INPUT() 	{ DDR_USI &= ~(1 << PORT_USI_SDA); }

#define USI_SET_SCL_OUTPUT()	{ DDR_USI |=  (1 << PORT_USI_SCL); }
#define USI_SET_SCL_INPUT() 	{ DDR_USI &= ~(1 << PORT_USI_SCL); }

#define USI_SET_BOTH_OUTPUT()	{ DDR_USI |= (1 << PORT_USI_SDA) | (1 << PORT_USI_SCL); }
#define USI_SET_BOTH_INPUT() 	{ DDR_USI &= ~((1 << PORT_USI_SDA) | (1 << PORT_USI_SCL)); }

void USI_I2C_Init(char address)
{
    PORT_USI &= ~(1 << PORT_USI_SCL);
    PORT_USI &= ~(1 << PORT_USI_SDA);

    usi_i2c_slave_address = address;

    USI_SET_BOTH_INPUT();
    
    USICR = (1 << USISIE) | (0 << USIOIE) | (1 << USIWM1) | (0 << USIWM0) | (1 << USICS1) | (0 << USICS0) | (0 << USICLK) | (0 << USITC);
    USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);
}

ISR(USI_STRT_vect)
{
    USI_I2C_Slave_State = USI_SLAVE_CHECK_ADDRESS;
    USI_SET_SDA_INPUT();
    while((PIN_USI & (1 << PIN_USI_SCL)) && !((PIN_USI & (1 << PIN_USI_SDA))));
    if(!(PIN_USI & (1 << PIN_USI_SDA)))
    {
        USICR = USI_SLAVE_STOP_NOT_OCCUR_USICR;
    }
    else
    {
        USICR = USI_SLAVE_STOP_DID_OCCUR_USICR;
    }
    USISR = USI_SLAVE_CLEAR_START_USISR;
}

ISR(USI_OVF_vect)
{
    switch (USI_I2C_Slave_State)
    {
        case USI_SLAVE_CHECK_ADDRESS:

        if((USIDR == 0) || ((USIDR >> 1) == usi_i2c_slave_address))
        {
            USI_walk = 0;
            if (USIDR & 0x01)
            {
                USI_I2C_Slave_State = USI_SLAVE_SEND_DATA;
            }
            else
            {
                USI_I2C_Slave_State = USI_SLAVE_RECV_DATA_WAIT;
            }

            USIDR = 0;
            USI_SET_SDA_OUTPUT();
            USISR = USI_SLAVE_COUNT_ACK_USISR;
        }
        else
        {
            USICR = USI_SLAVE_SET_START_COND_USICR;
            USISR = USI_SLAVE_SET_START_COND_USISR;
        }
        break;

        case USI_SLAVE_SEND_DATA_ACK_WAIT:

        PORT_USI &= ~(1 << PORT_USI_SDA);
        USI_I2C_Slave_State = USI_SLAVE_SEND_DATA_ACK_CHECK;
        USI_SET_SDA_INPUT();
        USISR = USI_SLAVE_COUNT_ACK_USISR;
        USIDR = 0;
        break;

        case USI_SLAVE_SEND_DATA_ACK_CHECK:
        
        if(USIDR)
        {
            USICR = USI_SLAVE_SET_START_COND_USICR;
            USISR = USI_SLAVE_SET_START_COND_USISR;
            return;
        }

        case USI_SLAVE_SEND_DATA:

        if(USI_walk <= ((sizeof(item_info) * USI_SLAVE_REGISTER_COUNT)+2))
        {
            USIDR = USI_Slave_register_buffer[USI_walk];
        }
        else
        {
            USIDR = 0;
        }
        USI_walk++;
        USI_I2C_Slave_State = USI_SLAVE_SEND_DATA_ACK_WAIT;

        USI_SET_SDA_OUTPUT();
        PORT_USI |= (1 << PORT_USI_SDA);
        USISR = USI_SLAVE_COUNT_BYTE_USISR;
        break;

        case USI_SLAVE_RECV_DATA_WAIT:

        USI_I2C_Slave_State = USI_SLAVE_RECV_DATA_ACK_SEND;

        USI_SET_SDA_INPUT();
        USISR = USI_SLAVE_COUNT_BYTE_USISR;
        break;

        case USI_SLAVE_RECV_DATA_ACK_SEND:
        USI_I2C_Slave_State = USI_SLAVE_RECV_DATA_WAIT;
        
        USI_Slave_register_buffer[USI_walk] = USIDR;
        
        USIDR = 0;
        USI_SET_SDA_OUTPUT();
        USISR = USI_SLAVE_COUNT_ACK_USISR;
        break;
    }
}
//USI

void initial_registers_set(){
    PORTB &= ~(_BV(RELE3) | _BV(RELE2) | _BV(RELE1));
    DDRB |= _BV(RELE3) | _BV(RELE2) | _BV(RELE1);
    PORTA &= ~(_BV(RED_LIGHT) | _BV(YELLOW_LIGHT));
    DDRA |= _BV(RELE3) | _BV(RELE2) | _BV(RELE1);
    ACSR |= _BV(ACD); //disable analog comparator
}

uint16_t readADC(uint8_t adc_num){
    ADCSR = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);
    uint8_t trys = ADCTR;
    uint16_t value = 0;
    switch(adc_num){
        case 0:{
            ADMUX = 0;
            break;
        }
        case 1:{
            ADMUX = _BV(MUX0);
            break;
        }
        case 2:{
            ADMUX = _BV(MUX1);
            break;
        }
    }
    //_delay_us(50);
    while(trys){
        ADCSR |= _BV(ADSC);
        while ((ADCSR & _BV(ADIF)) == 0);
        ADCSR |= _BV(ADIF);
        value += ADCW;
        trys--;
    }
    ADCSR &= ~(1<<ADEN);
    return value/ADCTR;
}

void redLight(uint8_t sw){
    if (sw) SetBit(PORTA, RED_LIGHT);
    else ClrBit(PORTA, RED_LIGHT);
}

void yellowLight(uint8_t sw){
    if (sw) SetBit(PORTA, YELLOW_LIGHT);
    else ClrBit(PORTA, YELLOW_LIGHT);
}

uint8_t checkRed(){
    return PORTA & _BV(RED_LIGHT);
}

uint8_t checkYellow(){
    return PORTA & _BV(YELLOW_LIGHT);
}

uint16_t adc_temp = 0, adc_temp2 = 0;
uint8_t flag = 0;

//For debug
int TmpLineIn = 0;
int TmpTemper = 0;
int Tmpcounter = 0;
uint8_t beg = 0;
uint16_t waveMax = 0;
uint16_t tmpMax = 0;
uint16_t prevVal = 0;

#define USI_addr 0x15
#define UNDEF 15000

uint16_t getWaveMax(uint16_t in_data){
    if (!beg){
        if (in_data>V100 && !waveMax){
            waveMax = in_data;
            } else if (in_data>V100 && prevVal<=in_data){
            if (waveMax < in_data) waveMax = in_data;
            beg = 1;
            } else {
            waveMax = 0;
        }
        } else {
        if (waveMax<in_data){
            waveMax = in_data;
            } else {
            if (in_data<=V100){
                tmpMax = waveMax;
                waveMax = 0;
                beg = 0;
                prevVal = 0;
                return tmpMax;
            }
        }
    }
    prevVal = in_data;
    return UNDEF;
}

int cond = 0; //0 < 180, 1 < 205, 2 < 235, 3 < 260, 4 > 260

uint16_t getLimit(uint16_t val){
    if (val == V180){
        if (cond == 1) return (V180 - V10);
    } else if (val == V205) {
        if (cond == 2) return (V205 - V10);
    } else if (val == V235) {
        if (cond == 3) return (V235 - V10);
    } else if (val == V260) {
        if (cond == 4) return (V260 - V10);
    }
    return val;
}

int main(void)
{
    initial_registers_set();
    USI_I2C_Init(USI_addr);
    protocol_prepare_send_buffer((item_info *)(USI_Slave_register_buffer+1), USI_SLAVE_REGISTER_COUNT);
    int *Lin = protocol_associate_data((item_info *)(USI_Slave_register_buffer+1), 0, &TmpLineIn, PROTO_INT, "LI");
    int *Tpr = protocol_associate_data((item_info *)(USI_Slave_register_buffer+1), 1, &TmpTemper, PROTO_INT, "TP");
    int *Cnt = protocol_associate_data((item_info *)(USI_Slave_register_buffer+1), 2, &Tmpcounter, PROTO_INT, "TM");
	int *cnd = protocol_associate_data((item_info *)(USI_Slave_register_buffer+1), 3, &cond, PROTO_INT, "CN");
	
    USI_Slave_register_buffer[0] = USI_SLAVE_REGISTER_COUNT;
    *Cnt = 0;
    sei();
    
    cond = 0;
	*cnd = cond;
    redLight(fON);

//#ifdef DEBUG_S
//    ddelay_ms(500);
//#else
//    _delay_ms(500);
//#endif
	
	adc_temp = 0;
	adc_temp2 = 0;
	*Lin = adc_temp;
	*Tpr = adc_temp2;
    //WorkMode
	uint8_t chg = 0;
    while (1)
    {
		chg = 0;
        (*Cnt)++;
        if (*Cnt>30000) {
            *Cnt = 0;
        }
        flag = PORTB;
        
        adc_temp = getWaveMax(readADC(1)); //LineIn
        if (adc_temp!=UNDEF) {
            *Lin = adc_temp;
            if ((adc_temp< getLimit(V180)) || (adc_temp>getLimit(V260))){
				if (checkYellow()) yellowLight(fOFF);
                redLight(fON);
                ClrBit(flag, RELE2);
                ClrBit(flag, RELE3);
                SetBit(flag, RELE1);
                PORTB = flag; //rele1 - V1, rele2 - V2, rele3 - out, no stab
                if (adc_temp < getLimit(V180)) {
					cond = 0;
					*cnd = cond;
				}
                if (adc_temp > getLimit(V260)) {
					cond = 4;
					*cnd = cond;
				}
//				#ifdef DEBUG_S
//				ddelay_ms(100);
//				#else
//				_delay_ms(100);
//				#endif
                continue;
            }
        }
        adc_temp2 = readADC(2); //Temp
        *Tpr = adc_temp2;
        if (adc_temp2>TEMP_WARN){
            ClrBit(flag, RELE2);
            ClrBit(flag, RELE3);
            SetBit(flag, RELE1);
            PORTB = flag; //rele1 - V1, rele2 - V2, rele3 - out, no stab
            if (checkYellow()) yellowLight(fOFF);
            redLight(fON);
//			#ifdef DEBUG_S
//			ddelay_ms(100);
//			#else
//			_delay_ms(100);
//			#endif
            continue;
        }
        if (adc_temp==UNDEF){
            continue;
        }
        if (checkRed()) redLight(fOFF);
        if (adc_temp<getLimit(V205)){
            cond = 1;
			*cnd = cond;
            if (!(flag & _BV(RELE1))){
                SetBit(flag, RELE1);
            }
            if (!(flag & _BV(RELE2))){
                SetBit(flag, RELE2);
            }
            if (!(flag & _BV(RELE3))){
                SetBit(flag, RELE3);
            }
            PORTB = flag; //rele1 - V1, rele2 - V3, rele3 - out
            yellowLight(fON);
//			#ifdef DEBUG_S
//			ddelay_ms(100);
//			#else
//			_delay_ms(100);
//			#endif
            continue;
        }
        if (adc_temp>getLimit(V235)){
            cond = 3;
			*cnd = cond;
            if (flag & _BV(RELE1)){
                ClrBit(flag, RELE1);
            }
            if (flag & _BV(RELE2)){
                ClrBit(flag, RELE2);
            }
            if (!(flag & _BV(RELE3))){
                SetBit(flag, RELE3);
            }
            PORTB = flag; //rele1 - V3, rele2 - V2, rele3 - out
            yellowLight(fON);
//			#ifdef DEBUG_S
//			ddelay_ms(100);
//			#else
//			_delay_ms(100);
//			#endif
            continue;
        }
        if (!(flag & _BV(RELE1))){
            SetBit(flag, RELE1);
			chg = 1;
        }
        if (flag & _BV(RELE2)){
            ClrBit(flag, RELE2);
			chg = 1;
        }
        if (!(flag & _BV(RELE3))){
            SetBit(flag, RELE3);
			chg = 1;
        }
        PORTB = flag; //rele1 - V1, rele2 - V2, rele3 - out, no stab
        if (checkYellow()) yellowLight(fOFF);
        cond = 2;
		*cnd = cond;
//		if (chg == 1){
//			#ifdef DEBUG_S
//			ddelay_ms(100);
//			#else
//			_delay_ms(100);
//			#endif
//		}
		
    }
}

