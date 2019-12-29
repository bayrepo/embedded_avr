#include "protocol.h"
#include <string.h>

void protocol_prepare_send_buffer(item_info *buffer, byte length){
	memset(buffer, 0, sizeof(item_info) * length);
}

void *protocol_associate_data(item_info *buffer, byte index, void *data, protocol_data_type type, char *name){
	item_info *ptr_buffer = (item_info *)((char *)buffer + sizeof(item_info) * index);//sizeof(item_info) * index);
	ptr_buffer->type = (byte)type;
	strncpy(ptr_buffer->name, name, sizeof(ptr_buffer->name));
	switch(type){
		case PROTO_UNSBYTE:
		case PROTO_BYTE_HEX:
		{
			byte *buf = (byte *)data;
			ptr_buffer->data.data1 = *buf;
			return &(ptr_buffer->data.data1);
		}
		case PROTO_INT:
		{
			int *buf = (int *)data;
			ptr_buffer->data.data2 = *buf;
			return &(ptr_buffer->data.data2);
		}
		case PROTO_STRING:
		{
			strncpy(&(ptr_buffer->data.data4), (char *)data, sizeof(ptr_buffer->data.data4));
			return &(ptr_buffer->data.data4);
		}
		default:{
			return NULL;
		}
	}
}
