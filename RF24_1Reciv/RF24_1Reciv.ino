/*
  Рабочий вариант 18.06.2014
  1 - GND
  2 - VCC 3.3V !!! NOT 5V
  3 - CE to Arduino pin 9
  4 - CSN to Arduino pin 10
  5 - SCK to Arduino pin 13
  6 - MOSI to Arduino pin 11
  7 - MISO to Arduino pin 12
  8 - UNUSED
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <stdint.h>

#define CE_PIN 9    //!!!!!!!!!!! Для Уно. Здесь установить для ваших плат!              
#define CSN_PIN 10  //!!!!!!!!!!! Для Уно. Здесь установить для ваших плат!
RF24 radio(CE_PIN, CSN_PIN); // Определяем рабочие ножки;

const uint64_t readingPipe = 0xE8E8F0F0AALL;
const uint64_t writingPipe = 0xE8E8F0F0ABLL;

#define PIN_RED 3
#define PIN_GREEN 5
#define PIN_BLUE 6
#define PIN_LIGHT 4
#define BrightGoToilet 15


int command;

unsigned long timeToStepMoodLamp = 0;
#define delayNextStepMoodLamp 5
#define delayToGoToilet 150

uint8_t i = 0;                                // Всякие счетчики;
uint8_t redOld = 0, greenOld = 0, blueOld = 0, red = 0, green = 0, blue = 0;
int iUp = 0;
int iDown = 25;
enum {
  stepCheckNRF,
  stepOff,
  stepOnLight,
  stepMoodLight,
  stepGotToilet,
} StepOfProgramNow;

struct {
  unsigned switchOff      : 1;
  unsigned switchOn        : 1;
  unsigned moodLamp        : 1;
  unsigned gotToilet      : 1;
  unsigned lightSwitchedOn : 1;
} flag;
/*=================================== Setup ===================================== */
void setup()  {
  Serial.begin(9600);
  delay(1500);
  Serial.println("Nrf24L01 Receiver Starting");

  pinMode(PIN_LIGHT, OUTPUT);
  delay(2000);
  radio.begin();                          // Включение модуля;
  radio.setChannel(0);                    // Установка канала вещания;
  radio.setRetries(15, 15);               // Установка интервала и количества попыток "дозвона" до приемника;
  radio.setDataRate(RF24_250KBPS);        // Установка минимальной скорости;
  radio.setPALevel(RF24_PA_MAX);          // Установка максимальной мощности;
  radio.setAutoAck(1);                    // Установка режима подтверждения приема;
  radio.openWritingPipe(writingPipe);     // Активация данных для отправки
  radio.openReadingPipe(1, readingPipe);  // Активация данных для чтения
  radio.startListening();                 // Слушаем эфир.
  //radio.flush_tx();                       // Новая функция, которая стала доступной.

  delay(500);

  StepOfProgramNow = stepCheckNRF;
  flag.switchOn = 0;
  flag.moodLamp = 0;
  flag.gotToilet = 0;
}

/*=================================== Loop ===================================== */

