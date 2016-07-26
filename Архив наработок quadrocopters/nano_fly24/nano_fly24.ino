/*
  Базовый модуль Arduino Nano
  Размещается на квадрокоптере.
  Установленное оборудование:
  +++++++++++++++++++++++++++++++++++++++
  GPS модуль GY-GPS6MV2 (NEO-6M-0-001)
  ++++++++++++++++++++++++++++++++++++++
  Подключение:
 Arduino Nano    GY-GPS6MV2
   VCC +5V          VCC
	 GND            GND
     D4             RX
	 D5             TX
  ---------------------------------------
  Модуль NRF24L01 Подключение
  +++++++++++++++++++++++++++++++++++++++
 Arduino Nano     NRF24L01
    GND           1 GND
  VCC +3,3V       2 VCC +3,3V
  	 D8           3 CE
     D7		      4 SCN
SCK  D13	      5 SCK
MOSI D11 	      6 MOSI
MISO D12	      7 MISO
  ------------------------------------------
  Счетчик Гейгера
  ++++++++++++++++++++++++++++++++++++++++++
Arduino Nano     Счетчик Гейгера
    GND              GND
 VCC +5,0V        VCC +5,0V
	D2       Выход коллектора транзистора
	D6       Управление питанием (реле)
  ------------------------------------------------
  Датчик давления  BMP180(BMO085)   (измерение высоты)
  ++++++++++++++++++++++++++++++++++++++++++++++++++++
Arduino Nano      BMP180(BMO085) 
    GND               GND  
 VCC +5,0V        VCC +5,0V
    A4               SDA
    A5               SCL

  ---------------------------------------------------


*/

#include <SoftwareSerial.h>             // Библиотека серийного порта
#include <TinyGPS.h>                    // Библиотека GPS
#include <MsTimer2.h>                   // Библиотеки таймера
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//------ Настройки счетчика Гейгера ------------------

// Conversion factor - CPM to uSV/h
#define CONV_FACTOR 0.00812
int geiger_input = 2;                         //Назначение ввода подключения счетчика Гейгера
long count = 0;
long countPerMinute = 0;
long timePrevious = 0;
long timePreviousMeassure = 0;
long time = 0;
long countPrevious = 0;
float radiationValue = 0.0;

//---------------------------------------------------

TinyGPS gps;                                  // Настройка GPS

static const int RXPin = 5, TXPin = 4;
static const uint32_t GPSBaud = 9600;         // Скорость обмена с модулем GPS

SoftwareSerial ss(RXPin, TXPin);              // Подключение GPS к сериал
//SoftwareSerial ss(5, 4);                    // Подключение GPS к сериал

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

//+++++++++++++++++ Настройки nRF24L01 ++++++++++++++++++++++++++
RF24 radio(7,8);
//RF24 radio(8,7);
const uint64_t pipe = 0xE8E8F0F0E1LL;
uint32_t message;  // Эта переменная для сбора обратного сообщения от приемника;

//---------------------------------------------------------------

#define  Pin10       10                       // Назначение 
#define  GazA0       A0                       // Назначение вывода для подключения датчика газа MQ 
#define  PowerGeiger 6                        // Назначение вывода для управления питанием счетчика Гейгера

int ledState = LOW;                           // Переменная состояния светодиода

void flash_time()                             // Программа обработчик прерывания
{
  //digitalWrite(ledPin, HIGH);             // включаем светодиод
  //slave.run();                            // Запрос протокола MODBUS
  //digitalWrite(ledPin, LOW);              // включаем светодиод
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
    for (int i = flen; i < len; ++i)
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
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
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
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartdelay(0);
}
//------------------------------------------------------------------------------
void run_nRF24L01()
{
    uint32_t message = 111;  //Вот какой потенциальной длины сообщение - uint32_t!
	//туда можно затолкать значение температуры от датчика или еще что-то полезное.

	radio.writeAckPayload( 1, &message, sizeof(message) ); // Грузим сообщение для автоотправки;
	if ( radio.available() ) 
	{ //Просто читаем и очищаем буфер - при подтверждении приема
		int dataIn;                       //передатчику приемник протолкнет ему в обратку наше сообщение;
		bool done = false;
		while (!done) 
		{
			radio.read(&dataIn, sizeof(dataIn));    // Значение dataIn в данном случае
			//done = radio.read(&dataIn, sizeof(dataIn));    // Значение dataIn в данном случае
			//не важно. Но его можно использовать и как управляющую команду.
		}
	}
}

void run_geiger()
{
  if (millis() - timePreviousMeassure > 10000)
  {
    countPerMinute = 6 * count;
    radiationValue = countPerMinute * CONV_FACTOR;
    timePreviousMeassure = millis();
    Serial.print("cpm = ");
    Serial.print(countPerMinute, DEC);
    Serial.print(" - ");
    Serial.print("uSv/h = ");
    Serial.println(radiationValue, 4);
    count = 0;
  }
}

void countPulse()
{
  detachInterrupt(0);
  count++;
  while (digitalRead(2) == 0){}
  attachInterrupt(0, countPulse, FALLING);
}


void setup(void)
{
  Serial.begin(9600);

  pinMode(geiger_input, INPUT);
  digitalWrite(geiger_input, HIGH);

  ss.begin(GPSBaud);                         // Настройка скорости обмена с GPS


	radio.begin();                           // Старт работы;
	radio.enableAckPayload();                // Разрешение отправки нетипового ответа передатчику;
	radio.openReadingPipe(1,pipe);           // Открываем трубу и
	radio.startListening();                  //начинаем слушать;


  //Mirf.cePin = 8;
  //Mirf.csnPin = 7;
  //Mirf.spi = &MirfHardwareSpi;
  //MirfHardwareSpi;
  //Mirf.init();

  //Mirf.setRADDR((byte*)ADDR);
  //Mirf.payload = sizeof(unsigned long);
  //Mirf.config();
  //Serial.println("Beginning ... ");

  //	MsTimer2::set(500, flash_time);            // 500ms период таймера прерывания
  //	MsTimer2::start();                         // Включить таймер прерывания

  attachInterrupt(0, countPulse, FALLING);
}

void loop(void)
{
  delay(100);
  run_geiger();
  run_nRF24L01();
}
