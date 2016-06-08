#include <SPI.h>
#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <nRF24L01.h>

// Адрес модуля
#define ADDR "serv1"
// Размер полезной нагрузки
#define PAYLOAD sizeof(unsigned long)
// Светодиод для индикации - 4 пин
#define StatusLed 4
// Переменная для приёма и передачи данных
unsigned long data = 0;
unsigned long command = 0;

void setup() {
  Serial.begin(9600);
  // Мигнём светодиодом:
  pinMode(StatusLed, OUTPUT);
  for (byte i = 0; i < 3; i++) {
    digitalWrite(StatusLed, HIGH);
    delay(100);
    digitalWrite(StatusLed, LOW);
  }

  Mirf.cePin = 8;
  Mirf.csnPin = 7;
  Mirf.spi = &MirfHardwareSpi;
  MirfHardwareSpi;
  Mirf.init();

  Mirf.setRADDR((byte*)ADDR);
  Mirf.payload = sizeof(unsigned long);
  Mirf.config();
 // digitalWrite(A0, HIGH);
 // digitalWrite(A1, HIGH);
 // digitalWrite(A2, HIGH);
  Serial.println("Beginning ... ");
}
void loop() {
  // Обнуляем переменную с данными:
  data = 0;
  command = 0;
  // Ждём данных
  if (!Mirf.isSending() && Mirf.dataReady()) {
    Serial.println("Got packet");
    //Сообщаем коротким миганием светодиода о наличии данных
    digitalWrite(StatusLed, HIGH);
    delay(100);
    digitalWrite(StatusLed, LOW);
    // Принимаем пакет данные в виде массива байт в переменную data:
    Mirf.getData((byte *) &command);
    // Сообщаем длинным миганием светодиода о получении данных
    digitalWrite(StatusLed, HIGH);
    delay(500);
    digitalWrite(StatusLed, LOW);
    // Выводим полученные данные в монитор серийного порта
    Serial.print("Get data: ");
    Serial.println(command);
  }
  // Если переменная не нулевая, формируем ответ:
  if (command != 0) {
    switch (command) {
      case 1:
        // Команда 1 - отправить число милисекунд,
        // прошедших с последней перезагрузки платы
        Serial.println("Command 1. Send millis().");
        data = millis();
        break;
      case 2:
        // команда 2 - отправить значение с пина AnalogPin0
        Serial.println("Command 2. Send A0 reference.");
        data = analogRead(A0);
        break;
      case 3:
        // команда 3 - отправить значение с пина AnalogPin1
        Serial.println("Command 3. Send A1 reference.");
        data = analogRead(A1);
        break;
      default:
        // Нераспознанная команда. Сердито мигаем светодиодом 10 раз и
        // жалуемся в последовательный порт
        Serial.println("Unknown command");
        for (byte i = 0; i < 10; i++) {
          digitalWrite(StatusLed, HIGH);
          delay(100);
          digitalWrite(StatusLed, LOW);
        }
        break;
    }
    // Отправляем ответ:

    Mirf.setTADDR((byte *)"clie1");
    //Отправляем ответ в виде массива байт:
    Mirf.send((byte *)&data);
  }
  // Экспериментально вычисленная задержка.
  // Позволяет избежать проблем с модулем.
  delay(10);
}
