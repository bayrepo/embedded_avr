/*
* radio_recv_attiny45_85.c
*
* Created: 04.12.2019 22:54:17
* Author : bayre
*/

#include <avr/io.h>
#include "ManchesterRF.h"
#include <avr/delay.h>
#include <string.h>
#include <avr/interrupt.h>

#define MAGIC_NUMBER 0xFFEE
#define MESSAGE_SIZE 4
int X_coord = 0;
int Y_coord = 0;
unsigned char button = 0;

unsigned char X_p = 0;
unsigned char Y_p = 0;

uint16_t *tmpData;
unsigned char *tmpBuffer;
unsigned char rxSize;
unsigned char bufferData[sizeof(uint16_t) * MESSAGE_SIZE];
unsigned char last_index;
unsigned char magic_found = 0;

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

#define HZ50_CKL 156//312
#define HZ50_CENTER 12//23
#define HZ50_LEFT 17//33
#define HZ50_RIGHT 7//13
#define ADC_TOCOUNTER 100 //1023/10
static int motor_counter = 0;
static int xTranslate = 0;
static int yTranslate = 0; 
unsigned char xMoto_level = 0;
unsigned char yMoto_level = 0;

void f_interruptor(){
    if (X_coord>=500 && X_coord<=520){
        xTranslate = 510 / ADC_TOCOUNTER;
    } else {
        xTranslate = X_coord / ADC_TOCOUNTER;
    }
    if (Y_coord>=486 && Y_coord<=506){
        yTranslate = 500 / ADC_TOCOUNTER;
        } else {
        yTranslate = Y_coord / ADC_TOCOUNTER;
    }
    xTranslate = HZ50_LEFT - xTranslate;
    yTranslate = HZ50_LEFT - yTranslate;
    if (button == 1){
        PORTB ^= (1<<PB3);
        button = 0;
    }
    motor_counter++;
    if (motor_counter>=HZ50_CKL){
        motor_counter = 0;
    }
    if (!motor_counter){
        PORTB |= ((1<<PB0) | (1<<PB2));
        xMoto_level = 1;
        yMoto_level = 1;
    }
    if (motor_counter==xTranslate && xMoto_level==1){
        PORTB &= ~(1<<PB2);
        xMoto_level = 0;
    }
    if (motor_counter==yTranslate && yMoto_level==1){
        PORTB &= ~(1<<PB0);
        yMoto_level = 0;
    }
}

int main(void){
	DDRB |= (1 << PB1) | (1 << PB0) | (1<<PB2) | (1<<PB3);
	PORTB &= ~((1<<PB1) | (1<<PB0)|(1<<PB2)|(1<<PB3));
	tmpData = (uint16_t *)bufferData;
	last_index = 0;
	int index = 0;
    X_coord = 510;
    Y_coord = 496;
    ManchesterRF_setFunction(&f_interruptor);
	ManchesterRF(MAN_1200);
	ManchesterRF_RXInit(2, _BV(PB4));
	ManchesterRF_beginReceive();
	sei();
#ifdef DEBUG_S
  //  ddelay_ms(10);
#else
    _delay_ms(10);
#endif // DEBUG_S
	while (1)
	{
		int av = ManchesterRF_available();
		if(av){
			if(ManchesterRF_receiveArray((uint8_t *)&rxSize, (uint8_t **)&tmpBuffer)){
				if (rxSize>0){
					rpt:
					if(magic_found==0){
						while(1){
							if (magic_found == 0) {
								if (last_index>0){
									if (tmpBuffer[0]==0xFF){
										bufferData[last_index]=tmpBuffer[index];
										last_index++;
										magic_found = 1;
										} else {
										last_index = 0;
									}
								}
								if (tmpBuffer[index]==0xEE && last_index == 0){
									if (index==(rxSize-1)){
										bufferData[last_index]=tmpBuffer[index];
										last_index++;
										index = 0;
										break;
										} else if (tmpBuffer[index+1]==0xFF){
										bufferData[last_index] = tmpBuffer[index];
										bufferData[last_index+1] = tmpBuffer[index+1];
										last_index+=2;
										index++;
										magic_found = 1;
									}
								}
								} else {
								index=0;
								break;
							}
							index++;
							if (index>=rxSize || magic_found == 1){
								if (magic_found == 1 && index < rxSize) goto rpt;
								if (index>=rxSize) index = 0;
								break;
							}
						}
						} else {
						while(1){
							if(last_index>=(sizeof(uint16_t) * MESSAGE_SIZE)){
								if ((tmpData[1]>=0 && tmpData[1]<=1023) && (tmpData[2]>=0 && tmpData[2]<=1023) && (tmpData[3]>=0 && tmpData[3]<=1)){
									X_coord = tmpData[1];
									Y_coord = tmpData[2];
									button = (unsigned char)tmpData[3];
								}
								magic_found = 0;
								last_index = 0;
								goto rpt;
							}
							bufferData[last_index]=tmpBuffer[index];
							last_index++;
							index++;
							if (index >= rxSize){
								if(last_index>=(sizeof(uint16_t) * MESSAGE_SIZE)){
									if ((tmpData[1]>=0 && tmpData[1]<=1023) && (tmpData[2]>=0 && tmpData[2]<=1023) && (tmpData[3]>=0 && tmpData[3]<=1)){
										X_coord = tmpData[1];
										Y_coord = tmpData[2];
										button = (unsigned char)tmpData[3];
									}
									magic_found = 0;
									last_index = 0;
								}
								index = 0;
								break;
							}
						}
					}
				}
			}
		}
		#ifdef DEBUG_S
		ddelay_ms(5);
		#else
		_delay_ms(5);
		#endif // DEBUG_S
	}
	return 0;
}

