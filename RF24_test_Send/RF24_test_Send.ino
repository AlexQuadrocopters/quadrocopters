#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9, 10);
const uint64_t pipe = 0xE8E8F0F0E1LL;
uint32_t message;  // Эта переменная для сбора обратного сообщения от приемника;

void setup()
{
  radio.begin();
  radio.enableAckPayload();
  radio.openWritingPipe(pipe);
}

void loop()
{
  int command = 555;  // Не суть - приемнику надо что-то передать, но это может быть и полезная информация;
  radio.flush_tx();// Новая функция, которая стала доступной.
  radio.write( &command, sizeof(command) );  //Отправляем команду;
  if ( radio.isAckPayloadAvailable() )
  { // Ждем получения...
    radio.read(&message, sizeof(message)); //... и имеем переменную message с числом 111 от приемника.
  }
}
int serial_putc( char c, FILE * )
{
  Serial.write( c );
  return c;
}

void printf_begin(void)
{
  fdevopen( &serial_putc, 0 );
}
