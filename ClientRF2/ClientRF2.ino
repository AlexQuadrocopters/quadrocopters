#include <SPI.h>
#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <nRF24L01.h>

// Адрес модуля
#define ADDR "clie1"
#define PAYLOAD sizeof(unsigned long)
// Светодиод для индикации - 4 пин
#define StatusLed 4
// Переменная для приёма и передачи данных
unsigned long data = 0;
unsigned long command = 0;
// Флаг для определения выхода по таймауту
boolean timeout = false;
// Таймаут ожидания ответа от сервера - 1 секунда
#define TIMEOUT 1000
// Переменная для запоминания времени отправки
unsigned long timestamp = 0;

void setup() {
  Serial.begin(9600);
  // Мигнём светодиодом:
  pinMode(StatusLed, OUTPUT);
  for (byte i = 0; i < 3; i++) {
    digitalWrite(StatusLed, HIGH);
    delay(100);
    digitalWrite(StatusLed, LOW);
  }

  Mirf.csnPin = 9;
  Mirf.cePin = 10;
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();

  Mirf.setRADDR((byte*)ADDR);
  Mirf.payload = sizeof(unsigned long);
  Mirf.config();
  Serial.println("Beginning ... ");
}

void loop() {
  timeout = false;
  // Устанавливаем адрес передачи
  Mirf.setTADDR((byte *)&"serv1");
  // Запрашиваем число милисекунд,
  // прошедших с последней перезагрузки сервера:
  
  Serial.println("Request millis()");
  command = 1;
  Mirf.send((byte *)&command);
  // Мигнули 1 раз - команда отправлена
  digitalWrite(StatusLed, HIGH);
  delay(200);
  digitalWrite(StatusLed, LOW);
  // Запомнили время отправки:
  timestamp = millis();
  // Запускаем профедуру ожидания ответа
  waitanswer();

  // Запрашиваем число милисекунд,
  // прошедших с последней перезагрузки сервера:
  Serial.println("Request A0 reference");
  command = 2;
  Mirf.send((byte *)&command);
  // Мигнули 1 раз - команда отправлена
  digitalWrite(StatusLed, HIGH);
  delay(100);
  digitalWrite(StatusLed, LOW);
  // Запомнили время отправки:
  timestamp = millis();
  // Запускаем профедуру ожидания ответа
  waitanswer();

 // Запрашиваем число милисекунд,
  // прошедших с последней перезагрузки сервера:
  Serial.println("Request A1 reference");
  command = 3;
  Mirf.send((byte *)&command);
  // Мигнули 1 раз - команда отправлена
  digitalWrite(StatusLed, HIGH);
  delay(200);
  digitalWrite(StatusLed, LOW);
  // Запомнили время отправки:
  timestamp = millis();
  // Запускаем профедуру ожидания ответа
  waitanswer();
/*
  // Отправляем невалидную команду
  // прошедших с последней перезагрузки сервера:
  Serial.println("Invalid command");
  command = 42;
  Mirf.send((byte *)&command);
  // Мигнули 1 раз - команда отправлена
  digitalWrite(StatusLed, HIGH);
  delay(100);
  digitalWrite(StatusLed, LOW);
  // Запомнили время отправки:
  timestamp = millis();
  // Запускаем профедуру ожидания ответа
  waitanswer();
  */
  // Эксперимаентально вычисленная задержка.
  // Позволяет избежать проблем с модулем.
  delay(10);
  Serial.println("-----------------------------------------");
  delay(1000);
}

void waitanswer() {
  // Немного плохого кода:
  // Устанавливаем timeout в ИСТИНУ
  // Если ответ будет получен, установим переменную в ЛОЖЬ
  // Если ответа не будет - считаем ситуацию выходом по таймауту
  timeout = true;
  // Ждём ответ или таймута ожидания
  while (millis() - timestamp < TIMEOUT && timeout) {
    if (!Mirf.isSending() && Mirf.dataReady()) {
      // Мигнули 2 раза - ответ получен
//      for (byte i = 0; i < 2; i++) {
//        digitalWrite(StatusLed, HIGH);
//        delay(100);
//        digitalWrite(StatusLed, LOW);
//      }
          delay(300);
      timeout = false;

      // Принимаем пакет данные в виде массива байт в переменную data:
      Mirf.getData((byte *)&data);
      // Выводим полученные данные в монитор серийного порта
      Serial.println("Get data: ");
    //  Serial.println(command);
      Serial.println(data);
      data = 0;
    }
  }
  if (timeout) {
    // Мигнули 10 раз - ответа не пришло
    for (byte i = 0; i < 10; i++) {
      digitalWrite(StatusLed, HIGH);
      delay(100);
      digitalWrite(StatusLed, LOW);
    }
    Serial.println("Timeout");
  }
}
