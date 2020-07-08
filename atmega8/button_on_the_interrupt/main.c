/*
* stepper_first_experiment.c
*
* Created: 01.06.2020 22:48:19
* Author : bayre
*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#define SetBit(x,y) (x|=(1<<y))
#define ClrBit(x,y) (x&=~(1<<y))
#define ToggleBit(x,y) (x^=(1<<y))
#define FlipBit(x,y) (x^=(1<<y)) // Same as ToggleBit.
#define TestBit(x,y) (x&(1<<y))

#define _BV(bit) (1 << (bit))

#define BUTTON_STOP_MOTOR PD2
#define MOTOR_STOP_LIGHT PC5

#define MOTO_FLAG _BV(0)
uint8_t button_flags = 0;

ISR(INT0_vect){
    button_flags = button_flags ^ 1;
}

uint8_t prev_flag = 0;

void doMotorLight(){
    if (prev_flag != button_flags){
        if (button_flags & MOTO_FLAG){
            SetBit(PORTC, MOTOR_STOP_LIGHT);
            } else {
            ClrBit(PORTC, MOTOR_STOP_LIGHT);
        }
        prev_flag = button_flags;
    }
}

int main(void)
{
    //включаем вход для чтения кнопки PD2
    ClrBit(DDRD, BUTTON_STOP_MOTOR);
    ClrBit(PORTD, BUTTON_STOP_MOTOR);
    //включаем выход, для управления диодом PC5
    SetBit(DDRC, MOTOR_STOP_LIGHT);
    PORTC = 0;
    prev_flag = button_flags;
    //Садим кнопку на прерывание
    SetBit(GICR, INT0);
    //Конфигурируем прерывание на переход 0->1
    MCUCR |= _BV(ISC01) | _BV(ISC00);
    sei();
    while (1)
    {
        doMotorLight();
        _delay_ms(20);
    }
}

