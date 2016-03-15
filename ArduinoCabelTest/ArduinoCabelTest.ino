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

//********************* ��������� �������� ***********************************

// Standard Arduino Mega/Due shield            : <display model>,38,39,40,41
UTFT          myGLCD(ITDB32S,38,39,40,41);
// Standard Arduino Mega/Due shield            : 6,5,4,3,2
UTouch        myTouch(6,5,4,3,2);
// Finally we set up UTFT_Buttons :)
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

boolean default_colors = true;
uint8_t menu_redraw_required = 0;

// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t Dingbats1_XL[];
extern uint8_t SmallSymbolFont[];


//+++++++++++++++++++++++++++ ��������� ����� +++++++++++++++++++++++++++++++
uint8_t second = 0;                      //Initialization time
uint8_t minute = 10;
uint8_t hour   = 10;
uint8_t dow    = 2;
uint8_t day    = 15;
uint8_t month  = 3;
uint16_t year  = 16;
RTC_DS1307 RTC;                         // define the Real Time Clock object

int clockCenterX     = 119;
int clockCenterY     = 119;
int oldsec=0;
const char* str[]          = {"MON","TUE","WED","THU","FRI","SAT","SUN"};
const char* str_mon[]      = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
//------------------------------------------------------------------------------




const unsigned int adr_control_command    PROGMEM       = 40001; // ����� �������� ������� �� ���������� 
const unsigned int adr_reg_count_err      PROGMEM       = 40002; // ����� �������� ���� ������
//-------------------------------------------------------------------------------------------------------
//+++++++++++++++++++++++++++ ����� ���������� ������ Arduino Nano +++++++++++++++++++++++++++++++

#define  kn1Nano   A0                                            // ���������� ������ ���������� Nano  A0
#define  kn2Nano   A1                                            // ���������� ������ ���������� Nano  A1
#define  kn3Nano   A2                                            // ���������� ������ ���������� Nano  A2
#define  kn4Nano   A3                                            // ���������� ������ ���������� Nano  A3

#define  kn5Nano   A4                                            // ���������� ������ ���������� Nano  A4
#define  kn6Nano   A5                                            // ���������� ������ ���������� Nano  A5


