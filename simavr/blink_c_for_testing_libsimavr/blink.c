#define F_CPU 9600000

#include <avr/io.h>
#include <util/delay.h>

void main() {
    DDRB = 0xff;
    while (1) {
        PORTB = 0xff;
        _delay_ms(1);
        PORTB = 0;
        _delay_ms(1);
    }
}