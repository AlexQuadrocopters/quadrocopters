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
  D3 - управление ключем питания газоанализатора
  А0 - аналоговый выход газоанализатора
  А1 - цифровой выход газоанализатора

*/

#include <SoftwareSerial.h>             // Библиотека серийного порта
#include <TinyGPS++.h>
//#include <TinyGPS.h>                    // Библиотека GPS
#include <MsTimer2.h>                   // Библиотеки таймера
#include <SPI.h>
#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <nRF24L01.h>
#include <Wire.h> 
#include <BMP085.h>

// Индикация полета

#define  LedFlyFront A2                 // Светодиод полета передний
#define  LedFlyRear  A3                 // Светодиод полета задний
#define  LedPause    A7                 // Пауза между включением
int TimelyFront    = 1100;              // Время включения светодиода 
int TimelyRear     = 1100;              // Время включения светодиода 
int TimeInterval   = 500;               // Время между включениями светодиодов
bool Front_Start   = false;             // Флаг запуска программы по команде 
bool Rear_Start    = false;             // Флаг запуска программы по команде 
int numled         = 0;


//------ Настройки счетчика Гейгера ------------------

// Conversion factor - CPM to uSV/h
#define CONV_FACTOR 0.00812
int geiger_input          = 2;                    //Назначение ввода подключения счетчика Гейгера
long count                = 0;
long countPerMinute       = 0;
long timePrevious         = 0;
long timePreviousMeassure = 0;
long timeGeiger           = 0;
long countPrevious        = 0;
float radiationValue      = 0.0;
int geiger_ready          = 0;
//---------------------------------------------------

//TinyGPS gps;                                  // Настройка GPS
TinyGPSPlus gps;
static const int RXPin = 5, TXPin = 4;
static const uint32_t GPSBaud = 9600;         // Скорость обмена с модулем GPS
SoftwareSerial ss(RXPin, TXPin);              // Подключение GPS к сериал
//
//static void smartdelay(unsigned long ms);
//static void print_float(float val, float invalid, int len, int prec);
//static void print_int(unsigned long val, unsigned long invalid, int len);
//static void print_date(TinyGPS &gps);
//static void print_str(const char *str, int len);

//
//int year;
//byte month, day, hour, minute, second, hundredths;
//unsigned long age_t;
//
//float flat                 = 0, flon = 0;
//unsigned long age, date, time, chars = 0;
//unsigned short sentences             = 0, failed = 0;
//static const double LONDON_LAT       = 51.508131, LONDON_LON = -0.128002;
//
//
//float DOM_LAT                        = 55.954994;
//float DOM_LON                        = 37.231121;
//int gps_satellites                   = 0;
//float gps_lat                        = 0;
//float gps_lon                        = 0;
//int gps_dist                         = 0;
unsigned long last        = 0UL;             // For stats that happen every 5 seconds

int cpm                     = 0;               // Счетчик Гейгера               
float uSv_h                 = 0;               // Счетчик Гейгера 
int temp_C                  = 0;               // Температура С 
int gaz_measure             = 0;               // Величина измеренной загазованности
int gaz_porog               = 0;               // Уровень порога газа
int P_mmHq                  = 0;               // Давление
int distance                = 0;               // Дистанция до объекта
int altitudeP               = 0;               // Высота по давлению
int f_altitude              = 0;               // Высота по GPS
int altitudeDom             = 0;               // Высота местности
int f_course                = 0;               // Направление на объект
int speed_kmph              = 0;               // Скорость движения
double gps_location_lat     = 0.0;             // Координата фактическая
double gps_location_lng     = 0.0;             // Координата фактическая
double DOM_LAT              = 55.954994;       // Координата домашняя
double DOM_LON              = 37.231121;       // Координата домашняя
float data_f                = 0;
int gound_m                 = 218;             // Высота местности над уровнем моря
double distanceToDOM        = 0;               // Расстояние до объекта
double courseToDOM          = 0;               // Направление на объект
int gps_date_value          = 0;
int gps_date_year           = 0;
int gps_date_month          = 0;
int gps_date_day            = 0;
int gps_time_value          = 0;
int gps_time_hour           = 0;
int gps_time_minute         = 0;
int gps_time_second         = 0;
int gps_time_centisecond    = 0;
int gps_speed_value         = 0;
int gps_speed_knots         = 0;
int gps_speed_mph           = 0;
int gps_speed_mps           = 0;
int gps_speed_kmph          = 0;
int gps_course_value        = 0;
int gps_course_deg          = 0;
int gps_altitude_value      = 0;
int gps_altitude_meters     = 0;
int gps_altitude_miles      = 0;
int gps_altitude_kilometers = 0;
int gps_altitude_feet       = 0;
int gps_satellites_value    = 0;               // Количество спутников
int gps_hdop_value          = 0;

