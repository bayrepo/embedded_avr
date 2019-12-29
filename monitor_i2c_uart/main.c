/*
* monitor_i2c.c
*
* Created: 25.11.2019 1:44:17
* Author : bayre
*/

#include "global.h"
#include <stdio.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include "protocol.h"
#include "twim.h"

byte screen = 0;
byte selected_device = 0;

#define MAX_FIELDS 8
#define MAX_DEVICES 8

byte buttonScreenPull = 0;
byte deviceIndexPull = 0;

typedef struct __device_info{
	byte device_name;
	item_info flds[MAX_FIELDS];
} device_info;

device_info global_storage[MAX_DEVICES];
#define UART_MAX_LEN 22
#define UART_MAX_LINES 6
unsigned char UART_array[UART_MAX_LINES][UART_MAX_LEN];

static void init_storage(){
	memset(&global_storage, 0, sizeof(global_storage));
}

static void clean_device(byte index){
	if (index<MAX_DEVICES){
		global_storage[index].device_name = 0;
		memset(&(global_storage[index].flds), 0, sizeof(global_storage[index].flds));
	}
}

static void add_element(byte device, item_info *data){
	byte index = 0;
	byte need_device = MAX_DEVICES;
	for (index=0; index<MAX_DEVICES; index++){
		if (global_storage[index].device_name == device){
			need_device = index;
			break;
		} else {
			if (!global_storage[index].device_name && need_device == MAX_DEVICES) {
				need_device = index;
			}
		}
	}
	if (need_device == MAX_DEVICES) return;
	if (!global_storage[need_device].device_name){
		clean_device(need_device);
		global_storage[need_device].device_name = device;
		memcpy(&(global_storage[need_device].flds[0]), (void *)data, sizeof(item_info));
	} else {
		byte fnd = 0;
		for(index=0;index<MAX_FIELDS;index++){
			if (global_storage[need_device].flds[index].type &&
			   !strncmp(global_storage[need_device].flds[index].name, data->name, sizeof(data->name))){
				   memcpy(&(global_storage[need_device].flds[index]), (void *)data, sizeof(item_info));
				   fnd = 1;
				   break;
			   }
		}
		if (fnd==0){
			for(index=0;index<MAX_FIELDS;index++){
				if (!global_storage[need_device].flds[index].type){
					memcpy(&(global_storage[need_device].flds[index]), (void *)data, sizeof(item_info));
					break;
				}
			}
		}
	}
	return;
}

static void remove_device(byte device){
	byte index = 0;
	byte jndex = 0;
	for (index=0; index<MAX_DEVICES; index++){
		if (global_storage[index].device_name == device){
			if (index == (MAX_DEVICES - 1)){
				clean_device(device);
				break;
				} else {
				for(jndex=(index+1); jndex < MAX_DEVICES; jndex++){
					memcpy(&(global_storage[jndex-1]), &(global_storage[jndex]), sizeof(device_info));
				}
				clean_device(MAX_DEVICES - 1);
				break;
			}
		}
	}
}

static void printAllEnabledDevices(){
	byte index = 0;
	byte dx = 0;
	char buffer[5];
	LCD_PUTSF(1,1, "Show detected devices");
	LCD_LINE(0,9, MAX_X, 9);
	for(index = 0; index < MAX_DEVICES; index++){
		dx = 0;
		if (index >= 4){
			dx += MAX_X / 2 + 2;
		}
		if (global_storage[index].device_name){
			sprintf(buffer, "%d", global_storage[index].device_name);
			LCD_PUTSF_B(1 + dx, (index % 4) * 9 + 11, buffer);
			} else {
			LCD_PUTSF(1 + dx, (index % 4) * 9 + 11, "NO DEV");
		}
	}
	LCD_LINE(MAX_X / 2,10, MAX_X / 2, MAX_Y);
}

static void printDeviceInfo(byte selected_device){
	byte index = 0;
	byte dx = 0;
	char buffer[20];
	LCD_PUTSF(1,1, "DEV:");
	sprintf(buffer, "%d", global_storage[index].device_name);
	LCD_PUTSF_B(24,1, buffer);
	LCD_LINE(0,9, MAX_X, 9);
	for(index = 0; index < MAX_FIELDS; index++){
		dx = 0;
		if (index >= 5){
			dx += MAX_X / 2 + 2;
		}
		if (global_storage[selected_device].flds[index].type>0){
			memset(buffer, 0, 11);
			strcpy(buffer, global_storage[selected_device].flds[index].name);
			LCD_PUTSF_B(1 + dx, (index % 5) * 9 + 11, buffer);
			switch (global_storage[selected_device].flds[index].type){
				case 1:{
					sprintf(buffer, "%d", global_storage[selected_device].flds[index].data.data1);
					LCD_PUTSF_B(25 + dx, (index % 5) * 9 + 11, buffer);
					break;
				}
				case 2:{
					sprintf(buffer, "%02X", global_storage[selected_device].flds[index].data.data1);
					LCD_PUTSF_B(25 + dx, (index % 5) * 9 + 11, buffer);
					break;
				}
				case 3:{
					sprintf(buffer, "%d", global_storage[selected_device].flds[index].data.data2);
					LCD_PUTSF_B(25 + dx, (index % 5) * 9 + 11, buffer);
					break;
				}
				case 4:{
					sprintf(buffer, "%s", global_storage[selected_device].flds[index].data.data4);
					LCD_PUTSF_B(25 + dx, (index % 5) * 9 + 11, buffer);
					break;
				}
			}
			} else {
			LCD_PUTSF(1 + dx, (index % 5) * 9 + 11, "NO VAR");
		}
	}
	LCD_LINE(MAX_X / 2,10, MAX_X / 2, MAX_Y);
}

