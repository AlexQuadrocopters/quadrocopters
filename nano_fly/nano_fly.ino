/*







*/


#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

#include <MsTimer2.h> 


#define  Pin8   8                       // ����������  
#define  Pin13  13                      // ��������� ����������� � ��������� ������ 13 
#define  PinA0  A0                      // ����������  


//+++++++++++++++++++ MODBUS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

modbusDevice regBank;
modbusSlave slave;

int ledState = LOW;             // ledState used to set the LED
unsigned int lux = 0;

BH1750 lightMeter;

void flash_time()                                              // ��������� ���������� ���������� 
{ 
	//digitalWrite(ledPin, HIGH);   // �������� ���������
	slave.run();
	//digitalWrite(ledPin, LOW);   // �������� ���������
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
	slave.setSerial(0,9600);                              // ����������� � ��������� MODBUS ���������� Serial

	//pinMode(Pin6, INPUT);                                 // ����������  
	//pinMode(Pin7, INPUT);                                 // ����������  
	pinMode(Pin8, INPUT);                                 // ����������  
	//pinMode(Pin9, INPUT);                                 // ����������  
	//
	//digitalWrite(Pin6, HIGH);                              //  
	//digitalWrite(Pin7, HIGH);                              //  
	digitalWrite(Pin8, HIGH);                              //  
	//digitalWrite(Pin9, HIGH);                              //  

	//pinMode(Pin10, OUTPUT);                                // ����������  
	//pinMode(Pin11, OUTPUT);                                // ����������  
	//pinMode(Pin12, OUTPUT);                                // ����������  
	pinMode(Pin13, OUTPUT);                                // ������������� ����� ������ ������, ��� "�����"
	//pinMode(PinA1, OUTPUT);                                // ����������  

	//digitalWrite(Pin10, LOW);                              //  
	//digitalWrite(Pin11, LOW);                              //  
	//digitalWrite(Pin12, LOW);                              //  
	digitalWrite(Pin13, LOW);                              //  
	//digitalWrite(PinA1, LOW);                              //  

	MsTimer2::set(500, flash_time);                               // 300ms ������ ������� ���������
	MsTimer2::start();                                           // �������� ������ ����������
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