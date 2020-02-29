/*
* power_manager_transmitter.c
*
* Created: 25.01.2020 17:26:06
* Author : bayrepo.info@gmail.com
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
uint16_t sesIdent = 0;

#define NO_COMMAND 0
#define DISABLE_TIMER 1 //выключить счет, работать все врем€ (-)
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

#define MAX_TRANSMIT_SHOW 7

const uint8_t displayData[MAX_DISP_VALUE] = {
    0xA4, 0x80, 0x77, 0x14, 0xB3, 0xB6, 0xD4, 0xE6, 0xE7, 0x34, 0xF7, 0xF6, 0x08
};

const uint8_t displayTransmit[MAX_TRANSMIT_SHOW] = {
    0x40, 0x20, 0x10, 0x04, 0x02, 0x01, 0x40
};

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

uint8_t message[MESSAGE_LEN] = { 0 };
unsigned char keySelect = 0;
unsigned char keySend = 0;
uint8_t current_mode = DISABLE_TIMER;

void showMode(){
    if (current_mode>=MAX_DISP_VALUE){
        PORTB = displayData[0];
    } else {
        PORTB = displayData[current_mode];
    }
}

void showShow(){
    uint8_t index = 0;
    for(index=0;index<MAX_TRANSMIT_SHOW;index++){
        PORTB = displayTransmit[index];
        _delay_ms(200);
    }
    showMode();
}

void sendMode(uint8_t command){
    ClearMessage(message, MESSAGE_LEN);
    SipherArray(message, MESSAGE_LEN, command);
    ManchesterRF_transmitArray(MESSAGE_LEN, message);
    if (!sesIdent) {
        ClearMessage(message, MESSAGE_LEN);
        SipherArray(message, MESSAGE_LEN, RESET_SESSION);
        ManchesterRF_transmitArray(MESSAGE_LEN, message);
    }
}

ISR(TIMER0_OVF_vect){
    rnd_cnt++;
}

int main(void)
{
    DDRB = 0xFF;
    PORTB = 0;
    DDRC &=~(_BV(PC2) | _BV(PC3));
    PORTC &=~(_BV(PC2) | _BV(PC3));
    TCCR0 = _BV(CS00);
    TCNT0 = 0;
    TIMSK |= _BV(TOIE0);
    showMode();
    ManchesterRF(MAN_4800);
    ManchesterRF_TXInit(1, _BV(PINC1));
    sei();
    _delay_ms(10);
    sendMode(RESET_SESSION);
    sendMode(DISABLE_TIMER);
    while (1)
    {
        //Check button
        if ((PINC & _BV(PINC2)) && !keySelect){
            keySelect = 1;
        }
        if (!(PINC & _BV(PINC2)) && keySelect){
            keySelect = 0;
            current_mode++;
            if (current_mode>POWEROFF){
                current_mode = DISABLE_TIMER;
            }
            showMode();
        }
        if ((PINC & _BV(PINC3)) && !keySend){
            keySend = 1;
        }
        if (!(PINC & _BV(PINC3)) && keySend){
            keySend = 0;
            sendMode(current_mode);
            showShow();
        }
    }
}