static byte getNextDevice(byte current_device){
	byte old_device = current_device;
	byte jump_begin = 0;
	current_device++;
	while(old_device!=current_device){
		jump_begin = 0;
		if (current_device>=MAX_DEVICES){
			current_device = 0;
			jump_begin = 1;
		}
		if (global_storage[current_device].device_name){
			return current_device;
		}
		if (!jump_begin) current_device++;
	}
	return 0;
}

static byte i2c_buffer[sizeof(item_info)*MAX_FIELDS+2];

static void scanBus(){
	byte index = 0;
	byte jndex = 0;
	byte resp = 0;
	for(index = 1; index < 127; index++){
		memset(i2c_buffer, 0, sizeof(item_info)*MAX_FIELDS+2);
		resp = 0;
		i2c_buffer[0] = (index << 1) | 1;
		TWI_SendData(i2c_buffer, sizeof(item_info)*MAX_FIELDS+2 );
		while(TWI_GetState()==TWI_NO_STATE);
		if (TWI_GetState()==TWI_SUCCESS){
			resp = 1;
			TWI_GetData(i2c_buffer, sizeof(item_info)*MAX_FIELDS+2 );
			if (i2c_buffer[1]>0){
				for(jndex=0;jndex<i2c_buffer[1];jndex++){
					item_info *p = (item_info *)(((byte *)i2c_buffer) + 2 + sizeof(item_info)*jndex);
					if (p->type!=PROTO_UNDEF){
							add_element(index, p);
					}
				}
			}
		}
		if (!resp){
			remove_device(index);
		}
	}
}

static void initUART_array(){
	byte index = 0;
	byte jndex = 0;
	for (index=0;index<UART_MAX_LINES;index++){
		for(jndex=0;jndex<UART_MAX_LEN;jndex++){
			UART_array[index][jndex]=0;
		}
	}
}

static void printUART(){
	byte index = 0;
	LCD_PUTSF(1,1, "UART output");
	LCD_LINE(0,9, MAX_X, 9);
	for (index=0;index<UART_MAX_LINES;index++){
		LCD_PUTSF_B(1, index * 9 + 11, (char *)UART_array[index]);
	}
}

byte position = 0;

static void shiftUARTLines(){
	byte index = 0;
	for(index=0; index < UART_MAX_LINES - 1; index++){
		memcpy((char *)UART_array[index], (char *)UART_array[index+1], UART_MAX_LEN);
	}
	memset((char *)UART_array[UART_MAX_LINES-1], 0, UART_MAX_LEN);
}

ISR(USART_RXC_vect)
{
	unsigned char temp;
	temp=UDR;
	if (position>=UART_MAX_LEN || temp=='\n'){
		shiftUARTLines();
		position = 0;
	}
	if (temp>=0x20 && temp<=0x7E){
		UART_array[UART_MAX_LINES-1][position]=temp;
		position++;
	}
}

void USART_Init( unsigned int ubrr)
{
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	UCSRB=(1<<RXEN)|( 1<<TXEN);
	UCSRB |= (1<<RXCIE);
	UCSRA &= ~(1<<U2X);
	/* ”станавливаем формат данных 8 бит данных, 2 стоп бита */
	UCSRC = (1<<URSEL)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
}

int main(void)
{
	DDRC=~(_BV(DDC3) | _BV(DDC2));
	PORTC=0x00;
	
	USART_Init(51); //9600 bod
	initUART_array();
	init_storage();
	LCD_INIT();
	met=MET_OR;
	TWI_MasterInit(50, sizeof(item_info));
	
	sei();
	
	while (1)
	{
		scanBus();
		LCD_CLS();
		
		if ((PINC & _BV(PINC2)) && !buttonScreenPull){
			buttonScreenPull = 1;
		}
		if (!(PINC & _BV(PINC2)) && buttonScreenPull){
			screen++;
			if (screen > 2) screen = 0;
			buttonScreenPull = 0;
		}
		if ((PINC & _BV(PINC3)) && !deviceIndexPull){
			deviceIndexPull = 1;
		}
		if (!(PINC & _BV(PINC3)) && deviceIndexPull){
			selected_device = getNextDevice(selected_device);
			deviceIndexPull = 0;
		}
		
		if (screen < 2){
			if (!screen || !global_storage[selected_device].device_name){
				printAllEnabledDevices();
			} else {
				printDeviceInfo(selected_device);
			}
		} else {
			printUART();
		}
		
		_delay_ms(300);
	}
}

