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
#include <Wire.h> 
#include <BMP085.h>

//------ Настройки счетчика Гейгера ------------------

// Conversion factor - CPM to uSV/h
#define CONV_FACTOR 0.00812
int geiger_input          = 2;                         //Назначение ввода подключения счетчика Гейгера
long count                = 0;
long countPerMinute       = 0;
long timePrevious         = 0;
long timePreviousMeassure = 0;
long timeGeiger           = 0;
long countPrevious        = 0;
float radiationValue      = 0.0;
int geiger_ready          = 0;
//---------------------------------------------------

TinyGPS gps;                                  // Настройка GPS

static const int RXPin = 5, TXPin = 4;
static const uint32_t GPSBaud = 9600;         // Скорость обмена с модулем GPS

 int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age_t;


SoftwareSerial ss(RXPin, TXPin);              // Подключение GPS к сериал
//SoftwareSerial ss(5, 4);                    // Подключение GPS к сериал

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

float flat, flon;
unsigned long age, date, time, chars = 0;
unsigned short sentences             = 0, failed = 0;
static const double LONDON_LAT       = 51.508131, LONDON_LON = -0.128002;
static const double DOM_LAT          = 55.954994, DOM_LON = 37.231121;
int gps_satellites                   = 0;
float gps_lat                        = 0;
float gps_lon                        = 0;
int gps_dist                         = 0;

//+++++++++++++++++ Настройки nRF24L01 ++++++++++++++++++++++++++

#define ADDR "fly10"                          // Адрес модуля
#define PAYLOAD sizeof(unsigned long)         // Размер полезной нагрузки
#define StatusLed 10                          // Светодиод для индикации - 10 пин
unsigned long data = 0;                       // Переменная для приёма и передачи данных
unsigned long command = 0;                    //

//---------------------------------------------------------------

#define  Power_gaz   7                        // Назначение вывода для управления питанием датчика газа MQ2 
#define  GazA0       A0                       // Назначение вывода для подключения датчика газа MQ2 
#define  PowerGeiger 6                        // Назначение вывода для управления питанием счетчика Гейгера

int ledState = LOW;                           // Переменная состояния светодиода

void flash_time()                             // Программа обработчик прерывания
{
  //digitalWrite(ledPin, HIGH);             // включаем светодиод
  //slave.run();                            // Запрос протокола MODBUS
  //digitalWrite(ledPin, LOW);              // включаем светодиод
}

//+++++++++++++++ Работа с GPS +++++++++++++++++++++++++++++++++++++++++++++++++
void run_GPS()
{
  float flat, flon;
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
  
  print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  print_date(gps);
  print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);
  print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
  print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
  print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, LONDON_LAT, LONDON_LON) / 1000, 0xFFFFFFFF, 9);
  print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);

  gps.stats(&chars, &sentences, &failed);
  print_int(chars, 0xFFFFFFFF, 6);
  print_int(sentences, 0xFFFFFFFF, 10);
  print_int(failed, 0xFFFFFFFF, 9);
  Serial.println();
  gps_satellites = gps.satellites();
  Serial.println( gps_satellites);
  gps.f_get_position(&flat, &flon, &age);
  float sat_lat = flat;
  Serial.println(sat_lat,6);
      
  smartdelay(1000);
}

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
 /* int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;*/
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age_t);
  if (age_t == TinyGPS::GPS_INVALID_AGE)
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
//+++++++++++++++ Работа с датчиком давления +++++++++++++++++++++++++++++++++++++++++++++++++
BMP085 dps = BMP085();    
long Temperature = 0, Pressure = 0, Altitude = 0;
int bmp_real_alt = 0;             // Реальная высота
int bmp_gnd      = 219;             // Высота местности над уровнем моря
//-----------------------------------------------------------------------------------------
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
        data = analogRead(A0);
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
		dps.getTemperature(&Temperature);
		data = Temperature;
         //digitalWrite(Power_gaz,HIGH);
        break;
	case 6:
		dps.getPressure(&Pressure); 
		data = Pressure/133.3;
         //digitalWrite(Power_gaz,LOW);
        break;
	case 7:
		 //digitalWrite(PowerGeiger,HIGH);
		gps.f_get_position(&flat, &flon, &age);;
		data = flat*1000000;
		//data = DOM_LAT*1000000;
        break;
	case 8:
		gps.f_get_position(&flat, &flon, &age);;
		data = flon*1000000;
       // data = gps_lon*1000000;
		//data = DOM_LON*1000000;
        break;
	case 9:
        dps.getAltitude(&Altitude); 
		data =Altitude/100;// Высота
		Serial.print("  Alt(m):"); 
        Serial.print(Altitude/100); 
        break;
	case 10:
         data = random(100,1100);    //Дистанция м. =      
        break;
	case 11:
        gps_satellites = gps.satellites();
	    data = gps_satellites;
        break;
	case 12:
 
        break;
	case 13:
 
        break;
	case 14:
 
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

	pinMode(Power_gaz,OUTPUT);
	pinMode(PowerGeiger,OUTPUT);
	pinMode(StatusLed,OUTPUT);
	digitalWrite(Power_gaz,LOW);
	digitalWrite(PowerGeiger,LOW);
	digitalWrite(StatusLed,LOW);


	pinMode(geiger_input, INPUT);
	digitalWrite(geiger_input, HIGH);

	ss.begin(GPSBaud);                         // Настройка скорости обмена с GPS

	Mirf.cePin = 8;
	Mirf.csnPin = 9;
	Mirf.spi = &MirfHardwareSpi;
	MirfHardwareSpi;
	Mirf.init();

	Mirf.setRADDR((byte*)ADDR);
	Mirf.payload = sizeof(unsigned long);
	Mirf.config();
	Wire.begin(); 
	delay(1000);

 // uncomment for different initialization settings
  //dps.init();     // QFE (Field Elevation above ground level) is set to 0 meters.
                  // same as init(MODE_STANDARD, 0, true);
  
 // dps.init(MODE_STANDARD, 101850, false);  // 101850Pa = 1018.50hPa, false = using Pa units
                  // this initialization is useful for normalizing pressure to specific datum.
                  // OR setting current local hPa information from a weather station/local airport (QNH).
  
  dps.init(MODE_ULTRA_HIGHRES, bmp_gnd, true);  // 220 meters, true = using meter units
                  // this initialization is useful if current altitude is known,
                  // pressure will be calculated based on TruePressure and known altitude.

  // note: use zeroCal only after initialization.
  // dps.zeroCal(101800, 0);    // set zero point
	//dps.init(MODE_ULTRA_HIGHRES, bmp_gnd, true);  // 220 meters, true = using meter units

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