unsigned long currentMillisGPS       = 0;              // Переменная для временного хранения текущего времени
unsigned long currentMillis          = 0;              // Переменная для временного хранения текущего времени
unsigned long timeGPS                = 2200;           //  
bool ButGPS_Start                    = false;          // Флаг запуска программы по команде 



//+++++++++++++++++ Настройки nRF24L01 ++++++++++++++++++++++++++

#define ADDR "fly10"                                   // Адрес модуля
#define PAYLOAD sizeof(unsigned long)                  // Размер полезной нагрузки
#define StatusLed 10                                   // Светодиод для индикации - 10 пин
unsigned long data = 0;                                // Переменная для приёма и передачи данных
unsigned int command = 0;                              //
unsigned long timePreviousRF24L01    = 0;   
unsigned long currentMillisnRF24L01  = 0;              // Переменная для временного хранения текущего времени
unsigned int time_nRF24L01           = 100;            //  
bool nRF24L01_Start                  = false;          // Флаг запуска программы по команде 


//---------------------------------------------------------------

#define  Power_gaz   3                                 // Назначение вывода для управления питанием датчика газа MQ2 
#define  GazA0       A0                                // Назначение вывода для подключения датчика газа MQ2, аналоговый выход газоанализатора
#define  GazA1       A1                                // Назначение вывода для подключения датчика газа MQ2, цифровой выход газоанализатора
#define  PowerGeiger 6                                 // Назначение вывода для управления питанием счетчика Гейгера
bool st_Power_gaz    = false;
bool st_PowerGeiger  = false;
bool old_Power_gaz   = false;
bool old_PowerGeiger = false;

int ledState = LOW;                                    // Переменная состояния светодиода


//+++++++++++++++ Работа с GPS +++++++++++++++++++++++++++++++++++++++++++++++++
 /*----------------------------------------------------------*/
  //Sats - количество найденных спутников
  //HDOP - горизонтальная точность
  //Latitude - широта в градусах
  //Longitude - долгота в градусах
  //Date - дата
  //Time - времы UTC(-4 часа от Московского)
  //Alt - высота над уровнем моря (в метрах)
  //Course - путевой угол (направление скорости) в градусах. Значение 0 - север, 90 — восток, 180 — юг, 270 — запад.
  //Speed - скорость(км/ч)
  /*----------------------------------------------------------*/
/*

/*      // Последняя версия
void UpdateGPS()  
{
  bool newData = false;

 if (currentMillis - currentMillisGPS >= timeGPS)
  {
	currentMillisGPS = millis();
	while (ss.available())
	{
	  char c = ss.read();
	  if (gps.encode(c)) // Did a new valid sentence come in?
		newData = true;
	}
  }

  if (newData)
  {
	gps.f_get_position(&flat, &flon, &age);
	//Serial.print("LAT=");
	//Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
	//Serial.print(" LON=");
	//Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
	//Serial.print(" SAT=");
	//Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
	//Serial.print(" PREC=");
	//Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
  }
}
*/

