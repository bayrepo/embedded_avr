/*
* stepper_first_experiment.c
*
* Created: 01.06.2020 22:48:19
* Author : bayrepo.info@gmail.com
*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>

//#define DBG 1

#define SetBit(x,y) (x|=(1<<y))
#define ClrBit(x,y) (x&=~(1<<y))
#define ToggleBit(x,y) (x^=(1<<y))
#define FlipBit(x,y) (x^=(1<<y)) // Same as ToggleBit.
#define TestBit(x,y) (x&(1<<y))

#define _BV(bit) (1 << (bit))

#define MAX_STEPS 4 //8
uint8_t states[MAX_STEPS] = {9, 12, 6, 3}; //{ 8, 12, 4, 6, 2, 3, 1, 9 }; //1000, 1100, 0100, 0110, 0010, 0011, 0001, 1001
uint8_t direction = 0;

#define BUTTON_STOP_MOTOR PD2
#define MOTOR_STOP_LIGHT PC5

#define BUTTON_LIGHT PD3
#define LIGHT_ON PC4

#define MOTO_FLAG _BV(0)
#define LIGHT_M_FLAG _BV(1)
uint8_t button_flags = 0;
uint8_t prev_flag = 0;

int voltage = 0;

unsigned int potentiometer = 0;

void doMotorLight(){
    if (prev_flag != button_flags){
        if (button_flags & MOTO_FLAG){
            SetBit(PORTC, MOTOR_STOP_LIGHT);
            } else {
            ClrBit(PORTC, MOTOR_STOP_LIGHT);
        }
        if (button_flags & LIGHT_M_FLAG){
            SetBit(PORTC, LIGHT_ON);
            } else {
            ClrBit(PORTC, LIGHT_ON);
        }
        prev_flag = button_flags;
    }
}

ISR(INT0_vect){
    button_flags = button_flags ^ 1;
    doMotorLight();
}

ISR(INT1_vect){
    button_flags = button_flags ^ 2;
    doMotorLight();
}

ISR (ADC_vect){
    potentiometer = ADCW;
    ADCSR |= (1<<ADSC);
}

#define MIN_R_100 1
#define MAX_R_100 6

#define MIN_POT ((1024 * MIN_R_100) / 7)
#define MAX_POT ((1024 * MAX_R_100) / 7)

#define N_speed 8

int speeds_array[N_speed+1] = { -1, 500, 480, 430, 400, 380, 330, 300, 250};

#define Stop_center (MIN_POT + (MAX_POT - MIN_POT)/2)
#define Stop_min (Stop_center - Stop_center / 5)
#define Stop_max (Stop_center + Stop_center / 5)

#define Step_val2 ((Stop_min - MIN_POT) / N_speed)
#define Step_val1 ((MAX_POT - Stop_max) / N_speed)

int8_t getSpeed(unsigned int val){
    if (val <= Stop_min) {
        int8_t step = (Stop_min - val) / Step_val2 + 1;
        if (step > N_speed) step = N_speed;
        return -step;
        } else if (val >= Stop_max) {
        int8_t step = (val - Stop_max) / Step_val1 + 1;
        if (step > N_speed) step = N_speed;
        return step;
    } else return 0;
    
}

int8_t cur_index = 0;
#define _PIN0 PB4
#define _PIN1 PB3
#define _PIN2 PB2
#define _PIN3 PB1

void _local_delay_ms(uint16_t count) {
    while(count--) {
        _delay_ms(1);
    }
}

void _local_delay_us(uint16_t count) {
    while(count--) {
        _delay_us(10);
    }
}

void setDrivePins(uint8_t value){
    uint8_t tmp = PORTB;
    if (TestBit(value, 0)){
        SetBit(tmp, _PIN0);
    } else {
        ClrBit(tmp, _PIN0);
    }
    if (TestBit(value, 1)){
        SetBit(tmp, _PIN1);
        } else {
        ClrBit(tmp, _PIN1);
    }
    if (TestBit(value, 2)){
        SetBit(tmp, _PIN2);
        } else {
        ClrBit(tmp, _PIN2);
    }
    if (TestBit(value, 3)){
        SetBit(tmp, _PIN3);
        } else {
        ClrBit(tmp, _PIN3);
    }
    PORTB = tmp;
}

void setSpeed(unsigned int val){
    int8_t spd = getSpeed(val);
    uint8_t direct = 0;
    if (spd > 0) direct = 1;
    else if (spd < 0) {
        direct = 2;
        spd = -spd;
    }        
    if (spd > N_speed) spd = 0;
    int value = speeds_array[spd];
    if (value >= 0 && (button_flags & MOTO_FLAG)){
        setDrivePins(states[cur_index]);
        if (direct == 1) {
            cur_index++;
            if (cur_index>=MAX_STEPS) cur_index = 0;
        } else {
            cur_index--;
            if (cur_index<0) cur_index = MAX_STEPS - 1;
        }
        _local_delay_us(value);
    } else {
        _delay_ms(10);
		setDrivePins(0);
    }
}

#ifdef DBG

uint8_t i = 0;

void USART_Init( unsigned int ubrr)
{
    UBRRH = (unsigned char)(ubrr>>8);
    UBRRL = (unsigned char)ubrr;
    UCSRB = (1<<TXEN);
    UCSRA &= ~(1<<U2X);
    /* Устанавливаем формат данных 8 бит данных, 2 стоп бита */
    UCSRC = (1<<URSEL)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
}