//-------------------------------------------------------------------------------------------------------
//���������� ���������� ��� �������� � ����� ���� (������)
int but1, but2, but3, but4, but5, but6, but7, but8, but9, but10, butX, butY, butA, butB, butC, butD, but_m1, but_m2, but_m3, but_m4, but_m5, pressed_button;
 //int kbut1, kbut2, kbut3, kbut4, kbut5, kbut6, kbut7, kbut8, kbut9, kbut0, kbut_save,kbut_clear, kbut_exit;
 //int kbutA, kbutB, kbutC, kbutD, kbutE, kbutF;
 int m2 = 1; // ���������� ������ ����

 //------------------------------------------------------------------------------------------------------------------
 // ���������� ���������� ��� �������� �������

 char  txt_menu1_1[] = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N1";                   // ���� ������ N 1
 char  txt_menu1_2[] = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N2";                   // ���� ������ N 2
 char  txt_menu1_3[] = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N3";                   // ���� ������ N 3
 char  txt_menu1_4[] = "Tec\xA4 ""\x9F""a\x96""e\xA0\xAF N4";                   // ���� ������ N 4

 char  txt_menu2_1[] = "menu2_1";                                                // 
 char  txt_menu2_2[] = "menu2_2";                                                //
 char  txt_menu2_3[] = "menu2_3";                                                //
 char  txt_menu2_4[] = "menu2_4";                                                //

 char  txt_menu3_1[] = "Ta""\x96\xA0\x9D\xA6""a coe""\x99"".";                   // ������� ����.
 char  txt_menu3_2[] = "Pe""\x99""a""\x9F\xA4"". ""\xA4""a""\x96\xA0\x9D\xA6";   // ������. ������
 char  txt_menu3_3[] = "\x85""a""\x98""py""\x9C"". y""\xA1""o""\xA0\xA7"".";     // ������. �����.
 char  txt_menu3_4[] = "Me""\xA2\xAE"" 3.4";                                     //

 char  txt_menu4_1[] = "C\x9D\xA2yco\x9D\x99""a";                                // ���������
 char  txt_menu4_2[] = "Tpey\x98o\xA0\xAC\xA2\xAB\x9E";                          // �����������
 char  txt_menu4_3[] = "\x89\x9D\xA0oo\x96pa\x9C\xA2\xAB\x9E";                   // ������������
 char  txt_menu4_4[] = "\x89p\xAF\xA1oy\x98o\xA0\xAC\xA2\xAB\x9E";               // �������������

 char  txt_menu5_1[] = " ";// 
 char  txt_menu5_2[] = " ";//
 char  txt_menu5_3[] = " ";// 
 char  txt_menu5_4[] = " ";// 

 char  txt_pass_ok[] = "Tec\xA4 Pass!"; // ���� Pass!
 char  txt_pass_no[] = "Tec\xA4 NO!"; // ���� NO!

 
 char  txt_info1[] = "Tec\xA4 ""\x9F""a\x96""e\xA0""e\x9E";                     // ���� �������
 char  txt_info2[] = "Tec\xA4 \x96\xA0o\x9F""a \x98""ap\xA2\x9D\xA4yp";         // ���� ����� ��������
 char  txt_info3[] = "Hac\xA4po\x9E\x9F""a c\x9D""c\xA4""e\xA1\xAB";            // ��������� �������
 char  txt_info4[] = "\x81""e\xA2""epa\xA4op c\x9D\x98\xA2""a\xA0o\x97";        // ��������� ��������
 char  txt_info5[] = "Oc\xA6\x9D\xA0\xA0o\x98pa\xA5";                           // �����������

 int   temp_buffer[40] ;                                                        // ����� �������� ��������� ����������
 const unsigned int connektN1_default[]    PROGMEM  = { 
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,                                                     // ������ �
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20                                                      // ������ B
 }; // 20 x 2 �����
 const unsigned int connektN2_default[]    PROGMEM  = { 
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,                                   // ������ �
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26                                    // ������ B
 }; // 26 x 2 �����
 const unsigned int connektN3_default[]    PROGMEM  = { 
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,  // ������ �
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37   // ������ B
 }; // 37 x 2 �����
 const unsigned int connektN4_default[]    PROGMEM  = { 
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,           // ������ �
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34            // ������ B
 }; // 34 x 2 �����

 //++++++++++++++++++ ������� � 1 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 const unsigned int adr_memN1_1      PROGMEM       =    100;                      // ��������� ����� ������ ������� ������������ ��������� �������� �1�, �1�
 const unsigned int adr_memN2_1      PROGMEM       =    141;                      // ��������� ����� ������ ������� ������������ ��������� �������� �2�, �2�
 const unsigned int adr_memN3_1      PROGMEM       =    194;                      // ��������� ����� ������ ������� ������������ ��������� �������� �3�, �3�
 const unsigned int adr_memN4_1      PROGMEM       =    269;                      // ��������� ����� ������ ������� ������������ ��������� �������� �4�, �4�
 //++++++++++++++++++ ������� � 2 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 const unsigned int adr_memN1_2      PROGMEM       =    300;                      // ��������� ����� ������ ������� ������������ ��������� �������� �1�, �1�
 const unsigned int adr_memN2_2      PROGMEM       =    341;                      // ��������� ����� ������ ������� ������������ ��������� �������� �2�, �2�
 const unsigned int adr_memN3_2      PROGMEM       =    394;                      // ��������� ����� ������ ������� ������������ ��������� �������� �3�, �3�
 const unsigned int adr_memN4_2      PROGMEM       =    469;                      // ��������� ����� ������ ������� ������������ ��������� �������� �4�, �4�

 void dateTime(uint16_t* date, uint16_t* time)                  // ��������� ������ ������� � ���� �����
{
  DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

void serial_print_date()                           // ������ ���� � �������    
{
	  DateTime now = RTC.now();
	  Serial.print(now.day(), DEC);
	  Serial.print('/');
	  Serial.print(now.month(), DEC);
	  Serial.print('/');
	  Serial.print(now.year(), DEC);//Serial display time
	  Serial.print(' ');
	  Serial.print(now.hour(), DEC);
	  Serial.print(':');
	  Serial.print(now.minute(), DEC);
	  Serial.print(':');
	  Serial.print(now.second(), DEC);
}
void set_time()
{
	RTC.adjust(DateTime(__DATE__, __TIME__));
	DateTime now = RTC.now();
	second = now.second();       //Initialization time
	minute = now.minute();
	hour   = now.hour();
	day    = now.day();
	day++;
	if(day > 31)day = 1;
	month  = now.month();
	year   = now.year();
	DateTime set_time = DateTime(year, month, day, hour, minute, second); // ������� ������ � ������� � ������ "set_time"
	RTC.adjust(set_time);             
}
void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data )
{
	int rdata = data;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.write(rdata);
	Wire.endTransmission();
	delay(10);
}
byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
	byte rdata = 0xFF;
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress,1);
	if (Wire.available()) rdata = Wire.read();
	return rdata;
}
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length )
{
	
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddress >> 8)); // MSB
	Wire.write((int)(eeaddress & 0xFF)); // LSB
	Wire.endTransmission();
	Wire.requestFrom(deviceaddress,length);
	int c = 0;
	for ( c = 0; c < length; c++ )
	if (Wire.available()) buffer[c] = Wire.read();
	
}
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) 
{
	
	Wire.beginTransmission(deviceaddress);
	Wire.write((int)(eeaddresspage >> 8)); // MSB
	Wire.write((int)(eeaddresspage & 0xFF)); // LSB
	byte c;
	for ( c = 0; c < length; c++)
	Wire.write(data[c]);
	Wire.endTransmission();
	
}


