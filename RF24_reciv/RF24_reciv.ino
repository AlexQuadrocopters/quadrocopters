/* В дефайнах */
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <stdint.h>

#define CE_PIN 9    //!!!!!!!!!!! Для Уно. Здесь установить для ваших плат!              
#define CSN_PIN 10  //!!!!!!!!!!! Для Уно. Здесь установить для ваших плат!
RF24 radio(CE_PIN, CSN_PIN); // Определяем рабочие ножки;

const uint64_t readingPipe = 0xE8E8F0F0AALL;
const uint64_t writingPipe = 0xE8E8F0F0ABLL;

//const uint64_t pipe = 0xE8E8F0F0E1LL; // Определяем адрес рабочей трубы;

/* В сетапе */
void setup(void)
{
  Serial.begin(9600);
  delay(2000);
  Serial.println("Start reciv");
    radio.begin();                          // Включение модуля;
    radio.setChannel(0);                    // Установка канала вещания;
    radio.setRetries(15,15);                // Установка интервала и количества попыток "дозвона" до приемника;
    radio.setDataRate(RF24_250KBPS);        // Установка минимальной скорости;
    radio.setPALevel(RF24_PA_MAX);          // Установка максимальной мощности;
    radio.setAutoAck(1);                    // Установка режима подтверждения приема;
    radio.openWritingPipe(writingPipe);     // Активация данных для отправки
    radio.openReadingPipe(1,readingPipe);   // Активация данных для чтения
    radio.startListening();                 // Слушаем эфир.
   // delay(2000);
 }
/* В лупе */
void loop(void)
{

  uint32_t message = 111;  //Вот какой потенциальной длины сообщение - uint32_t!
  //туда можно затолкать значение температуры от датчика или еще что-то полезное.
 // radio.flush_tx();// Новая функция, которая стала доступной.
  radio.writeAckPayload( 1, &message, sizeof(message) ); // Грузим сообщение для автоотправки;
  if (radio.available()) 
  { //Просто читаем и очищаем буфер - при подтверждении приема
    int dataIn;  //передатчику приемник протолкнет ему в обратку наше сообщение;
    bool done = false;
    while (!done) 
    {
      done = radio.read( &dataIn, sizeof(dataIn)); // Значение dataIn в данном случае
      Serial.println(dataIn);
      //не важно. Но его можно использовать и как управляющую команду.
    }
  }
}
int serial_putc( char c, FILE * )
{
  Serial.write( c );
  return c;
}

void printf_begin(void) {
  fdevopen( &serial_putc, 0 );
}

