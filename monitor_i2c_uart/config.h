//Настройки драйвера
//К каким ногам подключены порты дисплея
#define A0 6
#define RDWR 7
#define E 2
#define E1 3
#define E2 4
#define RES 5 
//Расположение флагов на шине
#define BUSY 7
#define ONOFF 5
#define RESET 4
//Какой порт за что           
#define LCD_DATA_PORT   PORTB
#define LCD_DATA_DDR     DDRB
#define LCD_DATA_PIN     PINB
#define LCD_CONTROL_PORT     PORTD
#define LCD_CONTROL_DDR     DDRD
//Максимальное разрешение экрана
#define MAX_X 127
#define MAX_Y 63  