void flash_time()                                              // ��������� ���������� ���������� 
{ 
	// PORTB = B00000000; // ��� 12 ��������� � ��������� LOW
	slave.run();
	// PORTB = B01000000; // ��� 12 ��������� � ��������� HIGH
}
void serialEvent3()
{
	control_command();
}

void control_command()
{
	/*
	��� ������ ������������ �������� ���������� �������� ����� �������� �� ������ adr_control_command (40120) 
	��� ��������
	0 -  ���������� ������� ��������
	1 -   
	2 -   
	3 -   
	4 -   
	5 -   
	6 -   
	7 -   
	8 -   
	9 -   
	10 -  ���������� ������� ������� ���������� �1
	11 -  ���������� ������� ������� ���������� �2
	12 -  
	13 -  
	14 -  
	15 -  
	16 -                   
	17 -  
	18 -  
	19 -  
	20 -  
	21 -  
	22 -  
	23 -  
	24 - 
	25 - 
	26 - 
	27 - 
	28 - 
	29 - 
	30 - 

	*/


	int test_n = regBank.get(adr_control_command);   //�����  40000
	if (test_n != 0)
	{
		if(test_n != 0) Serial.println(test_n);	
		switch (test_n)
		{
			case 1:
				 test_cabel_N1();             // 
				 break;
			case 2:	
				 test_cabel_N2();             //
				 break;
			case 3:
				 test_cabel_N3();             //
				 break;
			case 4:	
				 test_cabel_N4();             //
				 break;
			case 5:
				 test_panel_N1();             //
				 break;
			case 6:	
				 //
				 break;
			case 7:
				 //
				 break;
			case 8:	
				 //
				 break;
			case 9:
				 //
				 break;
			case 10:
				 set_rezistor1();                   // ���������� ������� ������� ���������� �1
				 break;
			case 11:
				 set_rezistor2();                   // ���������� ������� ������� ���������� �1
				 break;
			case 12:
				 //
				 break;
			case 13:
				 //
				 break;
			case 14:
				 //
				 break;
			case 15:
				 //
				 break;
			case 16:
				 //                  
				 break;
			case 17:
				 //
				 break;
			case 18:
				 //
				 break;
			case 19:
				 //
				 break;
			case 20:                                         //  
				 //
				 break;
			case 21:                      		 		     //  
				//
				 break;
			case 22:                                         //  
				 //
				 break;
			case 23: 
				 //      
				 break;
			case 24: 
				 //    
				 break;
			case 25: 
				 //         
				 break;
			case 26: 
				 //    
				 break;
			case 27: 
				 //
				 break;
			case 28:
				 //
				 break;
			case 29:
				 //
				 break;
			case 30:  
				 //
				 break;

			default:
				 regBank.set(adr_control_command,0);        // ���������� ���������� �1,�2  ������� �������
				 break;
		 }

	}
	else
	{
	   regBank.set(adr_control_command,0);
	}
}



