#include "global.h"
#include <avr/io.h>
#include <util/delay.h>

/*Функция которая ресует на экране 1 пиксел*/
void LCD_PUT_PIXEL(byte x, byte y){
    //Обьявляем переменные
    byte bite;
    byte page; 
    byte data, data2;
    //Выход если точка лежит вне экрана 
    if((x>MAX_X)|(y>MAX_Y)) return;
    //Выбираем кристалл
    if(x>=64){
        ClrBit(LCD_CONTROL_PORT, E1);
        SetBit(LCD_CONTROL_PORT, E2);
        x=x-64;
        }
        else{
        ClrBit(LCD_CONTROL_PORT, E2);
        SetBit(LCD_CONTROL_PORT, E1);
        }   
    //page-номер страницы 
    page=y/8;
    //bite-Номер байта который нам предстоит вывести
    bite=y%8;
    //Устанавливаем страницу и адресс
    LCD_SET_PAGE(page);
    LCD_SET_ADDRESS(x);
    //Читаем байт с LCD(2 цикла т.к. в первом мусор)
    data=ReadData(); 
    data=ReadData();
    //В зависимости от метода вывода выводим наш пиксел.
    // Адресс страницы не менялся, поэтому его не устанавливаем  
    switch(met){
        case MET_OR : { data=data|(1<<bite); break;}
        case MET_XOR : {data=data^(1<<bite); break;}  
    }
    LCD_SET_ADDRESS(x);
    WriteData(data);
}
/*Функция вывода прямой по алгоритму Брезенхома.
Подробно ее описывать не буду т.к. по ней лучше почитать в интернете*/
void LCD_LINE(byte x1, byte y1, byte x2, byte y2){
        int dx, dy, i1, i2, i, kx, ky;
        int d;     
        int x = x1, y = y1;
        int flag;

        dy = y2 - y1;
        dx = x2 - x1;
        if (dx == 0 && dy == 0){
                LCD_PUT_PIXEL(x,y);    
                return;
        }
        kx = 1; 
        ky = 1; 

        if( dx < 0 ){ dx = -dx; kx = -1; }
        else if(dx == 0)        kx = 0;    

        if(dy < 0) { dy = -dy; ky = -1; }

        if(dx < dy){ flag = 0; d = dx; dx = dy; dy = d; }
        else         flag = 1;

        i1 = dy + dy; d = i1 - dx; i2 = d - dx;
        x = x1; y = y1;

        for(i=0; i < dx; i++){
                LCD_PUT_PIXEL(x,y);     

                if(flag) x += kx;
                else     y += ky;

                if( d < 0 ) 
                         d += i1;
                else{       
                         d += i2;
                         if(flag) y += ky; 
                         else     x += kx;
                }
        }
        LCD_PUT_PIXEL(x,y);
}
    
/*Функция вывода окружности по алгоритму Брезенхома. При малом радиусе есть неточности, но работает гораздо быстрее sin и cos*/
void LCD_CIRCLE( byte xc, byte yc, byte r )
{
  int d, x, y;
  x=0;
  y=r;
  d=3-2*r;
  LCD_PUT_PIXEL(x+xc,y+yc);
  LCD_PUT_PIXEL(x+xc,-y+yc);
  LCD_PUT_PIXEL(-x+xc,-y+yc);
  LCD_PUT_PIXEL(-x+xc,y+yc);
  LCD_PUT_PIXEL(y+xc,x+yc);
  LCD_PUT_PIXEL(y+xc,-x+yc);
  LCD_PUT_PIXEL(-y+xc,-x+yc);
  LCD_PUT_PIXEL(-y+xc,x+yc);
  while(x<y){
        if(d<=0){
        d=d+4*x+6;
        }else{
        d=d+4*(x-y)+10;
        y--;
        }   
  x++;
  //LCD_PUT_PIXEL(x, y);
  LCD_PUT_PIXEL(x+xc,y+yc);
  LCD_PUT_PIXEL(x+xc,-y+yc);
  LCD_PUT_PIXEL(-x+xc,-y+yc);
  LCD_PUT_PIXEL(-x+xc,y+yc);
  LCD_PUT_PIXEL(y+xc,x+yc);
  LCD_PUT_PIXEL(y+xc,-x+yc);
  LCD_PUT_PIXEL(-y+xc,-x+yc);
  LCD_PUT_PIXEL(-y+xc,x+yc);
  }  
}       