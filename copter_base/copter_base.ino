/*



  	
*/

#include <SPI.h>                                              // подключаем библиотеку SPI
#include <ModbusRtu.h>                                        // подключаем библиотеку MODBUS мастер
#include <LCD5110_Graph.h>
//#include <LCD5110_Basic.h>


LCD5110 myGLCD(8,7,6,5,4);


uint16_t au16data[16];                                        // Создание массива данных для приема запроса
uint16_t au16data1[16];                                       // Создание массива данных для приема запроса
uint8_t u8state;                                              // состояние 
uint8_t u8query;                                              // указатель на запрос сообщения
const int ledPin =  13;                                       // котрольный светодиод
int ledState = LOW;                                           // состояние светодиода

/*   Схема подключения дисплея  Nokia 5110 LCD 84x48
     Arduino Nano              Display 
          3,3v         <->     1 - VCC 3,3v   
		  GND		   <->	   2 - GND
			4	 	   <->	   3 - SCE
			5		   <->	   4 - RST
			6		   <->	   5 - D/C
			7		   <->	   6 - DN(MOSI) 
			8		   <->	   7 - SCLK
		  3,3v 		   <->	   8 - LED
*/

extern uint8_t SmallFont[];
extern uint8_t arduino_logo[];
extern unsigned char TinyFont[];
extern uint8_t The_End[];
extern uint8_t pacman1[];
extern uint8_t pacman2[];
extern uint8_t pacman3[];
extern uint8_t pill[];

float y;
uint8_t* bm;
int pacy;

//extern uint8_t MediumNumbers[];
/*
    Modbus object declaration
    u8id : node id = 0 for master, = 1..247 for slave
    u8serno : serial port (use 0 for Serial)
    u8txenpin : 0 for RS-232 and USB-FTDI
    or any pin number > 1 for RS-485
*/
Modbus master(0, 0, 0);                                      // режим мастер, serial 0, RS-232

/*
   Это структура, которая содержит запрос к ведомому устройству
*/
modbus_t telegram[2];                                        // Определено 2 типа запроса 

unsigned long u32wait;



//int y;




void test_display()
{
myGLCD.clrScr();
  myGLCD.drawBitmap(0, 0, arduino_logo, 84, 48);
  myGLCD.update();

  delay(2000);
  
  myGLCD.clrScr();
  myGLCD.print("LCD5110_Graph", CENTER, 0);
  myGLCD.print("DEMO", CENTER, 20);
  myGLCD.drawRect(28, 18, 56, 28);
  for (int i=0; i<6; i++)
  {
    myGLCD.drawLine(57, 18+(i*2), 83-(i*3), 18+(i*2));
    myGLCD.drawLine((i*3), 28-(i*2), 28, 28-(i*2));
  }
  myGLCD.setFont(TinyFont);
  myGLCD.print("(C)2015 by", CENTER, 36);
  myGLCD.print("Henning Karlsen", CENTER, 42);
  myGLCD.update();
  
  delay(5000);
  
  myGLCD.clrScr();
  for (int i=0; i<48; i+=2)
  {
    myGLCD.drawLine(0, i, 83, 47-i);
    myGLCD.update();
  }
  for (int i=83; i>=0; i-=2)
  {
    myGLCD.drawLine(i, 0, 83-i, 47);
    myGLCD.update();
  }

  delay(2000);
  
  myGLCD.clrScr();
  myGLCD.drawRect(0, 0, 83, 47);
  for (int i=0; i<48; i+=4)
  {
    myGLCD.drawLine(0, i, i*1.75, 47);
    myGLCD.update();
  }
  for (int i=0; i<48; i+=4)
  {
    myGLCD.drawLine(83, 47-i, 83-(i*1.75), 0);
    myGLCD.update();
  }

  delay(2000);
  
  myGLCD.clrScr();
  for (int i=0; i<8; i++)
  {
    myGLCD.drawRoundRect(i*3, i*3, 83-(i*3), 47-(i*3));
    myGLCD.update();
  }

  delay(2000);
  
  myGLCD.clrScr();
  for (int i=0; i<17; i++)
  {
    myGLCD.drawCircle(41, 23, i*3);
    myGLCD.update();
  }

  delay(2000);
  
  myGLCD.clrScr();
  myGLCD.drawRect(0, 0, 83, 47);
  myGLCD.drawLine(0, 23, 84, 23);
  myGLCD.drawLine(41, 0, 41, 47);
  for (int c=0; c<4; c++)
  {
    for (int i=0; i<84; i++)
    {
      y=i*0.017453292519943295769236907684886;
      myGLCD.invPixel(i, (sin(y*6)*20)+23);
      myGLCD.update();
      delay(20);
    }
  }

  delay(2000);

  for (int pc=0; pc<3; pc++)
  {
    pacy=random(0, 28);
  
    for (int i=-20; i<84; i++)
    {
      myGLCD.clrScr();
      for (int p=4; p>((i+20)/20); p--)
        myGLCD.drawBitmap(p*20-8, pacy+7, pill, 5, 5);
      switch(((i+20)/3) % 4)
      {
        case 0: bm=pacman1;
                break;
        case 1: bm=pacman2;
                break;
        case 2: bm=pacman3;
                break;
        case 3: bm=pacman2;
                break;
      }
      myGLCD.drawBitmap(i, pacy, bm, 20, 20);
      myGLCD.update();
      delay(25);
    }
  }

  for (int i=0; i<25; i++)
  {
    myGLCD.clrScr();
    myGLCD.drawBitmap(0, i-24, The_End, 84, 24);
    myGLCD.update();
    delay(100);
  }
  myGLCD.setFont(SmallFont);
  myGLCD.print("Runtime (ms):", CENTER, 32);
  myGLCD.printNumI(millis(), CENTER, 40);
  myGLCD.update();
  for (int i=0; i<5; i++)
  {
    myGLCD.invert(true);
    delay(1000);
    myGLCD.invert(false);
    delay(1000);
  }

}

