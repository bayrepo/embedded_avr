/*
* joystik_driver.c
*
* Created: 03.10.2020 15:44:08
* Author : bayre
*/
#define F_CPU 9600000


#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>

#define _BV(bit) (1 << (bit))
#define SetBit(x,y) (x|=(1<<y))
#define ClrBit(x,y) (x&=~(1<<y))
#define ToggleBit(x,y) (x^=(1<<y))
#define FlipBit(x,y) (x^=(1<<y)) // Same as ToggleBit.
#define TestBit(x,y) (x&(1<<y))

uint16_t Rx_coord = 510;
uint16_t Ry_coord = 496;

unsigned char Rkey = 0;
volatile unsigned char TransmitKey = 0;

uint16_t cnt_low = 0;
uint16_t cnt_high = 0;

#define DRV_X 1
#define DRV_Y 2
#define BUTON PINB2
#define SETBX 1
#define SETBY 2
#define STOPF 3

#define MAGICK_NUMBER 100 //Number of ticks in 1ms
#define DOWNV 60
#define MAXIV 240
#define ADCTR 4

#define ADC_C_LOW 500
#define ADC_C_HIGH 524
#define ANGLE_MAGIC_A 256
#define ANGLE_MAGIC_B 5
#define ADC_MID 512

uint16_t readADC(uint8_t adc_num){
	ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);
	uint8_t trys = ADCTR;
	uint16_t value = 0;
	switch(adc_num){
		case 1:{
			ADMUX = _BV(MUX0);
			break;
		}
		case 2:{
			ADMUX = _BV(MUX1);
			break;
		}
		case 3:{
			ADMUX = _BV(MUX1) | _BV(MUX0);
			break;
		}
	}
	_delay_us(50);
	while(trys){
		ADCSRA |= _BV(ADSC);
		while ((ADCSRA & _BV(ADIF)) == 0);
		ADCSRA |= _BV(ADIF);
		value += ADCW;
		trys--;
	}
	ADCSRA &= ~(1<<ADEN);
	return value/ADCTR;
}

uint8_t frontChecker = 0;
uint8_t cycleChecker = 0;
uint8_t globalFlags = 0;
uint8_t tickChecker = 0;

uint8_t xTrn = MAGICK_NUMBER + MAGICK_NUMBER/2;
uint8_t yTrn = MAGICK_NUMBER + MAGICK_NUMBER/2;

ISR(TIM0_COMPA_vect){
	
	if (frontChecker == xTrn) {
		PORTB &= ~(DRV_X);
		globalFlags |= SETBX;
	}
	if (frontChecker == yTrn) {
		PORTB &= ~(DRV_Y);
		globalFlags |= SETBY;
	}
	
	frontChecker++;
	
	if (tickChecker==MAGICK_NUMBER){
		cycleChecker++;
		if (cycleChecker >= 20) {
			cycleChecker = 0;
			PORTB |= (DRV_X | DRV_Y);
			globalFlags &= ~(SETBX&SETBY);
			frontChecker = 0;
		}
		tickChecker = 0;
	} else {
		tickChecker++;
	}

}

void getRotation(uint16_t coord, uint8_t *step, uint8_t *direct){
	uint16_t tmp = 0;
	if (coord < ADC_C_LOW){
		*direct = 1;
		tmp = ((ADC_MID-coord)*ANGLE_MAGIC_B)/ANGLE_MAGIC_A;
		*step = (uint8_t)tmp;
		} else if (coord > ADC_C_HIGH) {
		*direct = 2;
		tmp = ((coord - ADC_MID)*ANGLE_MAGIC_B)/ANGLE_MAGIC_A;
		*step = (uint8_t)tmp;
		} else {
		*direct = 0;
		*step = 0;
	}
	return;
}

uint8_t xTrn_tmp = 0;
uint8_t yTrn_tmp = 0;
uint8_t rot_x = 0;
uint8_t rot_y = 0;

int main(void)
{
	ACSR = _BV(ACD);
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS00);
	TCNT0 = 0;
	TIMSK0 = _BV(OCIE0A);
	OCR0A = 97;
	
	DDRB = DRV_X | DRV_Y;
	PORTB = 0;
	
	sei();
	while (1)
	{
		//Read X
		Rx_coord = readADC(2);
		//Read Y
		Ry_coord = readADC(3);
		//Check button
		if ((PINB & _BV(BUTON)) && !Rkey && !TransmitKey){
			Rkey = 1;
		}
		if (!(PINB & _BV(BUTON)) && Rkey){
			Rkey = 0;
			cnt_low = 0;
			cnt_high = 0;
			TransmitKey = 1;
		}
		
		getRotation(Rx_coord, &xTrn_tmp, &rot_x);
		getRotation(Ry_coord, &yTrn_tmp, &rot_y);
		
		if (!TestBit(globalFlags, STOPF)) {
			if (rot_x==1){
				if ((xTrn+xTrn_tmp)<MAXIV) xTrn = xTrn+xTrn_tmp;
				} else if (rot_x==2){
				if ((xTrn-xTrn_tmp)>DOWNV) xTrn = xTrn-xTrn_tmp;
			}
			if (rot_y==1){
				if ((yTrn+yTrn_tmp)<=MAXIV) yTrn = yTrn+yTrn_tmp;
				} else if (rot_y==2){
				if ((yTrn-yTrn_tmp)>=DOWNV) yTrn = yTrn-yTrn_tmp;
			}
		}
		
		if (TransmitKey){
			if (TestBit(globalFlags, STOPF) || (!TestBit(globalFlags, STOPF) && (cnt_high>=5))){
				ToggleBit(globalFlags, STOPF);
				TransmitKey = 0;
			}
		}

		_delay_ms(10);

		
	}
}

