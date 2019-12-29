#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define PROTOCOL_MAX_LEN 6
#define PROTOCOL_MAX_NAME_LEN 3

#ifndef byte
#define byte unsigned char
#endif

typedef enum _protocol_data_type
{
	PROTO_UNDEF = 0,
	PROTO_UNSBYTE,
	PROTO_BYTE_HEX,
	PROTO_INT,
	PROTO_STRING
} protocol_data_type;

/*
* type: //0 - undefined, 1 - unsigned byte/char decimal, 2 - byte/char hex, 3 - int, 4 - string
*/

typedef struct __attribute__ ((packed)) __item_info{
	byte type;
	char name[PROTOCOL_MAX_NAME_LEN];
	union Data {
		byte data1;
		int data2;
		char data4[PROTOCOL_MAX_LEN];
	} data;
} item_info;

void protocol_prepare_send_buffer(item_info *buffer, byte length);
void *protocol_associate_data(item_info *buffer, byte index, void *data, protocol_data_type type, char *name);

#endif