void draw_Glav_Menu()
{

  but1   = myButtons.addButton( 10,  20, 250,  35, txt_menu1_1);
  but2   = myButtons.addButton( 10,  65, 250,  35, txt_menu1_2);
  but3   = myButtons.addButton( 10, 110, 250,  35, txt_menu1_3);
  but4   = myButtons.addButton( 10, 155, 250,  35, txt_menu1_4);
  butX   = myButtons.addButton( 279, 199,  40,  40, "W", BUTTON_SYMBOL); // ������ ���� 
  but_m1 = myButtons.addButton(  10, 199, 45,  40, "1");
  but_m2 = myButtons.addButton(  61, 199, 45,  40, "2");
  but_m3 = myButtons.addButton(  112, 199, 45,  40, "3");
  but_m4 = myButtons.addButton(  163, 199, 45,  40, "4");
  but_m5 = myButtons.addButton(  214, 199, 45,  40, "5");
  myButtons.drawButtons(); // ������������ ������
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_WHITE);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("                      ", CENTER, 0); 

  switch (m2) 
				   {
					case 1:
					      myGLCD.print(txt_info1, CENTER, 0);
					      break;
					 case 2:
                          myGLCD.print(txt_info2, CENTER, 0);
					      break;
					 case 3:
					      myGLCD.print(txt_info3, CENTER, 0);
					      break;
					 case 4:
					      myGLCD.print(txt_info4, CENTER, 0);
					      break;
					 case 5:
					      myGLCD.print(txt_info5, CENTER, 0);
					      break;
					 }
  myButtons.drawButtons();

}
// ����� ���� ������ ���� � ������� "txt....."
void swichMenu() // ������ ���� � ������� "txt....."
	