void setup()
{
  Serial.begin(9600);

  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  randomSeed(analogRead(7));

  // telegram 0: read registers                              // формирование запроса чтения регистров 
  telegram[0].u8id = 1;                                      // slave address
  telegram[0].u8fct = 3;                                     // function code (this one is registers read)
  telegram[0].u16RegAdd = 0;                                 // start address in slave
  telegram[0].u16CoilsNo = 10;                               // number of elements (coils or registers) to read
  telegram[0].au16reg = au16data;                            // pointer to a memory array in the Arduino
 
  // telegram 1: write a single coil                         // формирование запроса записи данных в регистры
  telegram[1].u8id = 1;                                      // slave address
  telegram[1].u8fct = 5;                                     // function code (this one is write a single register)
  telegram[1].u16RegAdd = 13;                                // start address in slave
  telegram[1].u16CoilsNo = 1;                                // number of elements (coils or registers) to read
  telegram[1].au16reg = au16data1;                           // pointer to a memory array in the Arduino

  master.begin( 9600 );                                      // baud-rate at 9600
  master.setTimeOut( 5000 );                                 // производить запрос каждые 5 секунд.
  u32wait = millis() + 1000;                                 // ожидание ответа
  u8state = u8query = 0;                                     // 
  pinMode(ledPin, OUTPUT);                                   // устанавливаем режим работы вывода, как "выход"
  digitalWrite(ledPin, LOW);                                 // выключить светодиод



  //myGLCD.drawBitmap(0, 0, arduino_logo, 84, 48);             // 
  //delay(2000);                                               //
  //myGLCD.clrScr();                                           //   
  //myGLCD.setFont(SmallFont);                                           //  
  myGLCD.print("READY",CENTER,16);                           //
  myGLCD.update();
  delay(3000);                                               //
  myGLCD.clrScr();                                           //  
  //randomSeed(analogRead(0));
}

void loop()
{

	test_display();


  //switch ( u8state )
  //{
  //  case 0:
  //    if (millis() > u32wait) u8state++;                    // wait state
  //    break;
  //  case 1:
  //    master.query( telegram[u8query] );                    // send query (only once)
  //    u8state++;
  //    u8query++;
  //    if (u8query > 2) u8query = 0;
  //    break;
  //  case 2:
  //    master.poll();                                        // check incoming messages
  //    if (master.getState() == COM_IDLE)
  //    {
  //      u8state = 0;
  //      u32wait = millis() + 1000;

		//if(u8query == 1)
		//{
		//	Serial.print("temp - ");
		//	for (int i = 0; i < telegram[0].u16CoilsNo; i++)
		//	{
		//	  Serial.print(telegram[0].au16reg[i], HEX);
		//	  Serial.print(" ");
		//	  telegram[0].au16reg[i] = 0;
		//	}
		//	Serial.println();
		//}


  //      if (ledState == LOW) 
		//{
  //        ledState = HIGH;
  //      }
		//else 
		//{
  //        ledState = LOW;
  //      }
  //    }
///*
//      if (ledState == LOW)
//      {
//        au16data1[0] = 0;
//        telegram[1].au16reg = au16data1;
//        digitalWrite(ledPin, ledState);
//      }
//      else
//      {
//        au16data1[0] = 255;
//        telegram[1].au16reg =au16data1;
//        digitalWrite(ledPin, ledState);
//      }
//	  */
//      break;
//  }

  // if the LED is off turn it on and vice-versa:

 // au16data[4] = analogRead( 0 );

}
