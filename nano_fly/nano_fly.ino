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
#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <nRF24L01.h>


//------ Настройки счетчика Гейгера ------------------

// Conversion factor - CPM to uSV/h
#define CONV_FACTOR 0.00812
int geiger_input          = 2;                         //Назначение ввода подключения счетчика Гейгера
long count                = 0;
long countPerMinute       = 0;
long timePrevious         = 0;
long timePreviousMeassure = 0;
long time                 = 0;
long countPrevious        = 0;
float radiationValue      = 0.0;
int geiger_ready          = 0;
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

#define ADDR "fly10"                          // Адрес модуля
#define PAYLOAD sizeof(unsigned long)         // Размер полезной нагрузки
#define StatusLed 9                           // Светодиод для индикации - 9 пин
unsigned long data = 0;                       // Переменная для приёма и передачи данных
unsigned long command = 0;                    //

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
  // Обнуляем переменную с данными:
  data = 0;
  command = 0;
  // Ждём данных
  if (!Mirf.isSending() && Mirf.dataReady()) 
  {
    Serial.println("Got packet");
    //Сообщаем коротким миганием светодиода о наличии данных
    digitalWrite(StatusLed, HIGH);
    delay(100);
    digitalWrite(StatusLed, LOW);
    // Принимаем пакет данные в виде массива байт в переменную data:
    Mirf.getData((byte *) &command);
    // Сообщаем длинным миганием светодиода о получении данных
    digitalWrite(StatusLed, HIGH);
    delay(500);
    digitalWrite(StatusLed, LOW);
    // Выводим полученные данные в монитор серийного порта
    Serial.print("Get data: ");
    Serial.println(command);
  }
  // Если переменная не нулевая, формируем ответ:geiger_ready = 1;
  if (command != 0)
  {
    switch (command)
    {
      case 1:
        // Команда 1 - отправить число милисекунд,
        // прошедших с последней перезагрузки платы
        Serial.println("Command 1. Send millis().");
        data = millis();
        break;
      case 2:
        // команда 2 - отправить значение
        data = geiger_ready;
        break;
      case 3:
          // команда 2 - отправить значение
        Serial.println("cpm = ");
        data = countPerMinute;
        break;

     case 4:
        // команда 3 - отправить значение
        Serial.println("uSv/h = ");
        data = radiationValue * 10000 ;
		geiger_ready = 0;
        break;
	case 5:
 
        break;
	case 6:
 
        break;
	case 7:
 
        break;
	case 8:
 
        break;
	case 9:
 
        break;
	case 10:
 
        break;

	case 11:
 
        break;



      default:
        // Нераспознанная команда. Сердито мигаем светодиодом 10 раз и
        // жалуемся в последовательный порт
        Serial.println("Unknown command");
        for (byte i = 0; i < 10; i++) 
		{
       /*   digitalWrite(StatusLed, HIGH);
          delay(100);
          digitalWrite(StatusLed, LOW);*/
        }
        break;
    }
    // Отправляем ответ:
    Mirf.setTADDR((byte *)"remot");
    Mirf.send((byte *)&data);                         //Отправляем ответ в виде массива байт:
  }
  // Экспериментально вычисленная задержка.
  // Позволяет избежать проблем с модулем.
  delay(10);
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
	geiger_ready = 1;
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

  Mirf.cePin = 8;
  Mirf.csnPin = 7;
  Mirf.spi = &MirfHardwareSpi;
  MirfHardwareSpi;
  Mirf.init();

  Mirf.setRADDR((byte*)ADDR);
  Mirf.payload = sizeof(unsigned long);
  Mirf.config();
  Serial.println("Beginning ... ");

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