{
	 m2=1;                           // ���������� ������ �������� ����
	 while(1) 
	   {
		  myButtons.setTextFont(BigFont);    // ���������� ������� ����� ������  

			if (myTouch.dataAvailable() == true) // ��������� ������� ������
			  {
			    pressed_button = myButtons.checkButtons(); // ���� ������ - ��������� ��� ������
					 if (pressed_button==butX) // ������ ����� ����
					      {  
							// AnalogClock();
							 myGLCD.clrScr();
							 myButtons.drawButtons(); // ������������ ������
							 print_up();              // ������������ ������� ������
					      }
		 
					 if (pressed_button==but_m1) // ������ 1 �������� ����
						  {
							  myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_BLUE); // ������� ��� ����
							  myButtons.drawButtons();   // ������������ ������
							  default_colors=true;
							  m2=1;                                                // ���������� ������ �������� ����
							  myButtons.relabelButton(but1, txt_menu1_1, m2 == 1);
							  myButtons.relabelButton(but2, txt_menu1_2, m2 == 1);
							  myButtons.relabelButton(but3, txt_menu1_3, m2 == 1);
							  myButtons.relabelButton(but4, txt_menu1_4, m2 == 1);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info1, CENTER, 0);            // "���� ������"
		
						  }
				    if (pressed_button==but_m2)
						  {
							  myButtons.setButtonColors(VGA_WHITE, VGA_RED, VGA_YELLOW, VGA_BLUE, VGA_TEAL);
							  myButtons.drawButtons();
							  default_colors=false;
							  m2=2;
							  myButtons.relabelButton(but1, txt_menu2_1 , m2 == 2);
							  myButtons.relabelButton(but2, txt_menu2_2 , m2 == 2);
							  myButtons.relabelButton(but3, txt_menu2_3 , m2 == 2);
							  myButtons.relabelButton(but4, txt_menu2_4 , m2 == 2);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info2, CENTER, 0);            // ����������
						 }

				   if (pressed_button==but_m3)
						 {
							  myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_GREEN);
							  myButtons.drawButtons();
							  default_colors=false;
							  m2=3;
							  myButtons.relabelButton(but1, txt_menu3_1 , m2 == 3);
							  myButtons.relabelButton(but2, txt_menu3_2 , m2 == 3);
							  myButtons.relabelButton(but3, txt_menu3_3 , m2 == 3);
							  myButtons.relabelButton(but4, txt_menu3_4 , m2 == 3);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info3, CENTER, 0);            // ����������
						}
				   if (pressed_button==but_m4)
						{
							  myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_RED);
							  myButtons.drawButtons();
							  default_colors=false;
							  m2=4;
							  myButtons.relabelButton(but1, txt_menu4_1 , m2 == 4);
							  myButtons.relabelButton(but2, txt_menu4_2 , m2 == 4);
							  myButtons.relabelButton(but3, txt_menu4_3 , m2 == 4);
							  myButtons.relabelButton(but4, txt_menu4_4 , m2 == 4);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info4, CENTER, 0);            // 
						}

				   if (pressed_button==but_m5)
						{
							  myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_NAVY);
							  myButtons.drawButtons();
							  default_colors=false;
							  m2=5;
							  myButtons.relabelButton(but1, txt_menu5_1 , m2 == 5);
							  myButtons.relabelButton(but2, txt_menu5_2 , m2 == 5);
							  myButtons.relabelButton(but3, txt_menu5_3 , m2 == 5);
							  myButtons.relabelButton(but4, txt_menu5_4 , m2 == 5);
							  myGLCD.setColor(0, 255, 0);
							  myGLCD.setBackColor(0, 0, 0);
							  myGLCD.print("                      ", CENTER, 0); 
							  myGLCD.print(txt_info5, CENTER, 0);            // 
						}
	
	               //*****************  ���� �1  **************

		           if (pressed_button==but1 && m2 == 1)
			           {
						
								myGLCD.clrScr();   // �������� �����
								myGLCD.print(txt_pass_ok, RIGHT, 208); 
								delay (500);
		    				//	elektro_save_start(); // ���� ����� - ��������� ����� ����
					
			   				 myGLCD.clrScr();
							 myButtons.drawButtons();
							 print_up();
			           }
	  
		           if (pressed_button==but2 && m2 == 1)
					   {
						
								myGLCD.clrScr();   // �������� �����
								myGLCD.print(txt_pass_ok, RIGHT, 208); 
								delay (500);
		    				//	gaz_save_start(); // ���� ����� - ��������� ����� ����
					
			   				myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					   }
	  
		           if (pressed_button==but3 && m2 == 1)
					   {
						
								myGLCD.clrScr();   // �������� �����
								myGLCD.print(txt_pass_ok, RIGHT, 208); 
								delay (500);
		    				 //  colwater_save_start(); // ���� ����� - ��������� ����� ����
					
			   				myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					   }
		           if (pressed_button==but4 && m2 == 1)
					   {
					
								myGLCD.clrScr();   // �������� �����
								myGLCD.print(txt_pass_ok, RIGHT, 208); 
								delay (500);
		    		// 
			   				myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					   }

		         //*****************  ���� �2  **************


		           if (pressed_button==but1 && m2 == 2)
					  {
						//	print_info();
	        				myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
				      }

				  if (pressed_button==but2 && m2 == 2)
					  {
						//   info_nomer_user();
				  			myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					  }
	  
				  if (pressed_button==but3 && m2 == 2)
					  {
					
						 // test_arRequestMod();
							myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					  }
				  if (pressed_button==but4 && m2 == 2)
					  {
						 
						  //  testRemoteAtCommand();
						    myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
				      }
		
		        //*****************  ���� �3  **************
		           if (pressed_button==but1 && m2 == 3) // ������ ����� ���� 3
					{
						    myGLCD.clrScr();   // �������� �����
							myGLCD.print(txt_pass_ok, RIGHT, 208); 
							delay (500);
							//eeprom_clear == 0;
		    		//			system_clear_start(); // ���� ����� - ��������� ����� ����
							 myGLCD.clrScr();
							 myButtons.drawButtons();
							 print_up();
					  }

			 //--------------------------------------------------------------
		           if (pressed_button==but2 && m2 == 3)  // ������ ����� ���� 3
				      {
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
						//	set_n_telef();
							myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
				   
					  }

			   //------------------------------------------------------------------

			       if (pressed_button==but3 && m2 == 3)  // ������ ����� ���� 3
					  { 
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
							delay (500);
		    	//
							myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
				      }

	 //------------------------------------------------------------------
				   if (pressed_button==but4 && m2 == 3) // ��������� ����� ���� 3
				      {
				
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
							delay (500);
		    				//set_warm_temp();
						
						    myGLCD.clrScr();
						    myButtons.drawButtons();
						    print_up();
				      }

                   //*****************  ���� �4  **************

                   if (pressed_button==but1 && m2 == 4) // 
					  {
			
							myGLCD.clrScr();   // �������� �����
							myGLCD.print(txt_pass_ok, RIGHT, 208); 
							delay (500);
							//butA = myButtons.addButton(279, 20,  40,  35, "W", BUTTON_SYMBOL); // ���������
							//if (myButtons.buttonEnabled(butB)) myButtons.deleteButton(butB);
							//if (myButtons.buttonEnabled(butC)) myButtons.deleteButton(butC);
							//if (myButtons.buttonEnabled(butD)) myButtons.deleteButton(butD);
							myButtons.drawButtons();
							print_up();
							//
				   
					  }

				   if (pressed_button==but2 && m2 == 4)
					  {
					
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
							delay (500);
		    	//			butB = myButtons.addButton(279, 65, 40,  35, "W", BUTTON_SYMBOL); // �����������
							//if (myButtons.buttonEnabled(butA)) myButtons.deleteButton(butA);
							//if (myButtons.buttonEnabled(butC)) myButtons.deleteButton(butC);
							//if (myButtons.buttonEnabled(butD)) myButtons.deleteButton(butD);
							myButtons.drawButtons();
							print_up();
					  }

		           if (pressed_button==but3 && m2 == 4) // 
					  {
				
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
							delay (500);
							//butC = myButtons.addButton(279, 110,  40,  35, "W", BUTTON_SYMBOL); // ������������
							//if (myButtons.buttonEnabled(butA)) myButtons.deleteButton(butA);
							//if (myButtons.buttonEnabled(butB)) myButtons.deleteButton(butB);
							//if (myButtons.buttonEnabled(butD)) myButtons.deleteButton(butD);
							myButtons.drawButtons();
							print_up();
					  }
				   if (pressed_button==but4 && m2 == 4) //
					  {
							myGLCD.clrScr();
							myGLCD.print(txt_pass_ok, RIGHT, 208);
							delay (500);
							//butD = myButtons.addButton(279, 155,  40,  35, "W", BUTTON_SYMBOL); // ������������� ������
							//if (myButtons.buttonEnabled(butB)) myButtons.deleteButton(butB);
							//if (myButtons.buttonEnabled(butC)) myButtons.deleteButton(butC);
							//if (myButtons.buttonEnabled(butA)) myButtons.deleteButton(butA);
							myButtons.drawButtons();
							print_up();
					  }
				    //*****************  ���� �5  **************

                   if (pressed_button==but1 && m2 == 5) // ����� ������
					  {
							//ZigBee_status();
			   				myGLCD.clrScr();
							myButtons.drawButtons();
							print_up();
					  }
				   if (pressed_button==but2 && m2 == 5)
					  {
							  myGLCD.clrScr();   // �������� �����
							  myGLCD.print(txt_pass_ok, RIGHT, 208); 
							  delay (500);
		    				 // ZigBee_SetH(); // ���� ����� - ��������� ����� ����
		
							myButtons.drawButtons();
							print_up();
					  }

				   if (pressed_button==but3 && m2 == 5) // ���� ������ ������������
					  {
							  myGLCD.clrScr();   // �������� �����
							  myGLCD.print(txt_pass_ok, RIGHT, 208); 
							  delay (500);
		    				 // ZigBee_SetL(); // ���� ����� - ��������� ����� ����
					
							myButtons.drawButtons();
							print_up();
					  }

			       if (pressed_button==but4 && m2 == 5) // ����� ������ ��������������
			          {
				   
					
								myGLCD.clrScr();   // �������� �����
								myGLCD.print(txt_pass_ok, RIGHT, 208); 
								delay (500);
		    					//ZigBee_Set_Network();
						
							myButtons.drawButtons();
							print_up();
				      }
			
		           if (pressed_button==-1) 
					  {
						//  myGLCD.print("HET", 220, 220);
					  }
				  } 
       }
}
void print_up() // ������ ������� ������� ��� ����
{
	myGLCD.setColor(0, 255, 0);
	myGLCD.setBackColor(0, 0, 0);
	myGLCD.print("                      ", CENTER, 0); 
	switch (m2) 
	{
 		case 1:
			myGLCD.print(txt_info1, CENTER, 0);
			break;
		case 2:
            myGLCD.print(txt_info2, CENTER, 0);
			break;
		case 3:
			myGLCD.print(txt_info3, CENTER, 0);
			break;
		case 4:
			myGLCD.print(txt_info4, CENTER, 0);
			break;
        case 5:
			myGLCD.print(txt_info5, CENTER, 0);
			break;
    }
}

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
void resistor(int resist, int valresist)
{
	resistance = valresist;
	switch (resist)
	{
	case 1:
			Wire.beginTransmission(address_AD5252);     // transmit to device
			Wire.write(byte(control_word1));            // sends instruction byte  
			Wire.write(resistance);                     // sends potentiometer value byte  
			Wire.endTransmission();                     // stop transmitting
			break;
	case 2:				
			Wire.beginTransmission(address_AD5252);     // transmit to device
			Wire.write(byte(control_word2));            // sends instruction byte  
			Wire.write(resistance);                     // sends potentiometer value byte  
			Wire.endTransmission();                     // stop transmitting
			break;
	}
			//Wire.requestFrom(address_AD5252, 1, true);  // ������� ��������� ������ ��������� 
			//level_resist = Wire.read();                 // sends potentiometer value byte  
	// regBank.set(adr_control_command,0);
}
void set_rezistor1()
{
	int mwt1 = regBank.get(40003);             // ����� �������� �������� ������� ���������� � 1
	resistor(1, mwt1);
	regBank.set(adr_control_command,0);
}
void set_rezistor2()
{
	int mwt2 = regBank.get(40004);             // ����� �������� �������� ������� ���������� � 2
	resistor(2, mwt2);
	regBank.set(adr_control_command,0);
}