void loop(void)
{
  uint32_t message = 111;  //Вот какой потенциальной длины сообщение - uint32_t!
  //туда можно затолкать значение температуры от датчика или еще что-то полезное.
//  radio.flush_tx();// Новая функция, которая стала доступной.
//  radio.writeAckPayload( 1, &message, sizeof(message) ); // Грузим сообщение для автоотправки;
  switch (StepOfProgramNow)
  {
    case stepCheckNRF:
      if (timeToStepMoodLamp > millis())
      { // Проверим что со временем - не перешли ли через ноль?
        timeToStepMoodLamp = millis();
      }
       radio.writeAckPayload( 1, &message, sizeof(message) ); // Грузим сообщение для автоотправки;
      if ( radio.available())
      { // В случае, если есть что прочитать в буфере;
        bool done = false;
        while (!done)                                    // Начинаем упрямо считывать информацию;
        {
          done = radio.read( &command, sizeof(command) );
                                Serial.print(" Command = ");
                                Serial.println(command);

          if (command == 100) {                        // Флаги для "Все выключено";
            flag.switchOff = 1;
            flag.switchOn = 0;
            flag.moodLamp = 0;
            flag.gotToilet = 0;
          }

          if (command == 101) {                        // Флаги для "Включена подсветка";
            flag.switchOff = 0;
            flag.switchOn = 1;
            flag.lightSwitchedOn = 0;
            flag.moodLamp = 0;
            flag.gotToilet = 0;
          }
          if (command == 102) {                        // Флаги для "Лампа настроения";
            flag.switchOff = 0;
            flag.switchOn = 0;
            flag.moodLamp = 1;
            flag.lightSwitchedOn = 0;
            flag.gotToilet = 0;
          }
          if (command == 103) {                        // Флаги для "Ночная подсветка";
            flag.switchOff = 0;
            flag.switchOn = 0;
            flag.moodLamp = 0;
            flag.gotToilet = 1;
            flag.lightSwitchedOn = 0;
          }
        }
      }
      StepOfProgramNow = stepOff;
      break;

    /*=================================== Off ===================================== */
    case stepOff:
      if (flag.switchOff && flag.lightSwitchedOn) {
        if (millis() > timeToStepMoodLamp + delayToGoToilet) {
          iDown--;
          analogWrite(PIN_RED, iDown);
          analogWrite(PIN_GREEN, iDown);
          analogWrite(PIN_BLUE, iDown);

          if (0 == iDown) {
            digitalWrite(PIN_LIGHT, 0);
            flag.lightSwitchedOn = 0;
            Serial.println("Write 0");
            iDown = BrightGoToilet;
          }
        }
      }
      StepOfProgramNow = stepOnLight;
      break;
    /*=================================== On Light ===================================== */
    case stepOnLight:
      if (flag.switchOn && !flag.lightSwitchedOn) {
        analogWrite(PIN_RED, 255);
        analogWrite(PIN_GREEN, 255);
        analogWrite(PIN_BLUE, 255);
        digitalWrite(PIN_LIGHT, 1);
        flag.lightSwitchedOn = 1;
        iDown = 255;
          Serial.println("Write 255");
        //delay(3000);
      }

      StepOfProgramNow = stepMoodLight;
      break;
    /*=================================== Mood Lamp ===================================== */
    case stepMoodLight:
      if (flag.moodLamp) {

        if (!flag.lightSwitchedOn) {
          iDown = 127;
          digitalWrite(PIN_LIGHT, 1);
          flag.lightSwitchedOn = 1;
        }

        if (millis() > timeToStepMoodLamp + delayNextStepMoodLamp)
        {
          if ((red == redOld) && (green == greenOld) && (blue == blueOld) ) {
            do {
              red  = random(255); green = random(255); blue  = random(255);
            } while (((abs)(red - green) < 140) && ((abs)(green - blue) < 140) && ((abs)(blue - red) < 140 ) );

            i++;                                                        // Счетчик смены цветов;
            if (10 == i) {
              red = 255;  // Периодически устанавливаем чистые цвета;
              green = 0;
              blue = 0;
            }
            if (20 == i) {
              red = 0;
              green = 255;
              blue = 0;
            }
            if (30 == i) {
              red = 0;
              green = 0;
              blue = 255;
            }
            if (40 == i) {
              red = 255;
              green = 255;
              blue = 0;
            }
            if (50 == i) {
              red = 0;
              green = 255;
              blue = 255;
            }
            if (60 == i) {
              red = 255;
              green = 0;
              blue = 255;
            }
            if (70 == i) {
              red = 255;
              green = 255;
              blue = 255;
              i = 0;
            }
          }

          analogWrite (PIN_RED, redOld);
          analogWrite (PIN_GREEN, greenOld);
          analogWrite (PIN_BLUE, blueOld);
          //Serial.println("Write Mood");

          if (red > redOld) redOld++;
          if (red < redOld) redOld--;
          if (green > greenOld) greenOld++;
          if (green < greenOld) greenOld--;
          if (blue > blueOld)    blueOld++;
          if (blue < blueOld) blueOld--;

          timeToStepMoodLamp = millis();
        }
      }
      StepOfProgramNow = stepGotToilet;
      break;

    /*=================================== Go To Toilet ===================================== */
    case stepGotToilet:
      if ( flag.gotToilet && !flag.lightSwitchedOn) {
        if (millis() > timeToStepMoodLamp + delayToGoToilet)
        {
          iUp++;
          analogWrite(PIN_RED, iUp);
          analogWrite(PIN_GREEN, iUp);
          analogWrite(PIN_BLUE, iUp);
          //digitalWrite(PIN_LIGHT,0);

          if (iUp == BrightGoToilet) {
            flag.lightSwitchedOn = 1;
            iUp = 0;
          }
        }
        Serial.println("Write 25 to Toilet");
      }
      StepOfProgramNow = stepCheckNRF;
      break;

    default:
      break;
  }
}
int serial_putc( char c, FILE * ) {
  Serial.write( c );
  return c;
}

void printf_begin(void) {
  fdevopen( &serial_putc, 0 );
}

