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
������ NRF24L01 �����������
+++++++++++++++++++++++++++++++++++++++



        Arduino Nano     NRF24L01
  	        GND           1 GND
          VCC +3,3V       2 VCC +3,3V
			 D8           3 CE
			 D7			  4 SCN
		SCK  D13		  5 SCK
		MOSI D11 		  6 MOSI
		MISO D12		  7 MISO
						  8 IRQ


*/

#include <SoftwareSerial.h>             // ���������� ��������� �����
#include <TinyGPS.h>                    // ���������� GPS
#include <MsTimer2.h>                   // ���������� �������
#include <SPI.h>
#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <nRF24L01.h>


//------ ��������� �������� ������� ------------------

// Conversion factor - CPM to uSV/h
#define CONV_FACTOR 0.00812
int geiger_input = 2;                         //���������� ����� ����������� �������� ������� 
long count = 0;
long countPerMinute = 0;
long timePrevious = 0;
long timePreviousMeassure = 0;
long time = 0;
long countPrevious = 0;
float radiationValue = 0.0;

//---------------------------------------------------

TinyGPS gps;                                  // ��������� GPS

static const int RXPin = 5, TXPin = 4;
static const uint32_t GPSBaud = 9600;         // �������� ������ � ������� GPS

SoftwareSerial ss(RXPin, TXPin);              // ����������� GPS � ������
//SoftwareSerial ss(5, 4);                    // ����������� GPS � ������

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

//+++++++++++++++++ ��������� nRF24L01 ++++++++++++++++++++++++++

#define ADDR "gelicopter"                          // ����� ������
#define PAYLOAD sizeof(unsigned long)         // ������ �������� ��������
#define StatusLed 9                           // ��������� ��� ��������� - 9 ���
unsigned long data = 0;                       // ���������� ��� ����� � �������� ������
unsigned long command = 0;                    //

//---------------------------------------------------------------

#define  Pin10       10                       // ���������� 
#define  GazA0       A0                       // ���������� ������ ��� ����������� ������� ���� MQ 
#define  PowerGeiger 6                        // ���������� ������ ��� ���������� �������� �������� �������

int ledState = LOW;                           // ���������� ��������� ����������

void flash_time()                             // ��������� ���������� ���������� 
{ 
	//digitalWrite(ledPin, HIGH);             // �������� ���������
	//slave.run();                            // ������ ��������� MODBUS 
	//digitalWrite(ledPin, LOW);              // �������� ���������
}

//+++++++++++++++ ������ � GPS +++++++++++++++++++++++++++++++++++++++++++++++++
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
void run_nRF24L01()
{
 // �������� ���������� � �������:
  data = 0;
  command = 0;
  // ��� ������
  if (!Mirf.isSending() && Mirf.dataReady()) {
    Serial.println("Got packet");
    //�������� �������� �������� ���������� � ������� ������
    digitalWrite(StatusLed, HIGH);
    delay(100);
    digitalWrite(StatusLed, LOW);
    // ��������� ����� ������ � ���� ������� ���� � ���������� data:
    Mirf.getData((byte *) &command);
    // �������� ������� �������� ���������� � ��������� ������
    digitalWrite(StatusLed, HIGH);
    delay(500);
    digitalWrite(StatusLed, LOW);
    // ������� ���������� ������ � ������� ��������� �����
    Serial.print("Get data: ");
    Serial.println(command);
  }
  // ���� ���������� �� �������, ��������� �����:
  if (command != 0) 
  {
    switch (command) 
	{
      case 1:
        // ������� 1 - ��������� ����� ����������,
        // ��������� � ��������� ������������ �����
        Serial.println("Command 1. Send millis().");
		data = millis();
        break;
      case 2:
        // ������� 2 - ��������� �������� 
        Serial.println("cpm = ");
        data = countPerMinute;
        break;
      case 3:
        // ������� 3 - ��������� �������� 
        Serial.println("uSv/h = ");
 		data = radiationValue*10000 ;
        break;
      default:
        // �������������� �������. ������� ������ ����������� 10 ��� � 
        // �������� � ���������������� ����
        Serial.println("Unknown command");
        for (byte i = 0; i < 10; i++) {
          digitalWrite(StatusLed, HIGH);
          delay(100);
          digitalWrite(StatusLed, LOW);
        }
        break;
    }
    // ���������� �����:

    Mirf.setTADDR((byte *)"remote");
    //���������� ����� � ���� ������� ����:
    Mirf.send((byte *)&data);
  }
  // ���������������� ����������� ��������.
  // ��������� �������� ������� � �������.
  delay(10);
}

void run_geiger()
{
  if (millis()-timePreviousMeassure > 10000)
  {
    countPerMinute = 6*count;
    radiationValue = countPerMinute * CONV_FACTOR;
    timePreviousMeassure = millis();
    Serial.print("cpm = "); 
    Serial.print(countPerMinute,DEC);
    Serial.print(" - ");
    Serial.print("uSv/h = ");
    Serial.println(radiationValue,4);      
    count = 0;
  }
}

void countPulse()
{
  detachInterrupt(0);
  count++;
  while(digitalRead(2)==0)
  {
  }
  attachInterrupt(0,countPulse,FALLING);
}


void setup(void)
{
	Serial.begin(9600);

	pinMode(geiger_input, INPUT);
    digitalWrite(geiger_input,HIGH);

	ss.begin(GPSBaud);                         // ��������� �������� ������ � GPS

	Mirf.cePin = 8;
	Mirf.csnPin = 7;
	Mirf.spi = &MirfHardwareSpi;
	MirfHardwareSpi;
	Mirf.init();

	Mirf.setRADDR((byte*)ADDR);
	Mirf.payload = sizeof(unsigned long);
	Mirf.config();
	Serial.println("Beginning ... ");
 
//	MsTimer2::set(500, flash_time);            // 500ms ������ ������� ����������
//	MsTimer2::start();                         // �������� ������ ����������
	
    attachInterrupt(0,countPulse,FALLING);
}

void loop(void)
{
	delay(100);
	run_geiger();
	run_nRF24L01();
}