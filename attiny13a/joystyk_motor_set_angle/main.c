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
unsigned char TransmitKey = 0;

#define DRV_X PORTB0
#define DRV_Y PORTB1
#define BUTON PORTB2
#define FSTFL 0
#define SETBX 1
#define SETBY 2
#define STOPF 3

#define MAGICK_NUMBER 38 //Number of ticks in 1ms
#define ADCTR 8
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

ISR(TIM0_OVF_vect){
    if (TestBit(globalFlags, FSTFL)){
        PORTB |= (_BV(DRV_X) | _BV(DRV_Y));
        ClrBit(globalFlags, FSTFL);
    }
    
    if ((frontChecker >= xTrn) && (!TestBit(globalFlags, SETBX))) {
        ClrBit(PORTB, DRV_X);
        SetBit(globalFlags, SETBX);
    }
    if ((frontChecker >= yTrn) && (!TestBit(globalFlags, SETBY))) {
        ClrBit(PORTB, DRV_Y);
        SetBit(globalFlags, SETBY);
    }
    
    frontChecker++;
    
    
    if (tickChecker>=MAGICK_NUMBER){
        tickChecker = 0;
        cycleChecker++;
        } else {
        tickChecker++;
    }
    
    if (cycleChecker >= 20) {
        cycleChecker = 0;
        PORTB |= (_BV(DRV_X) | _BV(DRV_Y));
        ClrBit(globalFlags, SETBX);
        ClrBit(globalFlags, SETBY);
        frontChecker = 0;
        tickChecker = 0;
    }
}

#define OLDTR 12
uint8_t old_tr = 0; 
uint8_t tr_mem_x[OLDTR] = {MAGICK_NUMBER + MAGICK_NUMBER/2};
uint8_t tr_mem_y[OLDTR] = {MAGICK_NUMBER + MAGICK_NUMBER/2};
uint8_t tr_ind = 0;
uint16_t tr_sum_x = 0;
uint16_t tr_sum_y = 0;
    
void addelem(uint8_t *x, uint8_t *y){
    tr_sum_x = 0;
    tr_sum_y = 0;
    if (old_tr>=(OLDTR-1)){
        for(tr_ind=0; tr_ind <= (OLDTR - 2); tr_ind++){
            tr_mem_x[tr_ind] = tr_mem_x[tr_ind+1];
            tr_mem_y[tr_ind] = tr_mem_y[tr_ind+1];
        }
    }
    tr_mem_x[old_tr] = *x;
    tr_mem_y[old_tr] = *y;
    
    for(tr_ind=0; tr_ind<=old_tr; tr_ind++){
        tr_sum_x += tr_mem_x[tr_ind];
        tr_sum_y += tr_mem_y[tr_ind];
    }
    
    if (old_tr<(OLDTR-1)){
        old_tr++;
    }
    
    *x = (uint8_t)(tr_sum_x/OLDTR);
    *y = (uint8_t)(tr_sum_y/OLDTR);
}

int main(void)
{
    uint8_t xTrn_tmp = 0;
    uint8_t yTrn_tmp = 0;
    ACSR = _BV(ACD);
    TCCR0A = 0;
    TCCR0B = _BV(CS00);
    TCNT0 = 0;
    TIMSK0 = _BV(TOIE0);
    
    DDRB = _BV(DRV_X) | _BV(DRV_Y);
    PORTB = 0;
    globalFlags |= _BV(FSTFL);
    
    sei();
    while (1)
    {
        //Read X
        Rx_coord = readADC(2);
        //Read Y
        Ry_coord = readADC(3);
        //Check button
        if ((PINB & _BV(PINB1)) && !Rkey){
            Rkey = 1;
        }
        if (!(PINB & _BV(PINB1)) && Rkey){
            Rkey = 0;
            TransmitKey = 1;
        }
        
        xTrn_tmp = Rx_coord * MAGICK_NUMBER / 1024;
        yTrn_tmp = Ry_coord * MAGICK_NUMBER / 1024;
        
        if (!TestBit(globalFlags, STOPF)) {
            addelem(&xTrn_tmp, &yTrn_tmp);
            xTrn = xTrn_tmp + MAGICK_NUMBER;
            yTrn = yTrn_tmp + MAGICK_NUMBER;
            
        }
        
        if (TransmitKey){
            ToggleBit(globalFlags, STOPF);
            TransmitKey = 0;
        }

        _delay_ms(10);

        
    }
}

