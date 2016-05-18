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

#include <modbus.h>                     // ���������� ��������� MODBUS 
#include <modbusDevice.h>               // ���������� ��������� MODBUS 
#include <modbusRegBank.h>              // �������� ��������� MODBUS 
#include <modbusSlave.h>                // ���������� �������(�������)���������  MODBUS
#include <SoftwareSerial.h>             // ���������� ��������� �����
#include <TinyGPS.h>                    // ���������� GPS
#include <MsTimer2.h>                   // ���������� �������
#include <SPI.h>
#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <nRF24L01.h>





TinyGPS gps;                            // ��������� GPS

static const int RXPin = 5, TXPin = 4;
static const uint32_t GPSBaud = 9600;   // �������� ������ � ������� GPS



SoftwareSerial ss(RXPin, TXPin);        // ����������� GPS � ������
//SoftwareSerial ss(5, 4);              // ����������� GPS � ������

static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

//+++++++++++++++++ ��������� nRF24L01 ++++++++++++++++++++++++++
// ����� ������
#define ADDR "serv1"
// ������ �������� ��������
#define PAYLOAD sizeof(unsigned long)
// ��������� ��� ��������� - 9 ���
#define StatusLed 9
// ���������� ��� ����� � �������� ������
unsigned long data = 0;
unsigned long command = 0;

//---------------------------------------------------------------

//#define  Pin9        9                  // ���������� 
#define  Pin10       10                 // ���������� 

#define  GazA0       A0                 // ���������� ������ ��� ����������� ������� ���� MQ 
#define  PowerGeiger 6                  // ���������� ������ ��� ���������� �������� �������� �������
#define  InGeiger    2                  // ���������� ����� ����������� �������� ������� 

//+++++++++++++++++++ MODBUS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

modbusDevice regBank;                  // ����������� ��������� 
modbusSlave slave;                     // ���������� �������(�������)���������  MODBUS

int ledState = LOW;                    // ���������� ��������� ����������

void flash_time()                      // ��������� ���������� ���������� 
{ 
	//digitalWrite(ledPin, HIGH);      // �������� ���������
	slave.run();                       // ������ ��������� MODBUS 
	//digitalWrite(ledPin, LOW);       // �������� ���������
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
  if (command != 0) {
    switch (command) {
      case 1:
        // ������� 1 - ��������� ����� ����������,
        // ��������� � ��������� ������������ �����
        Serial.println("Command 1. Send millis().");
        data = 1234567890;
	//	data = millis();
        break;
      case 2:
        // ������� 2 - ��������� �������� � ���� AnalogPin0
        Serial.println("Command 2. Send A0 reference.");
        data = analogRead(A0);
        break;
      case 3:
        // ������� 3 - ��������� �������� � ���� AnalogPin1
        Serial.println("Command 3. Send A1 reference.");
        data = analogRead(A1);
        break;
      default:
        // �������������� �������. ������� ������ ����������� 10 ��� �
        // �������� � ���������������� ����
        Serial.println("Unknown command");
       /* for (byte i = 0; i < 10; i++) {
          digitalWrite(StatusLed, HIGH);
          delay(100);
          digitalWrite(StatusLed, LOW);
        }*/
        break;
    }
    // ���������� �����:

    Mirf.setTADDR((byte *)"clie1");
    //���������� ����� � ���� ������� ����:
    Mirf.send((byte *)&data);
  }
  // ���������������� ����������� ��������.
  // ��������� �������� ������� � �������.
  delay(10);
}



void set_port()
{
	//digitalWrite(Pin13, regBank.get(13+1));     // ������� ���������� �����������
	//if(digitalRead(Pin8)== HIGH)
	//{
	//    regBank.set(40008,1);  
	//}
	//else
	//{
 //      regBank.set(40008,0);  
	//}

	//regBank.set(40010,analogRead(GazA0));      // �������� ��������� �0      
}
void setup_regModbus()                         // ���������� ��������� ��� �������� ���������� �� MODBUS 
{
    regBank.setId(1);                          // Slave ID 1  ����������   MODBUS �1

	regBank.add(1);                            //  
	regBank.add(6);                            //  
	regBank.add(7);                            //  
	regBank.add(8);                            // ������� ���������� �����������
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
	regBank.add(40010);                        // ������� ��������� �0 
}


void setup(void)
{
	//  Serial.begin(9600);
	setup_regModbus();
	slave._device = &regBank;                  // ����������� ��������� � MODBUS 
	slave.setSerial(0,9600);                   // ����������� � ��������� MODBUS ���������� Serial

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
 
	MsTimer2::set(500, flash_time);            // 500ms ������ ������� ����������
	MsTimer2::start();                         // �������� ������ ����������
}

void loop(void)
{
	set_port();                                // �������� ���������� ����������� ���������� �� MODBUS

	run_nRF24L01();
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