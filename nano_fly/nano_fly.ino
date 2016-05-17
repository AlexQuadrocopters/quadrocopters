/*
������� ������ Arduino Nano
����������� �� �������������.
������������� ������������:
+++++++++++++++++++++++++++++++++++++++
GPS ������ GY-GPS6MV2 (NEO-6M-0-001)
�����������:
    Arduino Nano    GY-GPS6MV2
	VCC +5V          VCC
	  GND            GND
      D4             RX
	  D5             TX
---------------------------------------








*/


#include <modbus.h>                     // ���������� ��������� MODBUS 
#include <modbusDevice.h>               // ���������� ��������� MODBUS 
#include <modbusRegBank.h>              // �������� ��������� MODBUS 
#include <modbusSlave.h>                // ���������� �������(�������)���������  MODBUS

#include <MsTimer2.h>                   // ���������� �������


#define  Pin8   8                       // ����������  
#define  Pin13  13                      // ��������� ����������� � ��������� ������ 13 
#define  PinA0  A0                      // ����������  


//+++++++++++++++++++ MODBUS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

modbusDevice regBank;                  // ����������� ��������� 
modbusSlave slave;                     // ���������� �������(�������)���������  MODBUS

int ledState = LOW;                    // ���������� ��������� ����������

void flash_time()                      // ��������� ���������� ���������� 
{ 
	//digitalWrite(ledPin, HIGH);      // �������� ���������
	slave.run();                       // ������ ��������� MODBUS 
	//digitalWrite(ledPin, LOW);       // �������� ���������
}



void set_port()
{
	digitalWrite(Pin13, regBank.get(13+1));     // ������� ���������� �����������
	if(digitalRead(Pin8)== HIGH)
	{
	    regBank.set(40008,1);  
	}
	else
	{
       regBank.set(40008,0);  
	}

	regBank.set(40010,analogRead(PinA0));      // �������� ��������� �0      
}


void setup_regModbus()                         // ���������� ��������� ��� �������� ���������� �� MODBUS 
{
    regBank.setId(1);                          // Slave ID 1  ����������   MODBUS �1

	regBank.add(1);                            //  
	regBank.add(6);                            //  
	regBank.add(7);                            //  
	regBank.add(8);                            // ������� ���������� �����������
	regBank.add(9);                            //  
	regBank.add(10);                           //  
	regBank.add(11);                           //  
	regBank.add(12);                           //  
	regBank.add(13);                           //  
 	regBank.add(14);                           // 
	regBank.add(15);                           //  
	regBank.add(40001);                        //   
	regBank.add(40002);                        //   
	regBank.add(40003);                        //   
	regBank.add(40004);                        //  
	regBank.add(40005);                        //   
	regBank.add(40006);                        //   
	regBank.add(40007);                        //   
	regBank.add(40008);                        //   
	regBank.add(40009);                        //  
	regBank.add(40010);                        // ������� ��������� �0 
}


void setup(void)
{
  //  Serial.begin(9600);
	setup_regModbus();
	slave._device = &regBank;  
	slave.setSerial(0,9600);                   // ����������� � ��������� MODBUS ���������� Serial

	pinMode(Pin8, INPUT);                      // ����������  
	digitalWrite(Pin8, HIGH);                  //  

	pinMode(Pin13, OUTPUT);                    // ������������� ����� ������ ������, ��� "�����"
	digitalWrite(Pin13, LOW);                  //  


	MsTimer2::set(500, flash_time);            // 500ms ������ ������� ���������
	MsTimer2::start();                         // �������� ������ ����������
}

void loop(void)
{
	set_port();                                // �������� ���������� ����������� ���������� �� MODBUS
	delay(100);

  /*
    if (ledState == LOW)
	{
      ledState = HIGH;
    } 
	else
	{
      ledState = LOW;
    }

    digitalWrite(Pin13, ledState);*/
}