int writeSerial(char* str)
{
    for(i=0;i<strlen(str); i++)
    {
        while(!(UCSRA&(1<<UDRE))){}; // wait ready of port
        UDR = str[i];
    }
    return 0;
}

int writeSerialInt(int data)
{
    char str[10];
    itoa(data, str, 10);
    
    for(i=0;i<strlen(str); i++)
    {
        while(!(UCSRA&(1<<UDRE))){}; // wait ready of port
        UDR = str[i];
    }
    while(!(UCSRA&(1<<UDRE))){}; // wait ready of port
    UDR = '\n';
    return 0;
}

int writeSerialSpeed(int data){
    int8_t sp = getSpeed(data);
    if (sp < 0) {
        writeSerial("Up ");
        writeSerialInt(-sp);
        } else if (sp > 0) {
        writeSerial("Down ");
        writeSerialInt(sp);
        } else {
        writeSerial("Stop\n");
    }
    return 0;
}
#endif

int main(void)
{
    //включаем вход для чтения кнопки принудительного останова моторов
    ClrBit(DDRD, BUTTON_STOP_MOTOR);
    ClrBit(PORTD, BUTTON_STOP_MOTOR);
    
    //включаем вход для чтения кнопки освещения
    ClrBit(DDRD, BUTTON_LIGHT);
    ClrBit(PORTD, BUTTON_LIGHT);
    //включаем выход, для управления диодом MOTOR STOP
    SetBit(DDRC, MOTOR_STOP_LIGHT);
    //включаем выход, для управления диодом LIGHT
    SetBit(DDRC, LIGHT_ON);
    ClrBit(PORTC, MOTOR_STOP_LIGHT);
    ClrBit(PORTC, LIGHT_ON);
    prev_flag = button_flags;
    
    //Включаем ножки для моторов как выходы
    SetBit(DDRB, _PIN0);
    SetBit(DDRB, _PIN1);
    SetBit(DDRB, _PIN2);
    SetBit(DDRB, _PIN3);
    
    ClrBit(PORTB, _PIN0);
    ClrBit(PORTB, _PIN1);
    ClrBit(PORTB, _PIN2);
    ClrBit(PORTB, _PIN3);
    
    #ifdef DBG
    USART_Init(51);
    #endif
    //Садим кнопку на прерывание
    SetBit(GICR, INT0);
    SetBit(GICR, INT1);
    MCUCR |= _BV(ISC01) | _BV(ISC00) | _BV(ISC11) | _BV(ISC10);
    
    //Настраиваем АЦП 0
    ADMUX = 0;
    ADCSRA |= (1<<ADEN) | (1<<ADSC) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    sei();
    while (1)
    {
        #if DBG
        writeSerialSpeed(potentiometer);
        #endif
        setSpeed(potentiometer);
    }
}

