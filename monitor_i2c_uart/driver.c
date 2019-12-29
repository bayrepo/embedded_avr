#include "global.h"
#include <avr/io.h>
#include <avr/delay.h>

/*Функция считывания статуса ЛСД. Работает с ране выбронным кристалом*/
byte ReadCode( void ){
    byte ret;
    //Конфигурируем порты на ввод
    LCD_DATA_PORT=0x00;
    LCD_DATA_DDR=0x00; 
    //Указываем ЛСД то что будет читаться команда
    ClrBit(LCD_CONTROL_PORT, A0);
    SetBit(LCD_CONTROL_PORT, RDWR); 
    //Нужно подождать прежде чем выстовлять стоб Е             
    _delay_us(10); 
    SetBit(LCD_CONTROL_PORT, E); 
    // Данные на шине появються не сразу             
    _delay_us(10);
    ret=LCD_DATA_PIN;         
    //Сбрасываем строб Е и возврощаем результат
    ClrBit(LCD_CONTROL_PORT,E);
    return ret;
}  
/*Функция ожидания сброса флага занятости*/
void WaitBusy(void){
    byte stat;
    //Читаем статус пока не сброситься бит BUSY
    stat=ReadCode();
    while(stat==(1<<BUSY)) stat=ReadCode();
}
//Тоже самое только для RESET
void WaitReset(void){
    byte stat;
    stat=ReadCode();
    while(stat==(1<<RESET)) stat=ReadCode();
} 
//Тоже самое только для ON/OFF
void WaitON(void){
    byte stat;
    stat=ReadCode();
    while(stat==(1<<ONOFF)) stat=ReadCode();
}
/*Функция считывания байта из памяти ЛСД*/
byte ReadData( void ){
    byte ret;
    //Ждем пока кристалл освободиться
    WaitBusy();
    //Конфигурируем порт
    LCD_DATA_PORT=0x00;
    LCD_DATA_DDR=0x00; 
    //Будим читать данные
    SetBit(LCD_CONTROL_PORT, A0);
    SetBit(LCD_CONTROL_PORT, RDWR); 
    //Нужно подождать прежде чем выстовлять стоб Е                     
    _delay_us(10);
    SetBit(LCD_CONTROL_PORT, E); 
    //Данные на шине появються не сразу                
    _delay_us(10);
    ret=LCD_DATA_PIN;         
    //Сбрасываем строб Е
    ClrBit(LCD_CONTROL_PORT,E);
    //Возврощаем результат
    return ret;
 }
/*Функция выводит данные на ЛСД
Все почти также как и со чтением*/
void WriteData(byte data){ 
    WaitBusy();   
    LCD_DATA_DDR=0xFF;
    
    SetBit(LCD_CONTROL_PORT, A0);
    ClrBit(LCD_CONTROL_PORT, RDWR);
    
    _delay_us(10);
    SetBit(LCD_CONTROL_PORT, E);
    LCD_DATA_PORT=data;
                               
    _delay_us(10);
    ClrBit(LCD_CONTROL_PORT, E);
}
/*Функция вывода команды на дисплей*/
void WriteCode(byte code){         
    WaitBusy();
    LCD_DATA_DDR=0xFF;    
    
    ClrBit(LCD_CONTROL_PORT, A0);
    ClrBit(LCD_CONTROL_PORT, RDWR);
     
    _delay_us(10);
    SetBit(LCD_CONTROL_PORT, E);
    LCD_DATA_PORT=code;
    
    _delay_us(10);
    ClrBit(LCD_CONTROL_PORT, E);
}
/*Функция Инициализации дисплея*/                              
void LCD_INIT( void ){ 
    byte stat;
    //Програмно вырубаем Jtag
    MCUCSR = 0x80;
    MCUCSR = 0x80;
    //Конфигурируем порт
    LCD_DATA_DDR=0xFF;           
    LCD_CONTROL_DDR=0xFF;
    //Все линии управляещего порта в 0. В том числе и RES
    LCD_CONTROL_PORT=0x00;
    //Удерживаем сигнал сброса на линии
    _delay_ms(100); 
    //Сбрасываем сигнал Сброса.(Сигнал RES инвертирующий)      
    SetBit(LCD_CONTROL_PORT, RES);    
    //Выбираем 1 кристал   
    SetBit(LCD_CONTROL_PORT, E1); 
    //Включаем дисплей и устанавливаем стартовую линию
    LCD_ON;
    LCD_START_LINE(0);

    ClrBit(LCD_CONTROL_PORT, E1);
    //Тоже самое для 2-го кристала
    SetBit(LCD_CONTROL_PORT, E2);
    WaitBusy();
    LCD_ON;
    LCD_START_LINE(0);
}                                   
//Функция отчистки экрана
void LCD_CLS(void){
    byte i;
    byte j;  
    byte stat;

    ClrBit(LCD_CONTROL_PORT, E2); 
    _delay_us(10);
    SetBit(LCD_CONTROL_PORT, E1);
    LCD_OFF;
    LCD_SET_ADDRESS(0);
    for(i=0;i<8; i++){
        LCD_SET_PAGE(i);
        for(j=0; j<64; j++){
            WriteData(0x00);
        }
    }                  
    
    LCD_ON;
    ClrBit(LCD_CONTROL_PORT, E1);
    _delay_us(10);
    SetBit(LCD_CONTROL_PORT, E2);
    LCD_OFF;
    LCD_SET_ADDRESS(0);
    for(i=0;i<8; i++){
        LCD_SET_PAGE(i); 
        for(j=0; j<64; j++){
            WriteData(0x00);
        }
    } 
    LCD_ON;
}  