#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <stdint.h>

#define CE_PIN 9    //!!!!!!!!!!! Для Уно. Здесь установить для ваших плат!              
#define CSN_PIN 10  //!!!!!!!!!!! Для Уно. Здесь установить для ваших плат!
RF24 radio(CE_PIN, CSN_PIN); // Определяем рабочие ножки;

const uint64_t readingPipe = 0xE8E8F0F0AALL;
const uint64_t writingPipe = 0xE8E8F0F0ABLL;
uint32_t message;  // Эта переменная для сбора обратного сообщения от приемника;

void setup()
{
  Serial.begin(9600);

  radio.begin();                          // Включение модуля;
  radio.setChannel(0);                    // Установка канала вещания;
  radio.setRetries(15, 15);               // Установка интервала и количества попыток "дозвона" до приемника;
  radio.setDataRate(RF24_250KBPS);        // Установка минимальной скорости;
  radio.setPALevel(RF24_PA_MAX);          // Установка максимальной мощности;
  radio.setAutoAck(1);                    // Установка режима подтверждения приема;
  radio.openWritingPipe(writingPipe);     // Активация данных для отправки
  radio.openReadingPipe(1, readingPipe);  // Активация данных для чтения
  radio.startListening();                 // Слушаем эфир.
}

void loop()
{
  int command = 100;  // Не суть - приемнику надо что-то передать, но это может быть и полезная информация;

  for (int i = 0; i < 4; i++)
  {
    Serial.print(command);
    Serial.print(" - ");
    radio.flush_tx();// Новая функция, которая стала доступной.
    radio.write( &command, sizeof(command) );  //Отправляем команду;
    if ( radio.isAckPayloadAvailable())
    { // Ждем получения...
      radio.read(&message, sizeof(message)); //... и имеем переменную message с числом 111 от приемника.
      Serial.println(message);
    }
    command++;
    Serial.println();
    //delay(500);
  }
  delay(500);
}
int serial_putc( char c, FILE * ) {
  Serial.write( c );
  return c;
}

void printf_begin(void) {
  fdevopen( &serial_putc, 0 );
}