void UpdateGPS() 
{
 if (currentMillis - currentMillisGPS >= timeGPS)
  {
		currentMillisGPS = millis();
		while (ss.available() > 0)
		gps.encode(ss.read());

		if (gps.location.isUpdated())
		{
			//Serial.print(F("LOCATION   Fix Age="));
			//Serial.print(gps.location.age());
			//Serial.print(F("ms Raw Lat="));
			//Serial.print(gps.location.rawLat().negative ? "-" : "+");
			//Serial.print(gps.location.rawLat().deg);
			//Serial.print("[+");
			//Serial.print(gps.location.rawLat().billionths);
			//Serial.print(F(" billionths],  Raw Long="));
			//Serial.print(gps.location.rawLng().negative ? "-" : "+");
			//Serial.print(gps.location.rawLng().deg);
			//Serial.print("[+");
			//Serial.print(gps.location.rawLng().billionths);
			//Serial.print(F(" billionths],  Lat="));
			//Serial.print(gps.location.lat(), 6);
			//Serial.print(F(" Long="));
			//Serial.println(gps.location.lng(), 6);
			gps_location_lat = gps.location.lat();             // Координата фактическая
			gps_location_lng = gps.location.lng();             // Координата фактическая
		}

		else if (gps.date.isUpdated())
		{
			//Serial.print(F("DATE       Fix Age="));
			//Serial.print(gps.date.age());
			//Serial.print(F("ms Raw="));
			//Serial.println(gps.date.value());
			//Serial.print(F(" Year="));
			//Serial.print(gps.date.year());
			//Serial.print(F(" Month="));
			//Serial.print(gps.date.month());
			//Serial.print(F(" Day="));
			//Serial.println(gps.date.day());
			gps_date_value  = gps.date.value();
			gps_date_year   = gps.date.year();
			gps_date_month  = gps.date.month();
			gps_date_day    = gps.date.day();
		}

		else if (gps.time.isUpdated())
		{
			//Serial.print(F("TIME       Fix Age="));
			//Serial.print(gps.time.age());
			//Serial.print(F("ms Raw="));
			//Serial.println(gps.time.value());
			//Serial.print(F(" Hour="));
			//Serial.print(gps.time.hour());
			//Serial.print(F(" Minute="));
			//Serial.print(gps.time.minute());
			//Serial.print(F(" Second="));
			//Serial.print(gps.time.second());
			//Serial.print(F(" Hundredths="));
			//Serial.println(gps.time.centisecond());
			gps_time_value        = gps.time.value();
			gps_time_hour         = gps.time.hour();
			gps_time_minute       = gps.time.minute();
			gps_time_second       = gps.time.second();
		}

		else if (gps.speed.isUpdated())
		{
			//Serial.print(F("SPEED      Fix Age="));
			//Serial.print(gps.speed.age());
			//Serial.print(F("ms Raw="));
			//Serial.println(gps.speed.value());
			//Serial.print(F(" Knots="));
			//Serial.print(gps.speed.knots());
			//Serial.print(F(" MPH="));
			//Serial.print(gps.speed.mph());
			//Serial.print(F(" m/s="));
			//Serial.print(gps.speed.mps());
			//Serial.print(F(" km/h="));
			//Serial.println(gps.speed.kmph());
			gps_speed_value = gps.speed.value();
			//gps_speed_knots = gps.speed.knots();
			gps_speed_mph   = gps.speed.mph();
			gps_speed_mps   = gps.speed.mps();
			gps_speed_kmph  = gps.speed.kmph();
		}

		else if (gps.course.isUpdated())
		{
			//Serial.print(F("COURSE     Fix Age="));
			//Serial.print(gps.course.age());
			//Serial.print(F("ms Raw="));
			//Serial.println(gps.course.value());
			//Serial.print(F(" Deg="));
			//Serial.println(gps.course.deg());
			gps_course_value = gps.course.value();
			gps_course_deg   = gps.course.deg();
		}

		else if (gps.altitude.isUpdated())
		{
			//Serial.print(F("ALTITUDE   Fix Age="));
			//Serial.print(gps.altitude.age());
			//Serial.print(F("ms Raw="));
			//Serial.println(gps.altitude.value());
			//Serial.print(F(" Meters="));
			//Serial.print(gps.altitude.meters());
			//Serial.print(F(" Miles="));
			//Serial.print(gps.altitude.miles());
			//Serial.print(F(" KM="));
			//Serial.print(gps.altitude.kilometers());
			//Serial.print(F(" Feet="));
			//Serial.println(gps.altitude.feet()); 
			 gps_altitude_value      = gps.altitude.value();
			 gps_altitude_meters     = gps.altitude.meters();
			 gps_altitude_miles      = gps.altitude.miles();
			 gps_altitude_kilometers = gps.altitude.kilometers();
			 gps_altitude_feet       = gps.altitude.feet();

		}

		else if (gps.satellites.isUpdated())
		{
			//Serial.print(F("SATELLITES Fix Age="));
			//Serial.print(gps.satellites.age());
			//Serial.print(F("ms Value="));
			gps_satellites_value = gps.satellites.value();
		}

		else if (gps.hdop.isUpdated())
		{
			//Serial.print(F("HDOP       Fix Age="));
			//Serial.print(gps.hdop.age());
			//Serial.print(F("ms Value="));
			//Serial.println(gps.hdop.value());
			gps_hdop_value = gps.hdop.value();
		}

		else if (millis() - last > 5000)
		{
		///	Serial.println();
			if (gps.location.isValid())
			{
				distanceToDOM = 
				TinyGPSPlus::distanceBetween(
					gps.location.lat(),
					gps.location.lng(),
					DOM_LAT, 
					DOM_LAT);
				courseToDOM =
				TinyGPSPlus::courseTo(
					gps.location.lat(),
					gps.location.lng(),
					DOM_LAT, 
					DOM_LAT);

				//Serial.print(F("LONDON     Distance="));
				//Serial.print(distanceToLondon/1000, 6);
				//Serial.print(F(" km Course-to="));
				//Serial.print(courseToLondon, 6);
				//Serial.print(F(" degrees ["));
				//Serial.print(TinyGPSPlus::cardinal(courseToLondon));
				//Serial.println(F("]"));


			}

			//Serial.print(F("DIAGS      Chars="));
			//Serial.print(gps.charsProcessed());
			//Serial.print(F(" Sentences-with-Fix="));
			//Serial.print(gps.sentencesWithFix());
			//Serial.print(F(" Failed-checksum="));
			//Serial.print(gps.failedChecksum());
			//Serial.print(F(" Passed-checksum="));
			//Serial.println(gps.passedChecksum());

			if (gps.charsProcessed() < 10)
			{
			   Serial.println(F("WARNING: No GPS data.  Check wiring."));
			}
			last = millis();
			//Serial.println();
		}
  }
}

