

/*
DS18B20 №1  oneWire_in
Черный - GND
Белый - 3
Красный - +5в

DS18B20 №2  oneWire_out
Черный - GND
Белый - 4
Красный - +5в

DS18B20 №3  oneWire_sun
Черный - GND
Белый - 5
Красный - +5в

 Connection: BH1750
 VCC-5v
 GND-GND
 SCL-SCL(analog pin 5)
 SDA-SDA(analog pin 4)
 ADD-NC or GND

*/


#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>
#include <Wire.h>
#include <BH1750.h>
#include <OneWire.h>
#include <MsTimer2.h> 

OneWire  ds_tube(2);                     // on pin 10 (a 4.7K resistor is necessary)
OneWire  ds_in(3);                      // on pin 10 (a 4.7K resistor is necessary)
OneWire  ds_out(4);                     // on pin 10 (a 4.7K resistor is necessary)
OneWire  ds_sun(5);                     // on pin 10 (a 4.7K resistor is necessary)

#define  Pin6   6                       // Назначение  
#define  Pin7   7                       // Назначение  
#define  Pin8   8                       // Назначение  
#define  Pin9   9                       // Назначение  
#define  Pin10  10                      // Назначение  
#define  Pin11  11                      // Назначение  
#define  Pin12  12                      // Назначение  
#define  Pin13  13                      // Светодиод подсоединен к цифровому выводу 13 
#define  PinA0  A0                      // Назначение  
#define  PinA1  A1                      // Назначение  




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
void measure_temp_in()
{
  byte i;
  byte present = 0;
  byte type_s = 0;
  byte data[12];
  byte addr[8];
  float celsius = 0;
 // regBank.set(40001,0); 

  if ( !ds_in.search(addr)) 
  {
    ds_in.reset_search();
    delay(250);
    return;
  }

  ds_in.reset();
  ds_in.select(addr);
  ds_in.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  present = ds_in.reset();
  ds_in.select(addr);    
  ds_in.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++)
  {           // we need 9 bytes
    data[i] = ds_in.read();
  }
  int16_t raw = 0;
  raw = (data[1] << 8) | data[0];
  if (type_s) 
  {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) 
	{
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  else 
  {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  }
  celsius = (float)raw / 16.0*10;

  regBank.set(40001,(int)celsius); 

}
void measure_temp_out()
{
  byte i;
  byte present = 0;
  byte type_s = 0;
  byte data[12];
  byte addr[8];
  float celsius = 0;
 // regBank.set(40002,0); 

  if ( !ds_out.search(addr)) 
  {
    ds_out.reset_search();
    delay(250);
    return;
  }

  ds_out.reset();
  ds_out.select(addr);
  ds_out.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  present = ds_out.reset();
  ds_out.select(addr);    
  ds_out.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++)
  {           // we need 9 bytes
    data[i] = ds_out.read();
  }
  int16_t raw = 0;
  raw = (data[1] << 8) | data[0];
  if (type_s) 
  {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) 
	{
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  else 
  {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  }
  celsius = (float)raw / 16.0*10;

  regBank.set(40002,(int)celsius); 

}
void measure_temp_sun()
{
  byte i;
  byte present = 0;
  byte type_s = 0;
  byte data[12];
  byte addr[8];
  float celsius = 0;
 // regBank.set(40003,0); 
  
  if ( !ds_sun.search(addr)) 
  {
    ds_sun.reset_search();
    delay(250);
    return;
  }

  ds_sun.reset();
  ds_sun.select(addr);
  ds_sun.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  present = ds_sun.reset();
  ds_sun.select(addr);    
  ds_sun.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++)
  {           // we need 9 bytes
    data[i] = ds_sun.read();
  }
  int16_t raw = 0;
  raw = (data[1] << 8) | data[0];
  if (type_s) 
  {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) 
	{
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  else 
  {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  }
  celsius = (float)raw / 16.0*10;

  regBank.set(40003,(int)celsius); 

}
void measure_temp_tube()
{
  byte i;
  byte present = 0;
  byte type_s = 0;
  byte data[12];
  byte addr[8];
  float celsius = 0;
 // regBank.set(40004,0); 
  
  if ( !ds_tube.search(addr)) 
  {
    ds_tube.reset_search();
    delay(250);
    return;
  }

  ds_tube.reset();
  ds_tube.select(addr);
  ds_tube.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  present = ds_tube.reset();
  ds_tube.select(addr);    
  ds_tube.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++)
  {           // we need 9 bytes
    data[i] = ds_tube.read();
  }
  int16_t raw = 0;
  raw = (data[1] << 8) | data[0];
  if (type_s) 
  {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) 
	{
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  else 
  {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  }
  celsius = (float)raw / 16.0*10;

  regBank.set(40004,(int)celsius); 

}
void set_port()
{
	digitalWrite(Pin10, regBank.get(10+1));                   //  
	digitalWrite(Pin11, regBank.get(11+1));                   //  
	digitalWrite(Pin12, regBank.get(12+1));                   //  
	digitalWrite(Pin13, regBank.get(13+1));                   //  
	digitalWrite(PinA1, regBank.get(14+1));                   //  

	if(digitalRead(Pin6)== HIGH)
	{
	    regBank.set(40006,1);  
	}
	else
	{
       regBank.set(40006,0);  
	}

	if(digitalRead(Pin7)== HIGH)
	{
	    regBank.set(40007,1);  
	}
	else
	{
       regBank.set(40007,0);  
	}

	if(digitalRead(Pin8)== HIGH)
	{
	    regBank.set(40008,1);  
	}
	else
	{
       regBank.set(40008,0);  
	}

	if(digitalRead(Pin9)== HIGH)
	{
	    regBank.set(40009,1);  
	}
	else
	{
       regBank.set(40009,0);  
	}

	regBank.set(40010,analogRead(PinA0)); 
}

void measure_light()
{
 	lux = lightMeter.readLightLevel();
	regBank.set(40005, lux);                           //   
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
	lightMeter.begin();

	pinMode(Pin6, INPUT);                                 // Назначение  
	pinMode(Pin7, INPUT);                                 // Назначение  
	pinMode(Pin8, INPUT);                                // Назначение  
	pinMode(Pin9, INPUT);                                // Назначение  
	
	digitalWrite(Pin6, HIGH);                              //  
	digitalWrite(Pin7, HIGH);                              //  
	digitalWrite(Pin8, HIGH);                              //  
	digitalWrite(Pin9, HIGH);                              //  

	pinMode(Pin10, OUTPUT);                                // Назначение  
	pinMode(Pin11, OUTPUT);                                // Назначение  
	pinMode(Pin12, OUTPUT);                                // Назначение  
	pinMode(Pin13, OUTPUT);                              // устанавливаем режим работы вывода, как "выход"
	pinMode(PinA1, OUTPUT);                                // Назначение  

	digitalWrite(Pin10, LOW);                            //  
	digitalWrite(Pin11, LOW);                            //  
	digitalWrite(Pin12, LOW);                            //  
	digitalWrite(Pin13, LOW);                            //  
	digitalWrite(PinA1, LOW);                            //  

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