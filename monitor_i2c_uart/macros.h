/*Фуйл с моими макросами*/
#define byte unsigned char
#define word unsigned int

#define uc unsigned char
#define sc signed char

#define SetBit(x,y) (x|=(1<<y))
#define ClrBit(x,y) (x&=~(1<<y))
#define ToggleBit(x,y) (x^=(1<<y))
#define FlipBit(x,y) (x^=(1<<y)) // Same as ToggleBit.
#define TestBit(x,y) (x&(1<<y))  

#define _BV(bit) (1 << (bit)) 