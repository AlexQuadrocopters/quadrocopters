#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
//#include "printf.h"
RF24 radio(9, 10); // Определяем рабочие ножки;
const uint64_t pipe = 0xE8E8F0F0E1LL; // Определяем адрес рабочей трубы;
//const uint64_t pipe = 0xE8E8F0F0AALL;
//const uint64_t pipe = 0xE8E8F0F0ABLL;

void setup()
{
  radio.begin(); // Старт работы;
  radio.enableAckPayload();  // Разрешение отправки нетипового ответа передатчику;
  radio.openReadingPipe(1, pipe); // Открываем трубу и
  radio.startListening();  //начинаем слушать;
}

void loop()
{
  uint32_t message = 111;  //Вот какой потенциальной длины сообщение - uint32_t!
  //туда можно затолкать значение температуры от датчика или еще что-то полезное.
  radio.writeAckPayload( 1, &message, sizeof(message) ); // Грузим сообщение для автоотправки;
  if ( radio.available() )
  { //Просто читаем и очищаем буфер - при подтверждении приема
    int dataIn;  //передатчику приемник протолкнет ему в обратку наше сообщение;
    bool done = false;
    while (!done)
    {
      done = radio.read( &dataIn, sizeof(dataIn)); // Значение dataIn в данном случае
      //не важно. Но его можно использовать и как управляющую команду.
    }
  }
}
