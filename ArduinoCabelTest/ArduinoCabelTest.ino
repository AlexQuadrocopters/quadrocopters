#include <avr/io.h>
#include "Wire.h"
#include <UTFT.h>
#include <UTouch.h>
#include <UTFT_Buttons.h>
#include <RTClib.h>
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <MsTimer2.h> 
#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>
#include "MCP23017.h"
#include <avr/pgmspace.h>

MCP23017 mcp_Out1;                                 // ���������� ������ ���������� MCP23017  4 A - Out, B - Out
MCP23017 mcp_Out2;                                 // ���������� ������ ���������� MCP23017  6 A - Out, B - Out

#define  ledPin13  13                              // ���������� ����������� �� �����
#define  ledPin12  12                              // ���������� ����������� �� �����

//+++++++++++++++++++ MODBUS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

modbusDevice regBank;
//Create the modbus slave protocol handler
modbusSlave slave;

//+++++++++++++++++++++++ ��������� ������������ ��������� +++++++++++++++++++++++++++++++++++++
#define address_AD5252   0x2F                      // ����� ���������� AD5252  
#define control_word1    0x07                      // ���� ���������� �������� �1
#define control_word2    0x87                      // ���� ���������� �������� �2
byte resistance        = 0x00;                     // ������������� 0x00..0xFF - 0��..100���


//+++++++++++++++++++++++++++++ ������� ������ +++++++++++++++++++++++++++++++++++++++
int deviceaddress        = 80;                     // ����� ���������� ������
unsigned int eeaddress   =  0;                     // ����� ������ ������
byte hi;                                           // ������� ���� ��� �������������� �����
byte low;                                          // ������� ���� ��� �������������� �����


void setup_resistor()
{ 
	Wire.beginTransmission(address_AD5252);        // transmit to device
	Wire.write(byte(control_word1));               // sends instruction byte  
	Wire.write(0);                                 // sends potentiometer value byte  
	Wire.endTransmission();                        // stop transmitting
	Wire.beginTransmission(address_AD5252);        // transmit to device
	Wire.write(byte(control_word2));               // sends instruction byte  
	Wire.write(0);                                 // sends potentiometer value byte  
	Wire.endTransmission();                        // stop transmitting
}

void setup_mcp()
{
	// ��������� ����������� ������
 
  mcp_Out1.begin(1);                               //  ����� (4) �������  ����������� ������
  mcp_Out1.pinMode(0, OUTPUT);                     //  1A1
  mcp_Out1.pinMode(1, OUTPUT);                     //  1B1  
  mcp_Out1.pinMode(2, OUTPUT);                     //  1C1
  mcp_Out1.pinMode(3, OUTPUT);                     //  1D1  
  mcp_Out1.pinMode(4, OUTPUT);                     //  1A2
  mcp_Out1.pinMode(5, OUTPUT);                     //  1B2
  mcp_Out1.pinMode(6, OUTPUT);                     //  1C2
  mcp_Out1.pinMode(7, OUTPUT);                     //  1D2
  
  mcp_Out1.pinMode(8, OUTPUT);                     //  1E1   U13
  mcp_Out1.pinMode(9, OUTPUT);                     //  1E2   U17
  mcp_Out1.pinMode(10, OUTPUT);                    //  1E3   U23
  mcp_Out1.pinMode(11, OUTPUT);                    //  1E4   U14
  mcp_Out1.pinMode(12, OUTPUT);                    //  1E5   U19
  mcp_Out1.pinMode(13, OUTPUT);                    //  1E6   U21 
  mcp_Out1.pinMode(14, OUTPUT);                    //  1E7   ��������  
  mcp_Out1.pinMode(15, OUTPUT);                    //  1E8   ��������

	
  mcp_Out2.begin(2);                               //  
  mcp_Out2.pinMode(0, OUTPUT);                     //  2A1  
  mcp_Out2.pinMode(1, OUTPUT);                     //  2B1  
  mcp_Out2.pinMode(2, OUTPUT);                     //  2C1
  mcp_Out2.pinMode(3, OUTPUT);                     //  2D1  
  mcp_Out2.pinMode(4, OUTPUT);                     //  2A2
  mcp_Out2.pinMode(5, OUTPUT);                     //  2B2
  mcp_Out2.pinMode(6, OUTPUT);                     //  2C2
  mcp_Out2.pinMode(7, OUTPUT);                     //  2D2
  
  mcp_Out2.pinMode(8, OUTPUT);                     //  2E1   U15
  mcp_Out2.pinMode(9, OUTPUT);                     //  2E2   U18
  mcp_Out2.pinMode(10, OUTPUT);                    //  2E3   U22
  mcp_Out2.pinMode(11, OUTPUT);                    //  2E4   U16
  mcp_Out2.pinMode(12, OUTPUT);                    //  2E5   U20     
  mcp_Out2.pinMode(13, OUTPUT);                    //  2E6   U24       
  mcp_Out2.pinMode(14, OUTPUT);                    //  2E7   ���� �1, �2
  mcp_Out2.pinMode(15, OUTPUT);                    //  2E8   ��������
  for(int i=0;i<16;i++)
  {
	  mcp_Out1.digitalWrite(i, LOW); 
	  mcp_Out2.digitalWrite(i, LOW); 
  }
}



void setup()
{
	Serial.begin(9600);                            // ����������� � USB ��
	Serial1.begin(115200);                         // ����������� � ��������� ������ 
	slave.setSerial(3,57600);                      // ����������� � ��������� MODBUS ���������� Serial3 
	Serial2.begin(115200);                         // 
	Serial.println(" ");
	Serial.println(" ***** Start system  *****");
	Serial.println(" ");
	Wire.begin();
	setup_mcp();                                   // ��������� ����� ����������  
	digitalWrite(ledPin12, LOW);                   // ��������� �������������� ���������
	digitalWrite(ledPin13, LOW);                   // ��������� �������������� ���������



	digitalWrite(ledPin13, HIGH);                  // ��������� �������������� ���������
	Serial.println(" ");                           //
	Serial.println("System initialization OK!.");  // ���������� � ���������� ���������
}

void loop()
{

  /* add main program code here */

}
