/**
    Modbus master example 2:
    The purpose of this example is to query several sets of data
    from an external Modbus slave device.
    The link media can be USB or RS232.

    Recommended Modbus slave:
    diagslave http://www.modbusdriver.com/diagslave.html

    In a Linux box, run
    "./diagslave /dev/ttyUSB0 -b 19200 -d 8 -s 1 -p none -m rtu -a 1"
  	This is:
  		serial port /dev/ttyUSB0 at 19200 baud 8N1
 		RTU mode and address @1
*/

#include <ModbusRtu.h>

uint16_t au16data[16]; //!< data array for modbus network sharing
uint16_t au16data1[16]; //!< data array for modbus network sharing
uint8_t u8state; //!< machine state
uint8_t u8query; //!< pointer to message query
const int ledPin =  13;      // the number of the LED pin

// Variables will change :
int ledState = LOW;             // ledState used to set the LED
/**
    Modbus object declaration
    u8id : node id = 0 for master, = 1..247 for slave
    u8serno : serial port (use 0 for Serial)
    u8txenpin : 0 for RS-232 and USB-FTDI
                 or any pin number > 1 for RS-485
*/
Modbus master(0, 3, 0); // this is master and RS-232 or USB-FTDI

/**
   This is an structe which contains a query to an slave device
*/
modbus_t telegram[2];

unsigned long u32wait;

void setup()
{

  Serial.begin(9600);
  // telegram 0: read registers
  telegram[0].u8id = 1; // slave address
  telegram[0].u8fct = 3; // function code (this one is registers read)
  telegram[0].u16RegAdd = 0; // start address in slave
  telegram[0].u16CoilsNo = 10; // number of elements (coils or registers) to read
  telegram[0].au16reg = au16data; // pointer to a memory array in the Arduino
 
  // telegram 1: write a single coil
  telegram[1].u8id = 1; // slave address
  telegram[1].u8fct = 5; // function code (this one is write a single register)
  telegram[1].u16RegAdd = 13; // start address in slave
  telegram[1].u16CoilsNo = 1; // number of elements (coils or registers) to read
  telegram[1].au16reg = au16data1; // pointer to a memory array in the Arduino

  master.begin( 9600 ); // baud-rate at 19200
  master.setTimeOut( 5000 ); // if there is no answer in 5000 ms, roll over
  u32wait = millis() + 1000;
  u8state = u8query = 0;
  pinMode(ledPin, OUTPUT);                           // устанавливаем режим работы вывода, как "выход"
}

void loop()
{

  switch ( u8state )
  {
    case 0:
      if (millis() > u32wait) u8state++; // wait state
      break;
    case 1:
      master.query( telegram[u8query] ); // send query (only once)
      u8state++;
      u8query++;
      if (u8query > 2) u8query = 0;
      break;
    case 2:
      master.poll(); // check incoming messages
      if (master.getState() == COM_IDLE)
      {
        u8state = 0;
        u32wait = millis() + 1000;

		if(u8query == 1)
		{
			Serial.print("temp - ");
			for (int i = 0; i < telegram[0].u16CoilsNo; i++)
			{
			  Serial.print(telegram[0].au16reg[i], HEX);
			  Serial.print(" ");
			  telegram[0].au16reg[i] = 0;
			}
			Serial.println();
		}


        if (ledState == LOW) 
		{
          ledState = HIGH;
        }
		else 
		{
          ledState = LOW;
        }
      }
/*
      if (ledState == LOW)
      {
        au16data1[0] = 0;
        telegram[1].au16reg = au16data1;
        digitalWrite(ledPin, ledState);
      }
      else
      {
        au16data1[0] = 255;
        telegram[1].au16reg =au16data1;
        digitalWrite(ledPin, ledState);
      }
	  */
      break;
  }

  // if the LED is off turn it on and vice-versa:

 // au16data[4] = analogRead( 0 );

}
