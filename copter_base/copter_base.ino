/*
��������� ����



*/


#include <SPI.h>
#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <nRF24L01.h>

// ����� ������
#define ADDR "remote"   // ����� ������ ����
#define PAYLOAD sizeof(unsigned long)
// ��������� ��� ��������� - 4 ���
#define StatusLed 13
// ���������� ��� ����� � �������� ������
unsigned long data = 0;
unsigned long command = 0;
// ���� ��� ����������� ������ �� ��������
boolean timeout = false;
// ������� �������� ������ �� ������� - 1 �������
#define TIMEOUT 1000
// ���������� ��� ����������� ������� ��������
unsigned long timestamp = 0;

void setup() {
  Serial.begin(9600);
  // ����� �����������:
  pinMode(StatusLed, OUTPUT);
  for (byte i = 0; i < 3; i++) {
    digitalWrite(StatusLed, HIGH);
    delay(100);
    digitalWrite(StatusLed, LOW);
  }

  Mirf.cePin = 10;
  Mirf.csnPin = 9;
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();

  Mirf.setRADDR((byte*)ADDR);
  Mirf.payload = sizeof(unsigned long);
  Mirf.config();
  Serial.println("Beginning ... ");
}

void loop() {
  timeout = false;
  // ������������� ����� ��������
  Mirf.setTADDR((byte *)&"gelicopter");
  // ����������� ����� ����������,
  // ��������� � ��������� ������������ �������:
  Serial.println("Request millis()");
  command = 1;
  Mirf.send((byte *)&command);
  // ������� 1 ��� - ������� ����������
  digitalWrite(StatusLed, HIGH);
  delay(100);
  digitalWrite(StatusLed, LOW);
  // ��������� ����� ��������:
  timestamp = millis();
  // ��������� ��������� �������� ������
  waitanswer();

  // ����������� ����� ����������,
  // ��������� � ��������� ������������ �������:
  Serial.print("cpm = ");
  command = 2;
  Mirf.send((byte *)&command);
  // ������� 1 ��� - ������� ����������
  digitalWrite(StatusLed, HIGH);
  delay(100);
  digitalWrite(StatusLed, LOW);
  // ��������� ����� ��������:
  timestamp = millis();
  // ��������� ��������� �������� ������
  waitanswer();

  Serial.print("uSv/h = ");
  command = 3;
  Mirf.send((byte *)&command);
  // ������� 1 ��� - ������� ����������
  digitalWrite(StatusLed, HIGH);
  delay(100);
  digitalWrite(StatusLed, LOW);
  // ��������� ����� ��������:
  timestamp = millis();
  // ��������� ��������� �������� ������
  waitanswer();

  //  // ���������� ���������� �������
  //  // ��������� � ��������� ������������ �������:
  //  Serial.println("Invalid command");
  //
  //  command=4;
  //  Mirf.send((byte *)&command);
  //  // ������� 1 ��� - ������� ����������
  //  digitalWrite(StatusLed, HIGH);
  //  delay(100);
  //  digitalWrite(StatusLed, LOW);
  //  // ��������� ����� ��������:
  //  timestamp=millis();
  //  // ��������� ��������� �������� ������
  //  waitanswer();
  //  // ����������������� ����������� ��������.
  //  // ��������� �������� ������� � �������.
  delay(10);
  Serial.println("-----------------------------------------");
  delay(1000);
}

void waitanswer() {
  // ������� ������� ����:
  // ������������� timeout � ������
  // ���� ����� ����� �������, ��������� ���������� � ����
  // ���� ������ �� ����� - ������� �������� ������� �� ��������
  timeout = true;
  // ��� ����� ��� ������� ��������
  while (millis() - timestamp < TIMEOUT && timeout) {
    if (!Mirf.isSending() && Mirf.dataReady()) {
      // ������� 2 ���� - ����� �������
      for (byte i = 0; i < 2; i++) {
        digitalWrite(StatusLed, HIGH);
        delay(100);
        digitalWrite(StatusLed, LOW);
      }
      timeout = false;

      // ��������� ����� ������ � ���� ������� ���� � ���������� data:
      Mirf.getData((byte *)&data);
      // ������� ���������� ������ � ������� ��������� �����
      //  Serial.print("Get data: ");
      if ( command == 3)
      {
        float data_f = data;
        data_f=data_f/10000;
        Serial.println(data_f ,4);
       //  Serial.println(data);
      }
      else
      {
        Serial.println(data);
      }

      data = 0;
    }
  }
  if (timeout) {
    // ������� 10 ��� - ������ �� ������
    for (byte i = 0; i < 10; i++) {
      digitalWrite(StatusLed, HIGH);
      delay(100);
      digitalWrite(StatusLed, LOW);
    }
    Serial.println("Timeout");
  }
}