void save_default_N1()                                          // ������ ��������� ��������� ������� �������� �1
{
	int _step_mem = 20*2;                                         // ����� ����� � �������
	byte _u_konnekt  = 0;                                       // ��������� �������� ����������� ��������.
    for (int i = 0; i < _step_mem;i++)                    
		{
			_u_konnekt = pgm_read_word_near(connektN1_default+i);
			i2c_eeprom_write_byte(deviceaddress,i+adr_memN1_1, _u_konnekt); 
		}
	regBank.set(adr_control_command,0);                        // ��������� ���������    
	delay(100);
}
void save_default_N2()                                          // ������ ��������� ��������� ������� �������� �1
{
	int _step_mem = 26*2;                                         // ����� ����� � �������
	byte _u_konnekt  = 0;                                       // ��������� �������� ����������� ��������.
    for (int i = 0; i < _step_mem;i++)                    
		{
			_u_konnekt = pgm_read_word_near(connektN2_default+i);
			i2c_eeprom_write_byte(deviceaddress,i+adr_memN2_1, _u_konnekt); 
		}
	regBank.set(adr_control_command,0);                        // ��������� ���������    
	delay(100);
}
void save_default_N3()                                          // ������ ��������� ��������� ������� �������� �1
{
	int _step_mem = 37*2;                                         // ����� ����� � �������
	byte _u_konnekt  = 0;                                       // ��������� �������� ����������� ��������.
    for (int i = 0; i < _step_mem;i++)                    
		{
			_u_konnekt = pgm_read_word_near(connektN3_default+i);
			i2c_eeprom_write_byte(deviceaddress,i+adr_memN3_1, _u_konnekt); 
		}
	regBank.set(adr_control_command,0);                        // ��������� ���������    
	delay(100);
}
void save_default_N4()                                          // ������ ��������� ��������� ������� �������� �1
{
	int _step_mem = 34*2;                                         // ����� ����� � �������
	byte _u_konnekt  = 0;                                       // ��������� �������� ����������� ��������.
    for (int i = 0; i < _step_mem;i++)                    
		{
			_u_konnekt = pgm_read_word_near(connektN4_default+i);
			i2c_eeprom_write_byte(deviceaddress,i+adr_memN4_1, _u_konnekt); 
		}
	regBank.set(adr_control_command,0);                        // ��������� ���������    
	delay(100);
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
void setup_port()
{
	pinMode(ledPin13, OUTPUT);   
	pinMode(ledPin12, OUTPUT);  
	digitalWrite(ledPin12, LOW);                   // 
	digitalWrite(ledPin13, LOW);                   // 
	pinMode(kn1Nano, OUTPUT);  
	pinMode(kn2Nano, OUTPUT);  
	pinMode(kn3Nano, OUTPUT);  
	pinMode(kn4Nano, OUTPUT);  
	pinMode(kn5Nano, OUTPUT);  
	pinMode(kn6Nano, OUTPUT);  

	digitalWrite(kn1Nano, HIGH);                        // 
	digitalWrite(kn2Nano, HIGH);                        //
	digitalWrite(kn3Nano, HIGH);                        //
	digitalWrite(kn4Nano, LOW);                         // 
	digitalWrite(kn5Nano, HIGH);                        // 
	digitalWrite(kn6Nano, HIGH);                        //
}
void test_cabel_N1()
{

}
void test_cabel_N2()
{

}
void test_cabel_N3()
{

}
void test_cabel_N4()
{

}
void test_panel_N1()
{

}

void setup_regModbus()
{

/*
��������� ������ Modbus ���������� ���������� ���������
��� ��, ��� ���������� ��������� ����� ��������, ����� ������ � ������
������������������ ������. � ��������� ����� �������� Modbus Slave ��������� �����
������� ������ ���� ���������� ����������� �� ����.
*/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //Assign the modbus device ID.  
  regBank.setId(1);               // Slave ID 1

/*
modbus registers follow the following format
00001-09999  Digital Outputs, A master device can read and write to these registers
10001-19999  Digital Inputs,  A master device can only read the values from these registers
30001-39999  Analog Inputs,   A master device can only read the values from these registers
40001-49999  Analog Outputs,  A master device can read and write to these registers 
����� �����, ����� ��������� �������� ��� ���� � ������� ������. ���
������������ ����� ����������� ����� � �������� � ��������� ���������� ���������
��������� ������� ��� ���������� ������.
*/

  	regBank.add(1);                               //  
	regBank.add(2);                               //  
	regBank.add(3);                               //  
	regBank.add(4);                               //  
	regBank.add(5);                               //  
	regBank.add(6);                               //  
	regBank.add(7);                               //  
	regBank.add(8);                               //  

	regBank.add(10001);                           //  
	regBank.add(10002);                           //  
	regBank.add(10003);                           //  
	regBank.add(10004);                           //  
	regBank.add(10005);                           //  
	regBank.add(10006);                           //  
	regBank.add(10007);                           //  
	regBank.add(10008);                           //  

	regBank.add(30001);                           //  
	regBank.add(30002);                           //  
	regBank.add(30003);                           //  
	regBank.add(30004);                           //  
	regBank.add(30005);                           //  
	regBank.add(30006);                           //  
	regBank.add(30007);                           //  
	regBank.add(30008);                           //  

	regBank.add(40001);                           //  ����� �������� ������� �� ���������� 
	regBank.add(40002);                           //  ����� �������� ���� ������
	regBank.add(40003);                           //  ����� �������� �������� ������� ���������� � 1
	regBank.add(40004);                           //  ����� �������� �������� ������� ���������� � 2
	regBank.add(40005);                           //  
	regBank.add(40006);                           //  
	regBank.add(40007);                           //  
	regBank.add(40008);                           //  

}

void setup()
{
	Serial.begin(9600);                                    // ����������� � USB ��
	Serial1.begin(115200);                                 // ����������� � 
	slave.setSerial(3,57600);                              // ����������� � ��������� MODBUS ���������� Serial3 
	Serial2.begin(115200);                                 // ����������� � 
	Wire.begin();
	if (!RTC.begin())                                      // ��������� ����� 
		{
			Serial.println("RTC failed");
			while(1);
		};
	//DateTime set_time = DateTime(16, 3, 15, 10, 19, 0);  // ������� ������ � ������� � ������ "set_time" ���, �����, �����, �����...
	//RTC.adjust(set_time);                                // �������� ����
	Serial.println(" ");
	Serial.println(" ***** Start system  *****");
	Serial.println(" ");
	//set_time();
	serial_print_date();
	pinMode(ledPin13, OUTPUT);   
	Wire.begin();
	setup_port();
	setup_mcp();                                          // ��������� ����� ����������  
	setup_resistor();                                     // ��������� ��������� ���������
	MsTimer2::set(300, flash_time);                       // 300ms ������ ������� ���������
	resistor(1, 200);                                     // ���������� ������� �������
	resistor(2, 200);                                     // ���������� ������� �������
	setup_regModbus();
	myGLCD.InitLCD();
	myGLCD.clrScr();
	myGLCD.setFont(BigFont);
	myTouch.InitTouch();
	// myTouch.setPrecision(PREC_MEDIUM);
	myTouch.setPrecision(PREC_HI);
	myButtons.setTextFont(BigFont);
	myButtons.setSymbolFont(Dingbats1_XL);
    

	draw_Glav_Menu();

	digitalWrite(ledPin13, HIGH);                       // 
	Serial.println(" ");                                //
	Serial.println("System initialization OK!.");       // ���������� � ���������� ���������
}

void loop()
{
	//draw_Glav_Menu();
	swichMenu();
	//delay(1000);
}
