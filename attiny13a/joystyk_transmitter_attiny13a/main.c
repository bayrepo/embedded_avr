/*
 * radio_send_atitny85.c
 *
 * Created: 03.12.2019 1:47:22
 * Author : bayre
 */ 

#include <avr/io.h>
#include "ManchesterRF.h"
#include <avr/delay.h>
#include <string.h>
#include <avr/interrupt.h>

#define MAGIC_NUMBER 0xFFEE
#define SEND_SIZE 4

#define Rx_coord sendData[SEND_SIZE-3]
#define Ry_coord sendData[SEND_SIZE-2]
#define TransmitKey sendData[SEND_SIZE-1]

unsigned int sendData[SEND_SIZE] = { MAGIC_NUMBER, 0, 0, 0 };
unsigned char Rkey = 0;

int main(void)
{
	ACSR=0x80;
	ADCSRA = _BV(ADEN) | _BV(ADPS1);
	
	sei();
	ManchesterRF(MAN_1200);
	ManchesterRF_TXInit(2, _BV(PINB2));
    while (1) 
    {
		//Read X
		ADMUX = _BV(MUX1);
		_delay_us(50);
		ADCSRA |= _BV(ADSC);
		while ((ADCSRA & _BV(ADIF)) == 0);
		ADCSRA |= _BV(ADIF);
		Rx_coord = ADCW;
		//Read Y
		ADMUX = _BV(MUX1) | _BV(MUX0);
		_delay_us(50);
		ADCSRA |= _BV(ADSC);
		while ((ADCSRA & _BV(ADIF)) == 0);
		ADCSRA |= _BV(ADIF);
		Ry_coord = ADCW;
		//Check button
		if ((PINB & _BV(PINB1)) && !Rkey){
			Rkey = 1;
		}
		if (!(PINB & _BV(PINB1)) && Rkey){
			Rkey = 0;
			TransmitKey = 1;
		}
		
		ManchesterRF_transmitArray(sizeof(unsigned int) * SEND_SIZE, (uint8_t *)(&sendData));

		TransmitKey = 0;
		_delay_ms(10);

		
    }
}

