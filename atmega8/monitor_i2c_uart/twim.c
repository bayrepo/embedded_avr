//***************************************************************************
//
//  Author(s)...: ����� ������  http://ChipEnable.Ru   
//
//  Target(s)...: mega16
//
//  Compiler....: GCC
//
//  Description.: ������� �������� TWI ����������. 
//                ��� ������� �� Atmel`������ ����� - AVR315.
//
//  Data........: 13.11.13
//
//***************************************************************************
#include "twim.h"

#define TWSR_MASK     0xfc  

volatile static uint8_t *twiBuf;
volatile static uint8_t twiState = TWI_NO_STATE;      
volatile static uint8_t twiMsgSize;
static uint8_t struct_sz = 0;       

/*������������ ��� ��������� �������� ������ twi ������*/
uint8_t pre[4] = {2, 8, 32, 128};

/****************************************************************************
 ������������� � ��������� ������� SCL �������
****************************************************************************/
uint8_t TWI_MasterInit(uint16_t fr, uint8_t sz)
{
  uint8_t i;
  uint16_t twbrValue;
  struct_sz = sz;
  
  for(i = 0; i<4; i++){
    twbrValue = ((((F_CPU)/1000UL)/fr)-16)/pre[i];
    if ((twbrValue > 0)&& (twbrValue < 256)){
       TWBR = (uint8_t)twbrValue;
       TWSR = i;
       TWDR = 0xFF;
       TWCR = (1<<TWEN);
       return TWI_SUCCESS;
    }
  }
  return 0;  
}    

/****************************************************************************
 �������� - �� ����� �� TWI ������. ������������ ������ ������
****************************************************************************/
static uint8_t TWI_TransceiverBusy(void)
{
  return (TWCR & (1<<TWIE));                 
}

/****************************************************************************
 ����� ������ TWI ������
****************************************************************************/
uint8_t TWI_GetState(void)
{
  while (TWI_TransceiverBusy());             
  return twiState;                        
}

/****************************************************************************
 �������� ��������� msg �� msgSize ������ �� TWI ����
****************************************************************************/
void TWI_SendData(uint8_t *msg, uint8_t msgSize)
{
  uint8_t i;

  while(TWI_TransceiverBusy());   //����, ����� TWI ������ �����������             

  twiMsgSize = msgSize;           //��������� ���. ���� ��� ��������             
  twiBuf = msg;            //� ������ ���� ��������� 
                       
  twiState = TWI_NO_STATE ;
  TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA); //��������� ���������� � ��������� ��������� �����                            
}

/****************************************************************************
 ���������� ���������� ������ � ����� msg � ���������� msgSize ����. 
****************************************************************************/
uint8_t TWI_GetData(uint8_t *msg, uint8_t msgSize)
{
  uint8_t i;

  while(TWI_TransceiverBusy());    //����, ����� TWI ������ ����������� 
  
  return twiState;                                   
}

/****************************************************************************
 ���������� ���������� TWI ������
****************************************************************************/
ISR(TWI_vect)
{
  static uint8_t ptr;
  uint8_t stat = TWSR & TWSR_MASK;
  
  switch (stat){
    
    case TWI_START:                   // ��������� START ������������ 
    case TWI_REP_START:               // ��������� ��������� START ������������        
       ptr = 0;      

    case TWI_MTX_ADR_ACK:             // ��� ������� ����� SLA+W � �������� �������������
    case TWI_MTX_DATA_ACK:            // ��� ������� ���� ������ � �������� �������������  
       if (ptr < twiMsgSize){
          TWDR = twiBuf[ptr];                    //��������� � ������� ������ ��������� ����
          TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT); //���������� ���� TWINT    
          ptr++;
       }
       else{
          twiState = TWI_SUCCESS;  
          TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO)|(0<<TWIE); //��������� ��������� ����, ���������� ����, ��������� ����������
       }
       break;
     
    case TWI_MRX_DATA_ACK:          //���� ������ ������ � �������� �������������  
       twiBuf[ptr] = TWDR;
	   if (ptr == 1) {
		   twiMsgSize = twiBuf[ptr] * struct_sz + 2;
	   }
       ptr++;
    
    case TWI_MRX_ADR_ACK:           //��� ������� ����� SLA+R � �������� ������������  
      if (ptr < (twiMsgSize-1)){
        TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA);  //���� ��� �� ������������� �������� ����, ��������� �������������                             
      }
      else {
        TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT);            //���� ������� ������������� ����, ������������� �� ���������
      }    
      break; 
      
    case TWI_MRX_DATA_NACK:       //��� ������ ���� ������ ��� �������������      
      twiBuf[ptr] = TWDR;
      twiState = TWI_SUCCESS;  
      TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO); //��������� ��������� ����
      break; 
     
    case TWI_ARB_LOST:          //��� ������� ��������� 
      TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA); // ���������� ���� TWINT, ��������� ��������� �����
      break;
      
    case TWI_MTX_ADR_NACK:      // ��� ������� ���� SLA+W � �� �������� �������������
    case TWI_MRX_ADR_NACK:      // ��� ������� ����� SLA+R � �� �������� �������������    
    case TWI_MTX_DATA_NACK:     // ��� ������� ���� ������ � �� �������� �������������
    case TWI_BUS_ERROR:         // ������ �� ���� ��-�� ����������� ��������� ����� ��� ����
    default:     
      twiState = stat;                                                                                    
      TWCR = (1<<TWEN)|(0<<TWIE)|(0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC); //��������� ����������                              
  }
}
