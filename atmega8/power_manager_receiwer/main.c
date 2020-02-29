/*
* receiver_power.c
*
* Created: 06.02.2020 23:27:20
* Author : bayre
*/

#include <avr/io.h>

#include <stdio.h>
#include <stdint.h>

#include "options.h"
#include "ManchesterRF.h"

#include <avr/delay.h>
#include <string.h>
#include <avr/interrupt.h>

uint8_t rnd_cnt = 0;

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

#define NO_COMMAND 0
#define DISABLE_TIMER 1 //выключить счет, работать все врем¤ (-)
#define RESET_INTERVAL 2 //сбросить счетчик интервала, т.е начать счет сначала (0)
#define SET_INTERVAL10 3 //установить интервал 10 минут, без сброса счетчика (1)
#define SET_INTERVAL20 4 //установить интервал 20 минут, без сброса счетчика (2)
#define SET_INTERVAL30 5 //установить интервал 30 минут, без сброса счетчика (3)
#define SET_INTERVAL40 6 //установить интервал 40 минут, без сброса счетчика (4)
#define SET_INTERVAL50 7 //установить интервал 50 минут, без сброса счетчика (5)
#define SET_INTERVAL60 8 //установить интервал 60 минут, без сброса счетчика (6)
#define SET_INTERVAL70 9 //установить интервал 70 минут, без сброса счетчика (7)
#define SET_INTERVAL80 10 //установить интервал 80 минут, без сброса счетчика (8)
#define SET_INTERVAL90 11 //установить интервал 90 минут, без сброса счетчика (9)
#define POWEROFF 12 //отключить немедленно и не подавать питание 5 минут (.)
#define RESET_SESSION 13 //—бросить счетчик сессии
#define MAX_DISP_VALUE RESET_SESSION

uint8_t GetRandomFromArray(uint8_t *array, uint8_t size) {
    if (!size)
    return RANDOM_MASK + rnd_cnt;
    uint8_t rnd = (array[0] ^ RANDOM_MASK) + rnd_cnt;
    while (size > 0) {
        rnd = rnd + (array[size] ^ RANDOM_MASK) + rnd_cnt;
        size--;
    }
    return rnd;
}

uint8_t ModExpRepeatedSquaring(uint8_t g, uint8_t x, uint8_t p) {
    uint8_t c = g % p;
    uint8_t d = x;
    uint8_t r = 1;
    uint16_t tmp = 0;
    while (d > 0) {
        if (d % 2) {
            tmp = r;
            tmp = tmp * c;
            r = (uint8_t) (tmp % p);
        }
        d >>= 1;
        tmp = c;
        tmp *= tmp;
        c = (uint8_t) (tmp % p);
    }
    return r;
}

/*
* 0           1                   2             3             4              5
* [public key][encoded start pos][encoded rnd2][encoded rnd2][encoded ident][encoded command]..
*/

uint16_t sesIdent = 0;

void SipherArray(uint8_t *array, uint8_t size, uint8_t command) {
    uint8_t index = 0;
    uint8_t sec = 0, a = 0, A = 0, B = 0;
    a = GetRandomFromArray(array, size) % (SEC_P - 1);
    A = ModExpRepeatedSquaring(SEC_G, a, SEC_P);
    B = ModExpRepeatedSquaring(SEC_G, CLIENT_SEC, SEC_P);
    array[0] = A;
    sec = ModExpRepeatedSquaring(B, a, SEC_P);

    uint8_t pos = 2 + GetRandomFromArray(array, size) % (size - 6);
    array[1] = pos ^ sec;
    for (index = 2; index < size; index++) {
        if (index == pos) {
            array[index] = DEVICE_IDENT ^ sec;
            } else if (index == (pos + 1)) {
            array[index] = command ^ sec;
            } else if (index == (pos + 2)) {
            array[index] = ((uint8_t)sesIdent & 0xFF) ^ sec;
            } else if (index == (pos + 3)) {
            array[index] = ((uint8_t)(sesIdent >> 8) & 0xFF) ^ sec;
            } else {
            array[index] = GetRandomFromArray(array, size) ^ sec;
        }
    }
    sesIdent++;
}

