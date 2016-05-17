/*







*/


#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

#include <MsTimer2.h> 


#define  Pin8   8                       // Назначение  
#define  Pin13  13                      // Светодиод подсоединен к цифровому выводу 13 
#define  PinA0  A0                      // Назначение  


//+++++++++++++++++++ MODBUS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

modbusDevice regBank;
modbusSlave slave;

int ledState = LOW;             // ledState used to set the LED
unsigned int lux = 0;

BH1750 lightMeter;

void flash_time()                                              // Программа обработчик прерывания 
{ 
	//digitalWrite(ledPin, HIGH);   // включаем светодиод
	slave.run();
	//digitalWrite(ledPin, LOW);   // включаем светодиод
}



void set_port()
{
	digitalWrite(Pin13, regBank.get(13+1));                   //  


	if(digitalRead(Pin8)== HIGH)
	{
	    regBank.set(40008,1);  
	}
	else
	{
       regBank.set(40008,0);  
	}

	regBank.set(40010,analogRead(PinA0)); 
}


void setup_regModbus()
{
    regBank.setId(1);        // Slave ID 1

	regBank.add(1);          //  
	regBank.add(6);          //  
	regBank.add(7);          //  
	regBank.add(8);          //  
	regBank.add(9);          //  
	regBank.add(10);         //  
	regBank.add(11);         //  
	regBank.add(12);         //  
	regBank.add(13);         //  
 	regBank.add(14);         // 
	regBank.add(15);         //  
	regBank.add(40001);      //   
	regBank.add(40002);      //   
	regBank.add(40003);      //   
	regBank.add(40004);      //  
	regBank.add(40005);      //   
	regBank.add(40006);      //   
	regBank.add(40007);      //   
	regBank.add(40008);      //   
	regBank.add(40009);      //  
	regBank.add(40010);      //  
}


void setup(void)
{
  //  Serial.begin(9600);
	setup_regModbus();
	slave._device = &regBank;  
	slave.setSerial(0,9600);                              // Подключение к протоколу MODBUS компьютера Serial

	//pinMode(Pin6, INPUT);                                 // Назначение  
	//pinMode(Pin7, INPUT);                                 // Назначение  
	pinMode(Pin8, INPUT);                                 // Назначение  
	//pinMode(Pin9, INPUT);                                 // Назначение  
	//
	//digitalWrite(Pin6, HIGH);                              //  
	//digitalWrite(Pin7, HIGH);                              //  
	digitalWrite(Pin8, HIGH);                              //  
	//digitalWrite(Pin9, HIGH);                              //  

	//pinMode(Pin10, OUTPUT);                                // Назначение  
	//pinMode(Pin11, OUTPUT);                                // Назначение  
	//pinMode(Pin12, OUTPUT);                                // Назначение  
	pinMode(Pin13, OUTPUT);                                // устанавливаем режим работы вывода, как "выход"
	//pinMode(PinA1, OUTPUT);                                // Назначение  

	//digitalWrite(Pin10, LOW);                              //  
	//digitalWrite(Pin11, LOW);                              //  
	//digitalWrite(Pin12, LOW);                              //  
	digitalWrite(Pin13, LOW);                              //  
	//digitalWrite(PinA1, LOW);                              //  

	MsTimer2::set(500, flash_time);                               // 300ms период таймера прерывани
	MsTimer2::start();                                           // Включить таймер прерывания
}

void loop(void)
{
	measure_light();
	delay(100);
	measure_temp_in();
	delay(100);
	measure_temp_out();
	delay(100);
	measure_temp_sun();
	delay(100);
	measure_temp_tube();
	delay(100);
	set_port();
	delay(100);

   // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
	{
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(Pin13, ledState);
}