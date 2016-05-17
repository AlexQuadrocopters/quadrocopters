#include <SPI.h>
#include "LiquidCrystal.h"
#define LOG_PERIOD 36000  //Logging period in milliseconds, recommended value 15000-60000.
#define MAX_PERIOD 60000  //Maximum logging period without modifying this sketch

unsigned long counts;     //variable for GM Tube events
unsigned long cpm;        //variable for CPM
unsigned int multiplier;  //variable for calculation CPM in this sketch
unsigned long previousMillis;  //variable for time measurement

LiquidCrystal lcd(4, 8, 5, 6, 3, 7);
void setup(){             //setup subprocedure
  lcd.begin(8, 2);
  pinMode(13, OUTPUT);
  counts = 0;
  cpm = 0;
  multiplier = MAX_PERIOD / LOG_PERIOD;      //calculating multiplier, depend on your log period
  Serial.begin(9600);
  attachInterrupt(0, tube_impulse, FALLING); //define external interrupts 
lcd.setCursor(0,0);
lcd.print("Counter");
lcd.setCursor(0,1);
lcd.print("Wait");
delay(2000);  lcd.clear();
lcd.print("FON"); 
}

void loop(){    //main cycle
 unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > LOG_PERIOD){
    previousMillis = currentMillis;
    cpm = counts * multiplier;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("FON");
    lcd.setCursor(0,1);
    lcd.print(cpm);  
    lcd.print(" uR/h");

   // Serial.println(cpm); // вывод в сом порт для отладки
    counts = 0;
  }
  
}

//////////////////////////////////////////////////

void tube_impulse(){       //subprocedure for capturing events from Geiger Kit
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1);              // wait for a second
  digitalWrite(13, LOW);
  counts++;
  lcd.setCursor(6,0); // в тпервой строке будет появляться количество пролетевших частичек за период измерения - 36 сек
  lcd.print(counts);
}