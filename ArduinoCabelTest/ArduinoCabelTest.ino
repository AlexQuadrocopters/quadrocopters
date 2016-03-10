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

MCP23017 mcp_Out1;                            // ���������� ������ ���������� MCP23017  4 A - Out, B - Out
MCP23017 mcp_Out2;                            // ���������� ������ ���������� MCP23017  6 A - Out, B - Out


//+++++++++++++++++++ MODBUS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

modbusDevice regBank;
//Create the modbus slave protocol handler
modbusSlave slave;

//+++++++++++++++++++++++ ��������� ������������ ��������� +++++++++++++++++++++++++++++++++++++
#define address_AD5252   0x2F                       // ����� ���������� AD5252  
#define control_word1    0x07                       // ���� ���������� �������� �1
#define control_word2    0x87                       // ���� ���������� �������� �2
byte resistance        = 0x00;                      // ������������� 0x00..0xFF - 0��..100���


//+++++++++++++++++++++++++++++ ������� ������ +++++++++++++++++++++++++++++++++++++++
int deviceaddress        = 80;                      // ����� ���������� ������
unsigned int eeaddress   =  0;                      // ����� ������ ������
byte hi;                                            // ������� ���� ��� �������������� �����
byte low;                                           // ������� ���� ��� �������������� �����


void setup_resistor()
{ 
	Wire.beginTransmission(address_AD5252);      // transmit to device
	Wire.write(byte(control_word1));             // sends instruction byte  
	Wire.write(0);                               // sends potentiometer value byte  
	Wire.endTransmission();                      // stop transmitting
	Wire.beginTransmission(address_AD5252);      // transmit to device
	Wire.write(byte(control_word2));             // sends instruction byte  
	Wire.write(0);                               // sends potentiometer value byte  
	Wire.endTransmission();                      // stop transmitting
}

void setup_mcp()
{
	// ��������� ����������� ������
 
  mcp_Out1.begin(4);              //  ����� (4) �������  ����������� ������
  mcp_Out1.pinMode(0, OUTPUT);    //  
  mcp_Out1.pinMode(1, OUTPUT);    //     
  mcp_Out1.pinMode(2, OUTPUT);    // 
  mcp_Out1.pinMode(3, OUTPUT);    //    
  mcp_Out1.pinMode(4, OUTPUT);    // 
  mcp_Out1.pinMode(5, OUTPUT);    // 
  mcp_Out1.pinMode(6, OUTPUT);    //  
  mcp_Out1.pinMode(7, OUTPUT);    // 
  
  mcp_Out1.pinMode(8, OUTPUT);    // 
  mcp_Out1.pinMode(9, OUTPUT);    //    
  mcp_Out1.pinMode(10, OUTPUT);   // 
  mcp_Out1.pinMode(11, OUTPUT);   //   
  mcp_Out1.pinMode(12, OUTPUT);   //    
  mcp_Out1.pinMode(13, OUTPUT);   //       
  mcp_Out1.pinMode(14, OUTPUT);   //       
  mcp_Out1.pinMode(15, OUTPUT);   //      

	
  mcp_Out2.begin(6);              //  
  mcp_Out2.pinMode(0, OUTPUT);    // J8-12    XP7 4 PTT2    
  mcp_Out2.pinMode(1, OUTPUT);    // XP1 - 20  HandUp    
  mcp_Out2.pinMode(2, OUTPUT);    // J8-11    XP7 2 sensor
  mcp_Out2.pinMode(3, OUTPUT);    // J8-23    XP7 1 PTT1    
  mcp_Out2.pinMode(4, OUTPUT);    // XP2-2    sensor "���."    
  mcp_Out2.pinMode(5, OUTPUT);    // XP5-3    sensor "��C." 
  mcp_Out2.pinMode(6, OUTPUT);    // XP3-3    sensor "��-�����1."
  mcp_Out2.pinMode(7, OUTPUT);    // XP4-3    sensor "��-�����2."
  
  mcp_Out2.pinMode(8, OUTPUT);    // XP1- 19 HaSs
  mcp_Out2.pinMode(9, OUTPUT);    // XP1- 17 HaSPTT
  mcp_Out2.pinMode(10, OUTPUT);   // XP1- 16 HeS2Rs
  mcp_Out2.pinMode(11, OUTPUT);   // XP1- 15 HeS2PTT
  mcp_Out2.pinMode(12, OUTPUT);   // XP1- 13 HeS2Ls           
  mcp_Out2.pinMode(13, OUTPUT);   // XP1- 6  HeS1PTT            
  mcp_Out2.pinMode(14, OUTPUT);   // XP1- 5  HeS1Rs            
  mcp_Out2.pinMode(15, OUTPUT);   // XP1- 1  HeS1Ls          
}



void setup()
{
	Wire.begin();
//	setup_mcp();                                      // ��������� ����� ����������  
	Serial.begin(9600);                               // ����������� � USB ��
	Serial1.begin(115200);                            // ����������� � ��������� ������ 
	slave.setSerial(3,57600);                         // ����������� � ��������� MODBUS ���������� Serial3 
	Serial2.begin(115200);                            // 
	Serial.println(" ");
	Serial.println(" ***** Start system  *****");
	Serial.println(" ");




	Serial.println(" ");                                         //
	Serial.println("System initialization OK!.");                // ���������� � ���������� ���������
}

void loop()
{

  /* add main program code here */

}
