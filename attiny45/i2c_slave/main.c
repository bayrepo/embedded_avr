/*
* atiny_i2c_slave.c
*
* Created: 28.11.2019 18:45:29
* Author : bayre
*/

#define F_CPU 8000000

#include <avr/io.h>
#include <avr/delay.h>
#include "protocol.h"
#include "TWI.h"

extern char incomingBuffer[1];
extern char outgoingBuffer[sizeof(item_info)*4+2];


int main(void)
{
	unsigned char tmp_data = 0;
	int tmp_data1 = 0;
	float tmp_data2 = 0.5;
	sei();   //!< Enable global interrupts.
	protocol_prepare_send_buffer((item_info *)(outgoingBuffer+1), 3);
	unsigned char *dt = protocol_associate_data((item_info *)(outgoingBuffer+1), 0, &tmp_data, PROTO_BYTE_HEX, "T1");
	int *dt_int = protocol_associate_data((item_info *)(outgoingBuffer+1), 1, &tmp_data1, PROTO_INT, "T2");
	unsigned char *dt_s = protocol_associate_data((item_info *)(outgoingBuffer+1), 2, "DT", PROTO_STRING, "T3");
	outgoingBuffer[0] = 3;

	twi_slave_init();
	twi_slave_enable();
	while (1)
	{
		(*dt)++;
		(*dt_int)++;
		_delay_ms(100);
	}
}

