#include "global.h"
#include <avr/io.h>
#include <avr/delay.h>

/*������� ���������� ������� ���. �������� � ���� ��������� ���������*/
byte ReadCode( void ){
    byte ret;
    //������������� ����� �� ����
    LCD_DATA_PORT=0x00;
    LCD_DATA_DDR=0x00; 
    //��������� ��� �� ��� ����� �������� �������
    ClrBit(LCD_CONTROL_PORT, A0);
    SetBit(LCD_CONTROL_PORT, RDWR); 
    //����� ��������� ������ ��� ���������� ���� �             
    _delay_us(10); 
    SetBit(LCD_CONTROL_PORT, E); 
    // ������ �� ���� ��������� �� �����             
    _delay_us(10);
    ret=LCD_DATA_PIN;         
    //���������� ����� � � ���������� ���������
    ClrBit(LCD_CONTROL_PORT,E);
    return ret;
}  
/*������� �������� ������ ����� ���������*/
void WaitBusy(void){
    byte stat;
    //������ ������ ���� �� ���������� ��� BUSY
    stat=ReadCode();
    while(stat==(1<<BUSY)) stat=ReadCode();
}
//���� ����� ������ ��� RESET
void WaitReset(void){
    byte stat;
    stat=ReadCode();
    while(stat==(1<<RESET)) stat=ReadCode();
} 
//���� ����� ������ ��� ON/OFF
void WaitON(void){
    byte stat;
    stat=ReadCode();
    while(stat==(1<<ONOFF)) stat=ReadCode();
}
/*������� ���������� ����� �� ������ ���*/
byte ReadData( void ){
    byte ret;
    //���� ���� �������� ������������
    WaitBusy();
    //������������� ����
    LCD_DATA_PORT=0x00;
    LCD_DATA_DDR=0x00; 
    //����� ������ ������
    SetBit(LCD_CONTROL_PORT, A0);
    SetBit(LCD_CONTROL_PORT, RDWR); 
    //����� ��������� ������ ��� ���������� ���� �                     
    _delay_us(10);
    SetBit(LCD_CONTROL_PORT, E); 
    //������ �� ���� ��������� �� �����                
    _delay_us(10);
    ret=LCD_DATA_PIN;         
    //���������� ����� �
    ClrBit(LCD_CONTROL_PORT,E);
    //���������� ���������
    return ret;
 }
/*������� ������� ������ �� ���
��� ����� ����� ��� � �� �������*/
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
/*������� ������ ������� �� �������*/
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
/*������� ������������� �������*/                              
void LCD_INIT( void ){ 
    byte stat;
    //��������� �������� Jtag
    MCUCSR = 0x80;
    MCUCSR = 0x80;
    //������������� ����
    LCD_DATA_DDR=0xFF;           
    LCD_CONTROL_DDR=0xFF;
    //��� ����� ������������ ����� � 0. � ��� ����� � RES
    LCD_CONTROL_PORT=0x00;
    //���������� ������ ������ �� �����
    _delay_ms(100); 
    //���������� ������ ������.(������ RES �������������)      
    SetBit(LCD_CONTROL_PORT, RES);    
    //�������� 1 �������   
    SetBit(LCD_CONTROL_PORT, E1); 
    //�������� ������� � ������������� ��������� �����
    LCD_ON;
    LCD_START_LINE(0);

    ClrBit(LCD_CONTROL_PORT, E1);
    //���� ����� ��� 2-�� ��������
    SetBit(LCD_CONTROL_PORT, E2);
    WaitBusy();
    LCD_ON;
    LCD_START_LINE(0);
}                                   
//������� �������� ������
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