//------------------------------------------------------------------------------
//+++++++++++++++ Работа с датчиком давления +++++++++++++++++++++++++++++++++++++++++++++++++
BMP085 dps = BMP085();    
long Temperature = 0, Pressure = 0, Altitude = 0;
int bmp_real_alt = 0;                  // Реальная высота
int bmp_gnd      = 219;                // Высота местности над уровнем моря
//-----------------------------------------------------------------------------------------

void power_on_off()
{
  if(old_Power_gaz != st_Power_gaz )
  {
	old_Power_gaz = st_Power_gaz;
	if(st_Power_gaz)
	{
	   digitalWrite(Power_gaz, LOW);          // Включить анализатор газа
	}
	else
	{
	   digitalWrite(Power_gaz, HIGH);         // Выключить анализатор газа
	}

  }
  if(old_PowerGeiger != st_PowerGeiger)
  {
	old_PowerGeiger = st_PowerGeiger;

	if(st_PowerGeiger)
	{
		digitalWrite(PowerGeiger,LOW);         // Включить счетчик Гейгера
	}
	else
	{
		digitalWrite(PowerGeiger,HIGH);        // Выключить счетчик Гейгера
	}
  }
}


void run_nRF24L01()
{
  // Обнуляем переменную с данными:
  data = 0;
  command = 0;
  // Ждём данных
  if (!Mirf.isSending() && Mirf.dataReady()) 
  {
	 // Принимаем пакет данные в виде массива байт в переменную data:
	Mirf.getData((byte *) &command);
	delay(300);
  }
  // Если переменная не нулевая, формируем ответ:geiger_ready = 1;
  if (command != 0)
  {
	switch (command)

	{
		case 1:
			data = geiger_ready;                      // Флаг готовности Счетчика Гейгера
			break;
		case 2:
			// команда 2 - отправить значение
			data = countPerMinute;                   // "cpm = "
			break;
		case 3:
			data = radiationValue * 10000 ;          // "uSv/h = "
			geiger_ready = 0;                        // Показания Счетчика Гейгера отправлены
			break;
		case 4:
			if(st_PowerGeiger == true)             // Состоянеи ключа включения питания счетчика Гейгера
				{
					data = 2;
				}
			else
				{
					data = 1;
				}
			break; 
		case 5:
			data = analogRead(A0);                    // Анализатор Газа
			break;
		case 6:                                       // Состоянеи ключа включения питания датчика газа
			if(st_Power_gaz == true)
				{
					data = 2;
				}
			else
				{
					data = 1;
				}
			break;
		case 7:
			dps.getTemperature(&Temperature); 
			data = Temperature;                      // Паказания температуры от BMP085
			break;
		case 8:
			dps.getPressure(&Pressure); 
			data = Pressure/133.3;                   // Показания давления от BMP085
			break;  
		case 9:
			dps.getAltitude(&Altitude); 
			data =Altitude/100;                      // Показания Высота от датчика давления BMP085
			break;
		case 10:
		   data = gps_altitude_meters;                // Высота в метрах со спутника
			break;
		case 11:                                     // 
			data = gps_location_lat*1000000;         // Передать  координаты LAT
			break;
		case 12:
			data = gps_location_lng*1000000;         // Передать  координаты LON
			break;
		case 13:
			data = DOM_LAT*1000000;                 // Передать местные координаты DOM_LAT
			break;
		case 14:
			data = DOM_LON*1000000;                 // Передать местные координаты DOM_LON
			break;
		case 15:                                   
		    data = gps_satellites_value;            // Количество спутников  
			break;
		case 16:
		    data = distanceToDOM;                   // Расстояние до объекта
			break;
		case 17:
		    data = gps_course_deg;                  // Направление на объект
			break;
		case 18:
		    data =  gps_speed_mph;                  // Скорость объекта метров в час
			break;
		case 19:             
			st_Power_gaz = true;                   // Включить питание датчика газа
			data = 2;
			break;
		case 20:
			st_Power_gaz = false;                  // Отключить питание датчика газа
			data = 1;
			break;
		case 21:
			st_PowerGeiger = true;                 // Включить питание датчика Гейгера
			data = 2;
			break;
		case 22:
			st_PowerGeiger = false;                // Отключить питание датчика Гейгера
			data = 1;
			break;
		case 23:                                   // Зафиксировать местные координаты
			DOM_LAT = gps_location_lat;
			DOM_LON = gps_location_lng;
			data = 1;
			break;
		case 24:                                  
			data = gps_date_value;
			break;
		case 25:
			data = gps_date_year;
			break;                                  
		case 26:
			data = gps_date_month;
			break;
		case 27:
			data = gps_date_day;
			break;
		case 28:                                  
			data = gps_time_value;
			break;
		case 29:
			data = gps_time_hour;
			break;                                  
		case 30:
			data = gps_time_minute;
			break;
		case 31:
			data = gps_time_second;
			break;
		case 32:                                  
			data = gps_speed_value;
			break;
		case 33:
			data = gps_speed_mph;
			break;                                  
		case 34:
			data = gps_speed_mps;
			break;
		case 35:
			data = gps_speed_kmph;
			break;
		case 36:                                  
			data = gps_course_value;
			break;
		case 37:
			data = gps_altitude_value;
			break;                                  
		case 38:
			data = gps_altitude_kilometers;
			break;
		case 39:
			data = gps_altitude_miles;
			break;
		case 40:
			data = gps_altitude_feet;
			break;

//	  case 1:
//		data = analogRead(A0);                     // Анализатор Газа
//		break;
//	  case 2:
//		// команда 2 - отправить значение
//		data = geiger_ready;                      // Флаг готовности Счетчика Гейгера
//		break;
//	  case 3:
//		  // команда 3 - отправить значение
//		Serial.println("cpm = ");
//		data = countPerMinute;  
//		break;
//	 case 4:
//		// команда 4 - отправить значение
//		Serial.println("uSv/h = ");
//		data = radiationValue * 10000 ;
//		geiger_ready = 0;                        // Показания Счетчика Гейгера отправлены
//		break;
//	case 5:
//		dps.getTemperature(&Temperature); 
//		data = Temperature;                      // Паказания температуры от BMP085
//		break;
//	case 6:
//		dps.getPressure(&Pressure); 
//		data = Pressure/133.3;                   // Показания давления от BMP085
//		break;  
//	case 7:
////		gps.f_get_position(&flat, &flon, &age); 
//		//data = flat*1000000;
//		break;
//	case 8:
//		//data = flon*1000000;
//		break;
//	case 9:
//		dps.getAltitude(&Altitude); 
//		data =Altitude/100;                    // Показания Высота от датчика давления BMP085
//		break;
//	case 10:
//	//	 data = (flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, DOM_LAT, DOM_LON), 0xFFFFFFFF, 9);  // Показания Дистанция м. =      
//		break;
//	case 11:
//		//gps_satellites = gps.satellites();
////		data = gps_satellites;                 // Количество спутников
//		break;
//	case 12:
//		st_Power_gaz = true;
//		data = 2;
//		break;
//	case 13:
//		st_Power_gaz = false;
//		data = 1;
//		break;
//	case 14:                                  // Состоянеи ключа включения питания датчика газа
//		if(st_Power_gaz == true)
//			{
//				data = 2;
//			}
//		else
//			{
//				data = 1;
//			}
//		break;
//	case 15:
//		st_PowerGeiger = true;
//		data = 2;
//		break;
//	case 16:
//		st_PowerGeiger = false;
//		data = 1;
//		break;
//	case 17:
//		if(st_PowerGeiger == true)             // Состоянеи ключа включения питания счетчика Гейгера
//			{
//				data = 2;
//			}
//		else
//			{
//				data = 1;
//			}
//		break; 
//	case 18:                                   // Зафиксировать местные координаты
//		//DOM_LAT = flat;
//		//DOM_LON = flon;
//		data = 1;
//		break;
//	case 19:                                   // Передать местные координаты DOM_LAT
//		data = DOM_LAT*1000000;
//		break;
//	case 20:
//		data = DOM_LON*1000000;                // Передать местные координаты DOM_LON
//		break;
//	case 21:
//	  //  data = (flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, DOM_LAT, DOM_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
//		break;
//	case 22:
//		//data = (gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);  // Высота GPS
//	    break;
//	case 23:
//		//data = (flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
//	    break;
//	case 24:
//		//data = (gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
//	    break;
//	case 25:
//        break;

	  default:
		// Нераспознанная команда.
		// жалуемся в последовательный порт
		Serial.println("Unknown command");
		break;
	}
	// Отправляем ответ:
	Mirf.setTADDR((byte *)"remot");
	Mirf.send((byte *)&data);                         //Отправляем ответ в виде массива байт:
  }
  delay(20);
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

// Программа управления светодиодами


class Flasher                                      // Управление светодиодами в многозадачном режиме
{
	int ledPin;
	long OnTime;
	long OffTime;

	int ledState;
	unsigned long previousMillis;
  public:
	Flasher(int pin,  long on, long off)
	{
	  ledPin = pin;
	  pinMode(ledPin, OUTPUT);

	  OnTime = on;
	  OffTime = off;

	  ledState = LOW;
	  previousMillis = 0;
	}

	void Update()
	{
	  unsigned long currentMillis = millis();

	  if ((ledState == HIGH) && (currentMillis - previousMillis >= OnTime))
	  {
		ledState = LOW;
		previousMillis = currentMillis;
		digitalWrite(ledPin, ledState);
		numled++;
		if(numled>3)
		{
			numled=0;
		}
	  }
	  else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime))
	  {
		ledState = HIGH;
		previousMillis = currentMillis;
		digitalWrite(ledPin, ledState);
	  }
	}
};

 Flasher led1(LedFlyFront, TimelyFront, TimeInterval);
 Flasher led2(LedFlyRear,  TimelyRear,  TimeInterval);
 Flasher Pause1(LedPause,  TimeInterval,  TimeInterval);
 Flasher Pause2(LedPause,  TimeInterval,  TimeInterval);


void UpdatenRF24L01()
{
  if (currentMillis - timePreviousRF24L01 > time_nRF24L01)
  {
	timePreviousRF24L01 = millis();
	run_nRF24L01();
  }
}

void UpdateLed()
{
  if(numled == 0)
  {
	led1.Update();
  }
  if(numled == 1)
  {
	Pause1.Update();
  }

  if(numled == 2)
  {
	led2.Update();
  }
  if(numled == 3)
  {
	Pause2.Update();
  }
}


void setup(void)
{
  Serial.begin(9600);

	pinMode(Power_gaz,OUTPUT);
	pinMode(PowerGeiger,OUTPUT);
	pinMode(StatusLed,OUTPUT);
	digitalWrite(Power_gaz,HIGH);
	digitalWrite(PowerGeiger,HIGH);
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
	Serial.println("Beginning ... ");
	attachInterrupt(0, countPulse, FALLING);
}

void loop(void)
{
  currentMillis=millis();
  run_geiger();
  UpdateGPS();
  UpdatenRF24L01();
  power_on_off();
  UpdateLed();
}
