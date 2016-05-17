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
#include <SoftwareSerial.h>             // Библиотека серийного порта
#include <TinyGPS.h>                    // Библиотека GPS
#include <MsTimer2.h>                   // Библиотеки таймера

TinyGPS gps;                            // Настройка GPS
SoftwareSerial ss(5, 4);                // Подключение GPS к сериал

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

#define  Pin8   8                       // Назначение  
#define  Pin13  13                      // Светодиод подсоединен к цифровому выводу 13 
#define  GazA0  A0                      // Назначение вывода для подключения датчика газа MQ 


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

//+++++++++++++++ Работа с GPS +++++++++++++++++++++++++++++++++++++++++++++++++
static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
static void print_float(float val, float invalid, int len, int prec)
{
  if (val == invalid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartdelay(0);
}
static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}
static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
        month, day, year, hour, minute, second);
    Serial.print(sz);
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
}
static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartdelay(0);
}
//------------------------------------------------------------------------------


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

	regBank.set(40010,analogRead(GazA0));      // Получить состояние А0      
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
	slave._device = &regBank;                  // Подключение регистров к MODBUS 
	slave.setSerial(0,9600);                   // Подключение к протоколу MODBUS компьютера Serial

	ss.begin(9600);                            // Настройка скорости обмена с GPS


	pinMode(Pin8, INPUT);                      // Назначение  
	digitalWrite(Pin8, HIGH);                  //  

	pinMode(Pin13, OUTPUT);                    // устанавливаем режим работы вывода, как "выход"
	digitalWrite(Pin13, LOW);                  //  


	MsTimer2::set(500, flash_time);            // 500ms период таймера прерывания
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