//Наш метод вывода
byte met;
//Дефайни методов. Текст запомнить легче
#define MET_OR 0
#define MET_XOR 1 
#define MET_NOT_OR 2
#define MET_NOT_XOR 3

byte ReadCode( void );
void WaitBusy(void);
void WaitReset(void);
void WaitON(void);
byte ReadData( void );
void WriteData(byte data);
void WriteCode(byte code);

void LCD_INIT( void );
void LCD_CLS(void);

//Список команд для ЛСД                         
#define LCD_ON WriteCode(0x3F)
#define LCD_OFF WriteCode(0x3E) 
#define LCD_START_LINE(x) WriteCode(0xC0 | x)
#define LCD_SET_PAGE(x) WriteCode(0xB8 | x)
#define LCD_SET_ADDRESS(x) WriteCode(0x40 | x) 

void LCD_PUT_PIXEL(byte x, byte y);
void LCD_LINE(byte x1, byte y1, byte x2, byte y2);
void LCD_CIRCLE( byte xc, byte yc, byte r );

void LCD_PUT_BYTE(byte x, byte y, byte data);
void LCD_PUTC(byte x, byte y, byte ch);
void LCD_PUTSF(byte x, byte y, const unsigned char str[]);
void LCD_PUTSF_B(byte x, byte y, char str[]);