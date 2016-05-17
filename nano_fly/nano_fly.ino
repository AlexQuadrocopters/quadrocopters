/*
Базовый модуль Arduino Nano
Размещается на квадрокоптере.
Установленное оборудование:
+++++++++++++++++++++++++++++++++++++++
GPS модуль GY-GPS6MV2 (NEO-6M-0-001)
Подключение:
    Arduino Nano    GY-GPS6MV2
	VCC +5V          VCC
	  GND            GND
      D4             RX
	  D5             TX
---------------------------------------








*/


#include <modbus.h>                     // Библиотеки протокола MODBUS 
#include <modbusDevice.h>               // Назначение устройств MODBUS 
#include <modbusRegBank.h>              // Регистры протокола MODBUS 
#include <modbusSlave.h>                // Назначение функции(ведомый)устройств  MODBUS

#include <MsTimer2.h>                   // Библиотеки таймера


#define  Pin8   8                       // Назначение  
#define  Pin13  13                      // Светодиод подсоединен к цифровому выводу 13 
#define  PinA0  A0                      // Назначение  


//+++++++++++++++++++ MODBUS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

modbusDevice regBank;                  // Определение регистров 
modbusSlave slave;                     // Назначение функции(ведомый)устройств  MODBUS

int ledState = LOW;                    // Переменная состояния светодиода

void flash_time()                      // Программа обработчик прерывания 
{ 
	//digitalWrite(ledPin, HIGH);      // включаем светодиод
	slave.run();                       // Запрос протокола MODBUS 
	//digitalWrite(ledPin, LOW);       // включаем светодиод
}



void set_port()
{
	digitalWrite(Pin13, regBank.get(13+1));     // функция управления светодиодом
	if(digitalRead(Pin8)== HIGH)
	{
	    regBank.set(40008,1);  
	}
	else
	{
       regBank.set(40008,0);  
	}

	regBank.set(40010,analogRead(PinA0));      // Получить состояние А0      
}


void setup_regModbus()                         // Назначение регистров для передачи информации по MODBUS 
{
    regBank.setId(1);                          // Slave ID 1  Устройство   MODBUS №1

	regBank.add(1);                            //  
	regBank.add(6);                            //  
	regBank.add(7);                            //  
	regBank.add(8);                            // Регистр управления светодиодом
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
	regBank.add(40010);                        // Регистр состояния А0 
}


void setup(void)
{
  //  Serial.begin(9600);
	setup_regModbus();
	slave._device = &regBank;  
	slave.setSerial(0,9600);                   // Подключение к протоколу MODBUS компьютера Serial

	pinMode(Pin8, INPUT);                      // Назначение  
	digitalWrite(Pin8, HIGH);                  //  

	pinMode(Pin13, OUTPUT);                    // устанавливаем режим работы вывода, как "выход"
	digitalWrite(Pin13, LOW);                  //  


	MsTimer2::set(500, flash_time);            // 500ms период таймера прерывани
	MsTimer2::start();                         // Включить таймер прерывания
}

void loop(void)
{
	set_port();                                // Передать информацию устройствам полученную по MODBUS
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