void ClearMessage(uint8_t *array, uint8_t size) {
    uint8_t index = 0;
    for (index = 0; index < size; index++) {
        array[index] = 0;
    }
}

uint8_t DecodeArray(uint8_t *array, uint8_t size, uint16_t *id) {
    uint8_t com = NO_COMMAND;
    uint8_t index = 0;
    uint8_t a = CLIENT_SEC;
    uint8_t sec = ModExpRepeatedSquaring(array[0], a, SEC_P);
    array[0] = sec;
    uint8_t pos = array[1] ^ sec;
    array[1] = pos;
    for (index = 2; index < MESSAGE_LEN; index++) {
        if (index == pos) {
            if ((array[index] ^ sec) != DEVICE_IDENT) {
                ClearMessage(array, size);
                return com;
            }
            array[index] = array[index] ^ sec;
            } else if (index == (pos + 1)) {
            com = array[index] ^ sec;
            array[index] = com;
            } else if (index == (pos + 2)) {
            *id = array[index] ^ sec;
            } else if (index == (pos + 3)) {
            *id = *id | ((uint16_t)(array[index] ^ sec) << 8);
            } else {
            array[index] = array[index] ^ sec;
        }
    }
    return com;
}

uint8_t message[MESSAGE_LEN] = { 0 };
uint8_t current_state = POWEROFF;
uint8_t rxSize = 0;
unsigned char *tmpBuffer;
uint16_t recvIdent = 0;
uint16_t time_counter = 0;
#define MAX_LIGHTS 10
char Lights[MAX_LIGHTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void PowerChanger(uint8_t on_off){
    if (on_off){
        PORTD |= _BV(PD4);
        } else {
        PORTD &= ~_BV(PD4);
    }
}

void flashLight(uint8_t numb, uint8_t on_off){
    switch(numb){
        case 0:{
            if (on_off){
                PORTC |= _BV(PC1);
                } else {
                PORTC &= ~_BV(PC1);
            }
            break;
        }
        case 1:{
            if (on_off){
                PORTC |= _BV(PC0);
                } else {
                PORTC &= ~_BV(PC0);
            }
            break;
        }
        case 2:{
            if (on_off){
                PORTB |= _BV(PB5);
                } else {
                PORTB &= ~_BV(PB5);
            }
            break;
        }
        case 3:{
            if (on_off){
                PORTB |= _BV(PB4);
                } else {
                PORTB &= ~_BV(PB4);
            }
            break;
        }
        case 4:{
            if (on_off){
                PORTB |= _BV(PB3);
                } else {
                PORTB &= ~_BV(PB3);
            }
            break;
        }
        case 5:{
            if (on_off){
                PORTB |= _BV(PB2);
                } else {
                PORTB &= ~_BV(PB2);
            }
            break;
        }
        case 6:{
            if (on_off){
                PORTB |= _BV(PB1);
                } else {
                PORTB &= ~_BV(PB1);
            }
            break;
        }
        case 7:{
            if (on_off){
                PORTB |= _BV(PB0);
                } else {
                PORTB &= ~_BV(PB0);
            }
            break;
        }
        case 8:{
            if (on_off){
                PORTB |= _BV(PB7);
                } else {
                PORTB &= ~_BV(PB7);
            }
            break;
        }
        case 9:{
            if (on_off){
                PORTB |= _BV(PB6);
                } else {
                PORTB &= ~_BV(PB6);
            }
            break;
        }
    }
}

uint16_t maxTimer = 0;
uint8_t light_mem = 0;

void poweroffallLights(){
    flashLight(0, 0);
    flashLight(1, 0);
    flashLight(2, 0);
    flashLight(3, 0);
    flashLight(4, 0);
    flashLight(5, 0);
    flashLight(6, 0);
    flashLight(7, 0);
    flashLight(8, 0);
    flashLight(9, 0);
}

void enableInterval(uint16_t interval, uint8_t com, uint8_t light){
    current_state = com;
    time_counter = 60*interval;
    maxTimer = time_counter;
    PowerChanger(1);
    poweroffallLights();
    flashLight(light, 1);
    light_mem = light;
    
}

void gotoState(uint8_t state){
    if (state>=DISABLE_TIMER && state<=RESET_SESSION){
        switch (state){
            case DISABLE_TIMER:{
                current_state = DISABLE_TIMER;
                time_counter = 0;
                PowerChanger(1);
                poweroffallLights();
                flashLight(9, 1);
                break;
            }
            case RESET_SESSION:{
                sesIdent = 0;
                current_state = DISABLE_TIMER;
                time_counter = 0;
                PowerChanger(1);
                poweroffallLights();
                flashLight(9, 1);
                break;
            }
            case POWEROFF:{
                current_state = POWEROFF;
                time_counter = 0;
                PowerChanger(0);
                poweroffallLights();
                flashLight(9, 1);
                break;
            }
            case RESET_INTERVAL:{
                if (current_state>=SET_INTERVAL10 && current_state<=SET_INTERVAL90){
                    time_counter = maxTimer;
                    PowerChanger(1);
                }
                break;
            }
            case SET_INTERVAL10:{
                enableInterval(10, SET_INTERVAL10, 0);
                break;
            }
            case SET_INTERVAL20:{
                enableInterval(20, SET_INTERVAL20, 1);
                break;
            }
            case SET_INTERVAL30:{
                enableInterval(30, SET_INTERVAL30, 2);
                break;
            }
            case SET_INTERVAL40:{
                enableInterval(40, SET_INTERVAL40, 3);
                break;
            }
            case SET_INTERVAL50:{
                enableInterval(50, SET_INTERVAL50, 4);
                break;
            }
            case SET_INTERVAL60:{
                enableInterval(60, SET_INTERVAL60, 5);
                break;
            }
            case SET_INTERVAL70:{
                enableInterval(70, SET_INTERVAL70, 6);
                break;
            }
            case SET_INTERVAL80:{
                enableInterval(80, SET_INTERVAL80, 7);
                break;
            }
            case SET_INTERVAL90:{
                enableInterval(90, SET_INTERVAL90, 8);
                break;
            }
			default:{
				
			}
        }
    }
}

uint8_t second_counter = 0;
uint8_t is_light = 0;

ISR(TIMER0_OVF_vect){
    TCNT0 = 5;
    second_counter++;
    if (second_counter==125){
        second_counter = 0;
        if ((current_state>=SET_INTERVAL10 && current_state<=SET_INTERVAL90) && time_counter>0){
            time_counter--;
            if (is_light){
                is_light = 0;
                poweroffallLights();
                flashLight(light_mem, 1);
                } else {
                is_light = 1;
                uint16_t temp = maxTimer/9;
                uint8_t cnt = (uint8_t)(time_counter/temp);
                if (cnt>8) cnt = 8;
                poweroffallLights();
                uint8_t ind = 0;
                for(ind=0;ind<cnt;ind++){
                    flashLight(ind, 1);
                }
            }
        }
        if ((current_state>=SET_INTERVAL10 && current_state<=SET_INTERVAL90) && time_counter==0){
            PowerChanger(0);
        }
    }
}

int main(void)
{
    DDRB = 0xFF;
    PORTB = 0;
    DDRC = 0xFF;
    PORTC = 0;
    DDRD |= _BV(PD4);
    PORTD &= _BV(PD4);
    uint8_t index = 0;
    TCCR0 = _BV(CS02);
    TCNT0 = 5;
    TIMSK |= _BV(TOIE0);
    TIFR = _BV(TOV0);
    ManchesterRF(MAN_4800);
    ManchesterRF_RXInit(2, _BV(PD3));
    ManchesterRF_beginReceive();
	PowerChanger(0);
    sei();
    flashLight(9, 1);
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
                    for(index=0; index<MESSAGE_LEN; index++){
                        message[index] = tmpBuffer[index];
                    }
                    uint8_t com = DecodeArray(message, MESSAGE_LEN, &recvIdent);
                    if (com!=NO_COMMAND){
                        if ((com!=RESET_SESSION) && sesIdent>=recvIdent){
                            com = NO_COMMAND;
                            } else {
                            gotoState(com);
                            sesIdent=recvIdent;
                        }
                    }
                }
            }
        }
        #ifdef DEBUG_S
        //  ddelay_ms(1);
        #else
        _delay_ms(1);
        #endif // DEBUG_S
    }
}

