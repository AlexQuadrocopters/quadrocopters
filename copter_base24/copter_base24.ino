/*
  copter_base24.ino
  VisualStudio
  Программа (Базовый вариант)
  Версия:      - 1.0
  Дата:        - 16.06.2016г.
  Автор:       - Мосейчук А.В.
  Реализовано:
  - Меню , Пароль, Часы,

*/

#include <avr/pgmspace.h>
#include <avr/io.h>
#include "Wire.h"
#include <UTFT.h>
#include <UTouch.h>
#include <UTFT_Buttons.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <OneWire.h>
#include <RTClib.h>
#include "I2Cdev.h"

#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <nRF24L01.h>

extern "C" {
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

//------------------------------------------------------------------------------------------------------
const int chipSelect = 53;              //
//#define  alarmInPin    44             // 44
int deviceaddress =    0x50;
unsigned int eeaddress = 0;

int sys_N = 1;                         //

uint8_t sec = 0;         //Initialization time
uint8_t min = 0;
uint8_t hour = 0;
uint8_t dow = 1;
uint8_t date = 1;
uint8_t mon = 1;
uint16_t year = 14;
unsigned long timeF;
int flag_time = 0;

// MsTimer2::TimerInterrupt

RTC_DS1307 RTC;  // define the Real Time Clock object

int clockCenterX = 119;
int clockCenterY = 119;
int oldsec = 0;
char* str[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
char* str_mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char start[80], *end;

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
File myFile;

int  stCurrentLen_pass = 0;              // Длина вводимой строки
char pass_user[20];                      // Строка с паролем пользователя

char pass_super_admin[8] = {'1', '4', '0', '7', '5', '4', '\0'}; // Пароль супер Админ.
char pass_admin[20] ;                     //Строка с паролем Админ
int pass = 0;                             // Признак правильности введенного общего пароля
int pass1 = 0;                            // Признак правильности введенного пароля пользователя
int pass2 = 0;                            // Признак правильности введенного пароля Админ
int pass3 = 0;                            // Признак правильности введенного пароля Супер Админ
int eeprom_clear = 0;
int adr_variant_sys          = 241;       //
//unsigned int eeaddress1 = 0;

byte hi;                                  // Старший байт для преобразования числа
byte low;                                 // Младший байт для преобразования числа

char n_user[20];                          // Переменная хранения № пользователя
char n_telefon[20];                       // Переменная хранения № пользователя
char temp_stLast[20];                     // Переменная для временного хранения содержания строки= stLast

unsigned long count_preobr_str = 0;       // Переменная для преобразования строк в число
unsigned int count_preobr_str1 = 0;       // Переменная для преобразования строк в число

int adr_pass_user = 8;                    // Адрес пароля пользователя

int adr_stCurrentLen1 = 92;               // Адрес указателя длины строки

int adr_pass_admin = 118;                 // адрес пароля администратора
int adr_n_user = 140;                     // Адрес хранения № номера пользователя
int adr_n_telef = 220;                    // Адрес хранения № номера телефона

volatile int state = LOW;

float power60 = 0;                       // Измерение источника питания 6,0 вольт
float power50 = 0;                       // Измерение источника питания 5,0 вольт
float power33 = 0;                       // Измерение источника питания 3,3 вольт
unsigned long currentTime;
unsigned long loopTime;
int time_power    = 1000;
bool geiger_ready = false;               // Готовность информации со счетчика Гейгера
float data_f      = 0;
int gound_m       = 218;                 // Высота местности над уровнем моря

// Настройка монитора
UTFT          myGLCD(ITDB32S, 38, 39, 40, 41);
UTouch        myTouch(6, 5, 4, 3, 2);
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

boolean default_colors = true;
uint8_t menu_redraw_required = 0;
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t Dingbats1_XL[];
extern uint8_t SmallSymbolFont[];

// объявления массив изображений
extern unsigned int rvsn2[0x2710];

//-----------------------------------------------------------------------------------------------

// Переменные для цифровой клавиатуры
int x, y, z;
char stCurrent[20] = "";           // Переменная хранения введенной строки
char stCurrent1[20];               // Переменная хранения введенной строки
int stCurrentLen = 0;              // Переменная хранения длины введенной строки
int stCurrentLen1 = 0;             // Переменная временного хранения длины введенной строки
int stCurrentLen_user = 0;         // Переменная  хранения длины введенной строки пароля пользователя
int stCurrentLen_telef = 0;        // Переменная  хранения длины введенной строки пароля пользователя
int stCurrentLen_admin = 0;        // Переменная  хранения длины введенной строки пароля администратора
char stLast[20] = "";              // Данные в введенной строке строке.
char stLast1[20] = "";             // Данные в введенной строке строке.
int ret = 0;                       // Признак прерывания операции
int lenStr = 0;                    // Длина строки

int st_Power_gaz       = 0;        // Состояние источника питания датчика газа
int st_PowerGeiger     = 0;        // Состояние источника питания датчика Гейгера
bool stop_info         = false;    // Остановить прием информации
bool save_file         = false;    // Записать данные в файл
bool start_save_file   = false;    // Открыть файл для записи    
bool stop_save_file    = false;    // Закрыть файл для записи  

double DOM_LAT         = 55.954994;
double DOM_LON         = 37.231121;
//----------------------------

//Назначение переменных для хранения № опций меню (клавиш)

int but1, but2, but3, but4, but5, but6, but7, but8, but9, but10, butX, butY, but_m1, but_m2, but_m3, but_m4, but_m5, pressed_button;
int kbut1, kbut2, kbut3, kbut4, kbut5, kbut6, kbut7, kbut8, kbut9, kbut0, kbut_save, kbut_clear, kbut_exit;
int kbutA, kbutB, kbutC, kbutD, kbutE, kbutF;
int m2 = 1; // Переменная номера меню

// Адрес радио модуля
#define ADDR "remot"   // Адрес модуля Базы
#define PAYLOAD sizeof(unsigned long)
// Переменная для приёма и передачи данных
unsigned long data = 0;
unsigned long command = 0;
// Флаг для определения выхода по таймауту
boolean timeout = false;
// Таймаут ожидания ответа от сервера - 1 секунда
#define TIMEOUT 1000
// Переменная для запоминания времени отправки
unsigned long timestamp = 0;

//------------------------------------------------------------------------------------------------------------------
// Назначение переменных для хранения текстов

char  txt_menu1_1[]            = "\x89""P""\x86""EM ""\x82\AHH""\x91""X";           // ПРИЕМ ДАННЫХ
char  txt_menu1_2[]            = "\x86""H""\x8B""O GPS";                            // ИНФО GPS
char  txt_menu1_3[]            = "\x89\x86""T""A""H""\x86""E \x81""e""\x9E\x98""epa";                // ПИТАНИЕ Гейгера
char  txt_menu1_4[]            = "\x89\x86""T""A""H""\x86""E ""\x99""a""\xA4"".""\x81""A""\x85";     // ПИТАНИЕ датч. ГАЗА
char  txt_menu2_1[]            = "BBO""\x82"" KOOP""\x82\x86""HAT";                 // ВВОД КООРДИНАТ
char  txt_menu2_2[]            = "BBO""\x82"" B""\x91""COT""\x91";                  // ВВОД ВЫСОТЫ
char  txt_menu2_3[]            = "\x89""OPO""\x81"" PA""\x82\x86""A""\x8C\x86\x86"; // ПОРОГ РАДИАЦИИ
char  txt_menu2_4[]            = "\x89""OPO""\x81"" ""\x81""A""\x85";               // ПОРОГ ГАЗ
char  txt_menu3_1[]            = "CTEPET""\x92"" ""\x8B""A""\x87\x88\x91";          // СТЕРЕТЬ ФАЙЛЫ
char  txt_menu3_2[]            = "BBO""\x82"" KO""\x93\x8B\x8B"".PA""\x82"".";      // ВВОД КОЭФФ. РАД.
char  txt_menu3_3[]            = "\x89""EPE""\x82""A""\x8D""A"" ""\x97"" ""\x89""K";// ПЕРЕДАЧА в ПК
char  txt_menu3_4[]            = "\x86""H""\x8B""O ""\x8A""CTAHOBK""\x86";          //ИНФО УСТАНОВКИ
char  txt_menu4_1[]            = "C\x96poc \x99""a""\xA2\xA2\xABx";                 // Сброс данных
char  txt_menu4_2[]            = "\x8A""c\xA4.N \xA3o\xA0\xAC\x9C.";                // Уст. № польз
char  txt_menu4_3[]            = "\x89""apo\xA0\xAC \xA3o\xA0\xAC\x9C.";            // Пароль польз.
char  txt_menu4_4[]            = "\x89""apo\xA0\xAC a\x99\xA1\x9D\xA2.";            // Пароль админ.
char  txt_menu5_1[]            = "CKAH.PA""\x82\x86""O""\x93\x8B\x86""PA";          // СКАН.РАДИОЭФИРА
char  txt_menu5_2[]            = "B""\x91\x80""OP KAHA""\x88""A";                   // ВЫБОР КАНАЛА
char  txt_menu5_3[]            = "B""\x91\x80""OP MO""\x8F""HOCT""\x86";            // ВЫБОР МОЩНОСТИ
char  txt_menu5_4[]            = "PE""\x84\x86""M ""\x89""APO""\x88\x95";           // РЕЖИМ ПАРОЛЯ
char  txt9[6]                  = "B\x97o\x99";                                      // Ввод
char  txt10[8]                 = "O""\xA4\xA1""e""\xA2""a";                         // "Отмена"
char  txt12[]                  = "B\x97""e\x99\x9D\xA4""e \xA3""apo\xA0\xAC!";      // "Введите пароль"
char  txt_pass_ok[]            = "\xA3""apo\xA0\xAC OK!";                           // Пароль ОК!
char  txt_pass_no[]            = "\xA3""apo\xA0\xAC NO!";                           // Пароль NO!
char  txt_botton_clear[]       = "C\x96poc";                                                         // Сброс
char  txt_botton_otmena[]      = "O""\xA4\xA1""e""\xA2""a";                                          // Отмена
char  txt_system_clear1[]      = "B\xA2\x9D\xA1""a\xA2\x9D""e!";                                     //Внимание !
char  txt_system_clear2[]      = "Bc\xAF \xA1\xA2\xA5op\xA1""a""\xA6\xA1\xAF \x96y\x99""e\xA4";      // Вся информация будет
char  txt_system_clear3[]      = "\x8A\x82""A""\x88""EHA!";                                          // УДАЛЕНА
char  txt_n_user[]             = "B\x97""e\x99\x9D\xA4""e N \xA3o\xA0\xAC\x9C.";                     // Введите № польз.
char  txt_rep_user[]           = "\x89o\x97\xA4op\x9D\xA4""e"" N \xA3o\xA0\xAC\x9C.  ";              // Повторите № польз.
char  txt_set_pass_user[]      = "Ho\x97\xAB\x9E \xA3""apo\xA0\xAC \xA3o\xA0\xAC\x9C.";              // "Новый пароль польз."
char  txt_set_pass_admin[]     = "Ho\x97\xAB\x9E \xA3""apo\xA0\xAC  a\x99\xA1\x9D\xA2.";             // Новый пароль админ.
char  txt_rep_pass_user[]      = "\x89o\x97\xA4op \xA3""apo\xA0\xAC \xA3o\xA0\xAC\x9C.";             // "Повтор пароль польз."
char  txt_err_pass_user[]      = "O\xA8\x9D\x96ka \x97\x97o\x99""a" ;                                // Ошибка ввода
char  txt_rep_pass_admin[]     = "\x89o\x97\xA4op \xA3""apo\xA0\xAC a\x99\xA1\x9D\xA2.";             // "Повтор пароль админ"
char  txt_count1[]             = "B\xA2\x9D\xA1""a\xA2\x9D""e!";                                     // Внимание !
char  txt_count2[]             = "B\x97""e\x99\x9D\xA4""e \xA3o\x9F""a""\x9C""a""\xA2\x9D\xAF";      // Введите показания
char  txt_count3[]             = "He \x96o\xA0\xAC\xA8""e 10 \xA6\x9D\xA5p !";                       // Не больше 10 цифр !
char  txt_count4[]             = "\x89o\x97\xA4op\x9D\xA4""e"" \xA3o\x9F""a""\x9C""a""\xA2\x9D\xAF"; // "Повторите показания"
char  txt_info1[]              = "\x86\xA2\xA5op\xA1""a""\xA6\x9D\xAF";                              // Информация
char  txt_info2[]              = "B""\x97""o""\x99"" ""\x99""a""\xA2\xA2\xAB""x";                    //"Ввод данных"
char  txt_info4[]              = "\x8A""c\xA4""a\xA2o\x97\x9F\x9D c\x9D""c\xA4""e\xA1\xAB";          //
char  txt_info3[]              = "Hac\xA4po\x9E\x9F""a c\x9D""c\xA4""e\xA1\xAB";                     // Настройка системы
char  txt_info5[]              = "\x86\xA2\xA5op\xA1""a""\xA6\x9D\xAF RADIO";                        //
char  txt_mount1[]             = "\x95\xA2\x97""ap\xAC";                                             // Январь
char  txt_mount2[]             = "\x8B""e\x97""pa\xA0\xAC";                                          // Февраль
char  txt_mount3[]             = "Map\xA4";                                                          // Март
char  txt_mount4[]             = "A\xA3pe\xA0\xAC";                                                  // Апрель
char  txt_mount5[]             = "Ma\x9E";                                                           // Май
char  txt_mount6[]             = "\x86\xAE\xA2\xAC";                                                 // Июнь
char  txt_mount7[]             = "\x86\xAE\xA0\xAC";                                                 // Июль
char  txt_mount8[]             = "A\x97\x98yc\xA4";                                                  // Август
char  txt_mount9[]             = "Ce\xA2\xA4\xAF\x96p\xAC";                                          // Сентябрь
char  txt_mount10[]            = "O\x9F\xA4\xAF\x96p\xAC";                                           // Октябрь
char  txt_mount11[]            = "Ho\xAF\x96p\xAC";                                                  // Ноябрь
char  txt_mount12[]            = "\x82""e\x9F""a\x96p\xAC";                                          // Декабрь
char  txt_radiacia[]           = " ***** "; //
char  txt_gaz[]                = "\x82""a""\xA4\xA7\x9D\x9F"" ""\x98""a""\x9C""a";                   // Датчик газа
char  txt_gazOn[]              = " \x82""a""\xA4\xA7\x9D\x9F"" ""\x98""a""\x9C""a BK""\x88"".";      // Датчик газа ВКЛ.
char  txt_gazOff[]             = "\x82""a""\xA4\xA7\x9D\x9F"" ""\x98""a""\x9C""a OTK""\x88"".";      // Датчик газа ОТКЛ.
char  txt_gazNo[]              = "\x82""a""\xA2\xA2\xAB""e ""\xA2""e ""\xA3""o""\xA0""y""\xA7""e""\xA2\xAB"; // Данные не получены
char  txt_On[]                 = "BK""\x88"".";                                                      // ВКЛ.
char  txt_Off[]                = "OTK""\x88"".";                                                     // ОТКЛ.
char  txt_Exit[]               = "B""\xAB""xo""\x99";                                                // Выход
char  txt_geiger[]             = "\x82""a""\xA4\xA7\x9D\x9F"" ""\x81""e""\x9E\x98""epa";             // Датчик Гейгера
char  txt_geigerOn[]           = "\x82""a""\xA4\xA7\x9D\x9F"" ""\x81""e""\x9E\x98""epa  BK""\x88";   // Датчик Гейгера  ВКЛ.
char  txt_geigerOff[]          = "\x82""a""\xA4\xA7\x9D\x9F"" ""\x81""e""\x9E\x98""epa OTK""\x88";   // Датчик Гейгера ОТКЛ.
char  txt_pressure[]           = " ***** "; //
char  txt_elevation[]          = " ***** "; //
char  txt_altitude[]           = " ***** "; //
char  txt_data[]               = "\x82""a\xA4""a";                                                   // Data
char  txt_pred[]               = "\x89pe\x99.";                                                      // Пред.
char  txt_file_save[]          = "\x85""a""\xA3\x9D""ca""\xA4\xAC"" ""\x97"" ""\xA5""a""\x9E\xA0";   // Записать в файл
char  txt_file_stop[]          = "Oc""\xA4""a""\xA2""o""\x97\x9D\xA4\xAC"" ""\x9C""a""\xA3\x9D""c""\xAC"; // Остановить запись
//char  txt_tek[]                = "Te\x9F.";                                                          // Тек.
//char  txt_summa[]              = "Pe\x9C.";                                                          // Рез.
char  txt_return[]             = "\x85""a\x97""ep\xA8\xA2\xA4\xAC \xA3poc\xA1o\xA4p";                // Завершить просмотр
char  txt_info_count[]         = "\x86H\x8BO C\x8D""ET\x8D\x86KOB";                                  //
char  txt_info_n_user[]        = "\x89""p""\x9D""e""\xA1"" ""\x9D\xA2\xA5""op""\xA1""a""\xA6\x9D\x9D"; // Прием информации
char  txt_info_n_user1[]       = "Ho\xA1""ep ""\xA3o\xA0\xAC\x9Co\x97""a""\xA4""e""\xA0\xAF";         // Номер пользователя
//char  txt_info_n_telef[]       = "Ho\xA1""ep ""\xA4""e\xA0""e\xA5o\xA2""a";// Номер телефона
//char  txt_info_n_device[]      = "Ho\xA1""ep ""\xA4""e\xA0""e\xA5o\xA2""a";// Номер телефона
//char  txt_info_n_device1[]     = "B\x97""e\x99\x9D\xA4""e N ""\xA4""e\xA0""e\xA5o\xA2""a";// Введите N телеф.
//char  txt_info_n_device2[]     = "\x89o\x97\xA4op\x9D\xA4""e N ""\xA4""e\xA0""e\xA5o\xA2""a";// Повторите N телеф.
//=====================================================================
void dateTime(uint16_t* date, uint16_t* time)                                                 // Программа записи времени и даты файла
{
  DateTime now = RTC.now();
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());
  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}
void draw_Glav_Menu()
{
  but1 = myButtons.addButton( 10,  20, 250,  35, txt_menu1_1);
  but2 = myButtons.addButton( 10,  65, 250,  35, txt_menu1_2);
  but3 = myButtons.addButton( 10, 110, 250,  35, txt_menu1_3);
  but4 = myButtons.addButton( 10, 155, 250,  35, txt_menu1_4);
  butX = myButtons.addButton(279, 199,  40,  40, "W", BUTTON_SYMBOL); // кнопка Часы
  but_m1 = myButtons.addButton(  10, 199, 45,  40, "1");
  but_m2 = myButtons.addButton(  61, 199, 45,  40, "2");
  but_m3 = myButtons.addButton(  112, 199, 45,  40, "3");
  but_m4 = myButtons.addButton(  163, 199, 45,  40, "4");
  but_m5 = myButtons.addButton(  214, 199, 45,  40, "5");
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_WHITE);
  myGLCD.setColor(0, 255, 0);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("                      ", CENTER, 0);
  test_power();
  switch (m2)
  {
    case 1:
      myGLCD.print(txt_info1, CENTER, 0);
      break;
    case 2:
      myGLCD.print(txt_info2, CENTER, 0);
      break;
    case 3:
      myGLCD.print(txt_info3, CENTER, 0);
      break;
    case 4:
      myGLCD.print(txt_info4, CENTER, 0);
      break;
    case 5:
      myGLCD.print(txt_info5, CENTER, 0);
      break;
  }
  myButtons.drawButtons();
}
// Выбор Меню Тексты меню в строках "txt....."
void swichMenu() // Тексты меню в строках "txt....."
{
  m2 = 1;                                         // Устанивить первую странице меню
  while (1)
  {
    test_power();
    myButtons.setTextFont(BigFont);               // Установить Большой шрифт кнопок

    if (myTouch.dataAvailable() == true)          // Проверить нажатие кнопок
    {
      pressed_button = myButtons.checkButtons();  // Если нажата - проверить что нажато
     if (pressed_button == butX)                 // Нажата вызов часы
      {
        AnalogClock();
        myGLCD.clrScr();
        myButtons.drawButtons();                 // Восстановить кнопки
        print_up();                              // Восстановить верхнюю строку
      }
     if (pressed_button == but_m1)              // Нажата 1 страница меню
      {
        myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_BLUE); // Голубой фон меню
        myButtons.drawButtons();                // Восстановить кнопки
        default_colors = true;
        m2 = 1;                                              // Устанивить первую странице меню
        myButtons.relabelButton(but1, txt_menu1_1, m2 == 1);
        myButtons.relabelButton(but2, txt_menu1_2, m2 == 1);
        myButtons.relabelButton(but3, txt_menu1_3, m2 == 1);
        myButtons.relabelButton(but4, txt_menu1_4, m2 == 1);
        myGLCD.setColor(0, 255, 0);
        myGLCD.setBackColor(0, 0, 0);
        myGLCD.print("                      ", CENTER, 0);
        myGLCD.print(txt_info1, CENTER, 0);            // "Ввод данных"
      }
     if (pressed_button == but_m2)
      {
        myButtons.setButtonColors(VGA_WHITE, VGA_RED, VGA_YELLOW, VGA_BLUE, VGA_TEAL);
        myButtons.drawButtons();
        default_colors = false;
        m2 = 2;
        myButtons.relabelButton(but1, txt_menu2_1 , m2 == 2);
        myButtons.relabelButton(but2, txt_menu2_2 , m2 == 2);
        myButtons.relabelButton(but3, txt_menu2_3 , m2 == 2);
        myButtons.relabelButton(but4, txt_menu2_4 , m2 == 2);
        myGLCD.setColor(0, 255, 0);
        myGLCD.setBackColor(0, 0, 0);
        myGLCD.print("                      ", CENTER, 0);
        myGLCD.print(txt_info2, CENTER, 0);            // Информация
      }
      if (pressed_button == but_m3)
      {
        myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_GREEN);
        myButtons.drawButtons();
        default_colors = false;
        m2 = 3;
        myButtons.relabelButton(but1, txt_menu3_1 , m2 == 3);
        myButtons.relabelButton(but2, txt_menu3_2 , m2 == 3);
        myButtons.relabelButton(but3, txt_menu3_3 , m2 == 3);
        myButtons.relabelButton(but4, txt_menu3_4 , m2 == 3);
        myGLCD.setColor(0, 255, 0);
        myGLCD.setBackColor(0, 0, 0);
        myGLCD.print("                      ", CENTER, 0);
        myGLCD.print(txt_info3, CENTER, 0);            // Информация
      }
      if (pressed_button == but_m4)
      {
        myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_RED);
        myButtons.drawButtons();
        default_colors = false;
        m2 = 4;
        myButtons.relabelButton(but1, txt_menu4_1 , m2 == 4);
        myButtons.relabelButton(but2, txt_menu4_2 , m2 == 4);
        myButtons.relabelButton(but3, txt_menu4_3 , m2 == 4);
        myButtons.relabelButton(but4, txt_menu4_4 , m2 == 4);
        myGLCD.setColor(0, 255, 0);
        myGLCD.setBackColor(0, 0, 0);
        myGLCD.print("                      ", CENTER, 0);
        myGLCD.print(txt_info4, CENTER, 0);            //
      }
      if (pressed_button == but_m5)
      {
        myButtons.setButtonColors(VGA_WHITE, VGA_GRAY, VGA_WHITE, VGA_RED, VGA_NAVY);
        myButtons.drawButtons();
        default_colors = false;
        m2 = 5;
        myButtons.relabelButton(but1, txt_menu5_1 , m2 == 5);
        myButtons.relabelButton(but2, txt_menu5_2 , m2 == 5);
        myButtons.relabelButton(but3, txt_menu5_3 , m2 == 5);
        myButtons.relabelButton(but4, txt_menu5_4 , m2 == 5);
        myGLCD.setColor(0, 255, 0);
        myGLCD.setBackColor(0, 0, 0);
        myGLCD.print("                      ", CENTER, 0);
        myGLCD.print(txt_info5, CENTER, 0);            //
      }

      //*****************  Меню №1  **************

      if (pressed_button == but1 && m2 == 1)
      {
        radiotraffic();                 // если верно - выполнить пункт меню
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }

      if (pressed_button == but2 && m2 == 1)
      {
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }
      if (pressed_button == but3 && m2 == 1)
      {
        menu_Geiger(); // если верно - выполнить пункт меню
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }
      if (pressed_button == but4 && m2 == 1)
      {
        menu_gaz(); // если верно - выполнить пункт меню
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }
      //*****************  Меню №2  **************

      if (pressed_button == but1 && m2 == 2)
      {
      //  print_info();
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }
      if (pressed_button == but2 && m2 == 2)
      {
        //   info_nomer_user();
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }

      if (pressed_button == but3 && m2 == 2)
      {
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }
      if (pressed_button == but4 && m2 == 2)
      {
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }

      //*****************  Меню №3  **************
      if (pressed_button == but1 && m2 == 3) // Первый пункт меню 3
      {
        pass_test_start();  // Нарисовать цифровую клавиатуру
        klav123();          // Считать информацию с клавиатуры
        if (ret == 1)           // Если "Возврат" - закончить
        {
          goto bailout13;  // Перейти на окончание выполнения пункта меню
        }
        else                 // Иначе выполнить пункт меню
        {
          pass_test();     // Проверить пароль
        }
        if (  ( pass1 == 1) || ( pass2 == 1) || ( pass3 == 1)) // если верно - выполнить пункт меню
        {
          myGLCD.clrScr();   // Очистить экран
          myGLCD.print(txt_pass_ok, RIGHT, 208);
          delay (500);
          eeprom_clear == 0;
          system_clear_start(); // если верно - выполнить пункт меню
        }
        else  // Пароль не верный - сообщить и закончить
        {
          txt_pass_no_all();
        }

        bailout13: // Восстановить пункты меню
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }

      //--------------------------------------------------------------
      if (pressed_button == but2 && m2 == 3) // Второй пункт меню 3
      {
        pass_test_start();
        klav123();
        if (ret == 1)
        {
          goto bailout23;
        }
        else
        {
          pass_test();
        }
        if ( ( pass1 == 1) || ( pass2 == 1) || ( pass3 == 1))
        {
          myGLCD.clrScr();
          myGLCD.print(txt_pass_ok, RIGHT, 208);
          set_n_telef();
          delay (500);
        }
        else
        {
          txt_pass_no_all();
        }
        bailout23:
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }

      //------------------------------------------------------------------

      if (pressed_button == but3 && m2 == 3) // Третий пункт меню 3
      {
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }

      //------------------------------------------------------------------
      if (pressed_button == but4 && m2 == 3) // Четвертый пункт меню 3
      {
        pass_test_start();
        klav123();
        if (ret == 1)
        {
          goto bailout43;
        }
        else
        {
          pass_test();
        }
        if ( ( pass1 == 1) || ( pass2 == 1) || ( pass3 == 1)) // если верно - выполнить пункт меню
        {
          myGLCD.clrScr();
          myGLCD.print(txt_pass_ok, RIGHT, 208);
          delay (500);
//          info_nomer_user();
        }
        else
        {
          txt_pass_no_all();
        }
bailout43:
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }

      //*****************  Меню №4  **************

      if (pressed_button == but1 && m2 == 4)   // Сброс данных
      {
        pass_test_start();              // Нарисовать цифровую клавиатуру
        klav123();                      // Считать информацию с клавиатуры
        if (ret == 1)                       // Если "Возврат" - закончить
        {
          goto bailout14;              // Перейти на окончание выполнения пункта меню
        }
        else
        {
          pass_test();
        }
        if ( ( pass2 == 1) || ( pass3 == 1)) // если верно - выполнить пункт меню
        {
          myGLCD.clrScr();              // Очистить экран
          myGLCD.print(txt_pass_ok, RIGHT, 208);
          delay (500);
          eeprom_clear = 1;             // Разрешить стереть информации
          system_clear_start();         // если верно - выполнить пункт меню
        }
        else  // Пароль не верный - сообщить и закончить
        {
          txt_pass_no_all();
        }

bailout14: // Восстановить пункты меню
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }

      if (pressed_button == but2 && m2 == 4)
      {
        pass_test_start();
        klav123();
        if (ret == 1)
        {
          goto bailout24;
        }
        else
        {
          pass_test();
        }
        if ( ( pass1 == 1) || ( pass2 == 1) || ( pass3 == 1))
        {
          myGLCD.clrScr();
          myGLCD.print(txt_pass_ok, RIGHT, 208);
          delay (500);
          set_n_user_start();
        }
        else
        {
          txt_pass_no_all();
        }

        bailout24:
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }

      if (pressed_button == but3 && m2 == 4) // Ввод пароля пользователя
      {
        int  stCurrentLen_pass_user = i2c_eeprom_read_byte( deviceaddress, adr_pass_user - 2); //считать длину пароля  из памяти
        if (stCurrentLen_pass_user == 0)
        {
          pass1 = 1;
          goto pass_cross_user;
        }
        pass_test_start();
        klav123();
        if (ret == 1)
        {
          goto bailout34;
        }
        pass_test();
        pass_cross_user:

        if ( ( pass1 == 1) || ( pass2 == 1) || ( pass3 == 1))
        {
          myGLCD.clrScr();
          myGLCD.print(txt_pass_ok, RIGHT, 208);
          delay (500);
          set_pass_user_start();
        }
        else
        {
          txt_pass_no_all();
        }

bailout34:
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }
      if (pressed_button == but4 && m2 == 4) // Смена пароля администратора
      {
        int stCurrentLen_pass_admin = i2c_eeprom_read_byte( deviceaddress, adr_pass_admin - 2);
        if (stCurrentLen_pass_admin == 0)
        {
          pass2 = 1;
          pass3 = 1;
          goto pass_cross_admin;
        }
        pass_test_start();
        klav123();
        if (ret == 1)
        {
          goto bailout44;
        }
        pass_test();
        pass_cross_admin:

        if (( pass2 == 1) || ( pass3 == 1))
        {
          myGLCD.clrScr();
          myGLCD.print(txt_pass_ok, RIGHT, 208);
          delay (500);
          set_pass_admin_start();
        }
        else
        {
          txt_pass_no_all();
        }

bailout44:
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }
      //*****************  Меню №5  **************

      if (pressed_button == but1 && m2 == 5) // Сброс данных
      {
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }
      if (pressed_button == but2 && m2 == 5)
      {
        pass_test_start();  // Нарисовать цифровую клавиатуру
        klav123();          // Считать информацию с клавиатуры
        if (ret == 1)        // Если "Возврат" - закончить
        {
          goto bailout25;  // Перейти на окончание выполнения пункта меню
        }
        else                 // Иначе выполнить пункт меню
        {
          pass_test();     // Проверить пароль
        }
        if ( ( pass2 == 1) || ( pass3 == 1)) // если верно - выполнить пункт меню
        {
          myGLCD.clrScr();   // Очистить экран
          myGLCD.print(txt_pass_ok, RIGHT, 208);
          delay (500);
          //					  ZigBee_SetH(); // если верно - выполнить пункт меню
          reset_klav();
        }
        else  // Пароль не верный - сообщить и закончить
        {
          txt_pass_no_all();
        }

bailout25:
        myButtons.drawButtons();
        print_up();
      }

      if (pressed_button == but3 && m2 == 5) // Ввод пароля пользователя
      {
        pass_test_start();  // Нарисовать цифровую клавиатуру
        klav123();          // Считать информацию с клавиатуры
        if (ret == 1)        // Если "Возврат" - закончить
        {
          goto bailout35;  // Перейти на окончание выполнения пункта меню
        }
        else                 // Иначе выполнить пункт меню
        {
          pass_test();     // Проверить пароль
        }
        if ( ( pass2 == 1) || ( pass3 == 1)) // если верно - выполнить пункт меню
        {
          myGLCD.clrScr();   // Очистить экран
          myGLCD.print(txt_pass_ok, RIGHT, 208);
          delay (500);
          //					  ZigBee_SetL(); // если верно - выполнить пункт меню
          reset_klav();
        }
        else  // Пароль не верный - сообщить и закончить
        {
          txt_pass_no_all();
        }

bailout35:
        myButtons.drawButtons();
        print_up();
      }

      if (pressed_button == but4 && m2 == 5) // Смена пароля администратора
      {

        pass_test_start();  // Нарисовать цифровую клавиатуру
        klav123();          // Считать информацию с клавиатуры
        if (ret == 1)        // Если "Возврат" - закончить
        {
          goto bailout45;  // Перейти на окончание выполнения пункта меню
        }
        else                 // Иначе выполнить пункт меню
        {
          pass_test();     // Проверить пароль
        }
        if ( ( pass2 == 1) || ( pass3 == 1)) // если верно - выполнить пункт меню
        {
          myGLCD.clrScr();   // Очистить экран
          myGLCD.print(txt_pass_ok, RIGHT, 208);
          delay (500);
          reset_klav();
        }
        else  // Пароль не верный - сообщить и закончить
        {
          txt_pass_no_all();
        }

bailout45:
        myButtons.drawButtons();
        print_up();
      }
    }
  }

}


void all_alarm()
{
  time_flag_start();
  //warm_temp();                    // Проверить температуру
  //warm_gaz();                     // Проверить уровень Газ
}

void reset_klav()
{
  myGLCD.clrScr();
  myButtons.deleteAllButtons();
  but1 = myButtons.addButton( 10,  20, 250,  35, txt_menu5_1);
  but2 = myButtons.addButton( 10,  65, 250,  35, txt_menu5_2);
  but3 = myButtons.addButton( 10, 110, 250,  35, txt_menu5_3);
  but4 = myButtons.addButton( 10, 155, 250,  35, txt_menu5_4);
  butX = myButtons.addButton(279, 199,  40,  40, "W", BUTTON_SYMBOL); // кнопка Часы
  but_m1 = myButtons.addButton(  10, 199, 45,  40, "1");
  but_m2 = myButtons.addButton(  61, 199, 45,  40, "2");
  but_m3 = myButtons.addButton(  112, 199, 45,  40, "3");
  but_m4 = myButtons.addButton(  163, 199, 45,  40, "4");
  but_m5 = myButtons.addButton(  214, 199, 45,  40, "5");

}
void txt_pass_no_all()
{
  myGLCD.clrScr();
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print(txt_pass_no, RIGHT, 208);
  delay (1000);
}

void klav123() // ввод данных с цифровой клавиатуры
{
  ret = 0;

  while (true)
  {
    if (myTouch.dataAvailable())
    {
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      if ((y >= 10) && (y <= 60)) // Upper row
      {
        if ((x >= 10) && (x <= 60)) // Button: 1
        {
          waitForIt(10, 10, 60, 60);
          updateStr('1');
        }
        if ((x >= 70) && (x <= 120)) // Button: 2
        {
          waitForIt(70, 10, 120, 60);
          updateStr('2');
        }
        if ((x >= 130) && (x <= 180)) // Button: 3
        {
          waitForIt(130, 10, 180, 60);
          updateStr('3');
        }
        if ((x >= 190) && (x <= 240)) // Button: 4
        {
          waitForIt(190, 10, 240, 60);
          updateStr('4');
        }
        if ((x >= 250) && (x <= 300)) // Button: 5
        {
          waitForIt(250, 10, 300, 60);
          updateStr('5');
        }
      }

      if ((y >= 70) && (y <= 120)) // Center row
      {
        if ((x >= 10) && (x <= 60)) // Button: 6
        {
          waitForIt(10, 70, 60, 120);
          updateStr('6');
        }
        if ((x >= 70) && (x <= 120)) // Button: 7
        {
          waitForIt(70, 70, 120, 120);
          updateStr('7');
        }
        if ((x >= 130) && (x <= 180)) // Button: 8
        {
          waitForIt(130, 70, 180, 120);
          updateStr('8');
        }
        if ((x >= 190) && (x <= 240)) // Button: 9
        {
          waitForIt(190, 70, 240, 120);
          updateStr('9');
        }
        if ((x >= 250) && (x <= 300)) // Button: 0
        {
          waitForIt(250, 70, 300, 120);
          updateStr('0');
        }
      }
      if ((y >= 130) && (y <= 180)) // Upper row
      {
        if ((x >= 10) && (x <= 130)) // Button: Clear
        {
          waitForIt(10, 130, 120, 180);
          stCurrent[0] = '\0';
          stCurrentLen = 0;
          myGLCD.setColor(0, 0, 0);
          myGLCD.fillRect(0, 224, 319, 239);
        }
        if ((x >= 250) && (x <= 300)) // Button: Exit
        {
          waitForIt(250, 130, 300, 180);
          myGLCD.clrScr();
          myGLCD.setBackColor(VGA_BLACK);
          ret = 1;
          stCurrent[0] = '\0';
          stCurrentLen = 0;
          break;
        }
        if ((x >= 130) && (x <= 240)) // Button: Enter
        {
          waitForIt(130, 130, 240, 180);
          if (stCurrentLen > 0)
          {
            for (x = 0; x < stCurrentLen + 1; x++)
            {
              stLast[x] = stCurrent[x];
            }
            stCurrent[0] = '\0';
            stLast[stCurrentLen + 1] = '\0';
            i2c_eeprom_write_byte(deviceaddress, adr_stCurrentLen1, stCurrentLen);
            stCurrentLen1 = stCurrentLen;
            stCurrentLen = 0;
            myGLCD.setColor(0, 0, 0);
            myGLCD.fillRect(0, 208, 319, 239);
            myGLCD.setColor(0, 255, 0);
            myGLCD.print(stLast, LEFT, 208);
            break;
          }
          else
          {
            myGLCD.setColor(255, 0, 0);
            myGLCD.print("\x80\x8A\x8B\x8B""EP \x89\x8A""CTO\x87!", CENTER, 192);//"БУФФЕР ПУСТОЙ!"
            delay(500);
            myGLCD.print("                ", CENTER, 192);
            delay(500);
            myGLCD.print("\x80\x8A\x8B\x8B""EP \x89\x8A""CTO\x87!", CENTER, 192);//"БУФФЕР ПУСТОЙ!"
            delay(500);
            myGLCD.print("                ", CENTER, 192);
            myGLCD.setColor(0, 255, 0);
          }
        }
      }
    }
  }
}

void drawButtons1() // Отображение цифровой клавиатуры
{
  // Draw the upper row of buttons
  for (x = 0; x < 5; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10 + (x * 60), 10, 60 + (x * 60), 60);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10 + (x * 60), 10, 60 + (x * 60), 60);
    myGLCD.printNumI(x + 1, 27 + (x * 60), 27);
  }
  // Draw the center row of buttons
  for (x = 0; x < 5; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10 + (x * 60), 70, 60 + (x * 60), 120);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10 + (x * 60), 70, 60 + (x * 60), 120);
    if (x < 4)
      myGLCD.printNumI(x + 6, 27 + (x * 60), 87);
  }

  myGLCD.print("0", 267, 87);
  // Draw the lower row of buttons
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (10, 130, 120, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10, 130, 120, 180);
  myGLCD.print(txt_botton_clear, 25, 147);//"Сброс"

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (130, 130, 240, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (130, 130, 240, 180);
  myGLCD.print("B\x97o\x99", 155, 147);// "Ввод"

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (250, 130, 300, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (250, 130, 300, 180);
  myGLCD.print("HET", 252, 147);// "HET"
  myGLCD.setBackColor (0, 0, 0);
}

void updateStr(int val)
{
  if (stCurrentLen < 20)
  {
    stCurrent[stCurrentLen] = val;
    stCurrent[stCurrentLen + 1] = '\0';
    stCurrentLen++;
    myGLCD.setColor(0, 255, 0);
    myGLCD.print(stCurrent, LEFT, 224);
  }
  else
  { // Вывод строки "ПЕРЕПОЛНЕНИЕ!"
    myGLCD.setColor(255, 0, 0);
    myGLCD.print("\x89""EPE""\x89O\x88HEH\x86""E!", CENTER, 224);// ПЕРЕПОЛНЕНИЕ!
    delay(500);
    myGLCD.print("              ", CENTER, 224);
    delay(500);
    myGLCD.print("\x89""EPE""\x89O\x88HEH\x86""E!", CENTER, 224);// ПЕРЕПОЛНЕНИЕ!
    delay(500);
    myGLCD.print("              ", CENTER, 224);
    myGLCD.setColor(0, 255, 0);
  }
}

// Draw a red frame while a button is touched
void waitForIt(int x1, int y1, int x2, int y2)
{
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
  myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
}

void InitializingSD()
{
  Serial.println("\nInitializing SD card...");
  if (!card.init(SPI_HALF_SPEED, chipSelect))
  {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card is inserted?");
    Serial.println("* Is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  }
  else
  {
    Serial.println("Wiring is correct and a card is present.");
  }
  Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) 
  {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}
// Примеры работы с SD
void DumpFileSD()
{
  /*
    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
    }
    Serial.println("card initialized.");

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt");

    // if the file is available, write to it:
    if (dataFile) {
    while (dataFile.available()) {
    Serial.write(dataFile.read());
    }
    dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else {
    Serial.println("error opening datalog.txt");
    }
  */
}
void FilesSD()
{
  /*
    if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    return;
    }
    Serial.println("initialization done.");

    if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
    }
    else {
    Serial.println("example.txt doesn't exist.");
    }

    // open a new file and immediately close it:
    Serial.println("Creating example.txt...");
    myFile = SD.open("example.txt", FILE_WRITE);
    myFile.close();

    // Check to see if the file exists:
    if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
    }
    else {
    Serial.println("example.txt doesn't exist.");
    }

    // delete the file:
    Serial.println("Removing example.txt...");
    SD.remove("example.txt");

    if (SD.exists("example.txt")){
    Serial.println("example.txt exists.");
    }
    else {
    Serial.println("example.txt doesn't exist.");
    }
  */
}
void setup_printDirectorySD()
{
  /*
    if (!SD.begin(53)) {
  	Serial.println("initialization failed!");
  	return;
    }
    Serial.println("initialization done.");

    // root = SD.open("/");

    myFile = SD.open("/");

    printDirectory(myFile, 0);

    Serial.println("done!");
  */
}
void printDirectory(File dir, int numTabs)
{
  /*
    while(true) {

    File entry =  dir.openNextFile();
    if (! entry) {
     // no more files
     //Serial.println("**nomorefiles**");
     break;
    }
    for (uint8_t i=0; i<numTabs; i++) {
     Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
     Serial.println("/");
     printDirectory(entry, numTabs+1);
    } else {
     // files have sizes, directories do not
     Serial.print("\t\t");
     Serial.println(entry.size(), DEC);
    }
    }
  */
}
//void ReadWriteSD()
//{
//  //
//  // if (!SD.begin(53))
//  // {
//  //Serial.println("initialization failed ReadWrite!");
//  //return;
//  // }
//  // Serial.println("initialization done.");
//
//  // open the file. note that only one file can be open at a time,
//  // so you have to close this one before opening another.
//  myFile = SD.open("test.txt", FILE_WRITE);
//
//  // if the file opened okay, write to it:
//  if (myFile) {
//    Serial.print("Writing to test.txt...");
//    myFile.println("testing 1, 2, 3.");
//    // close the file:
//    myFile.close();
//    Serial.println("done.");
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening test.txt");
//  }
//
//  // re-open the file for reading:
//  myFile = SD.open("test.txt");
//  if (myFile) {
//    Serial.println("test.txt:");
//
//    // read from the file until there's nothing else in it:
//    while (myFile.available()) {
//      Serial.write(myFile.read());
//    }
//    // close the file:
//    myFile.close();
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening test.txt");
//  }
//
//}
void create_fileSD()
{
  if (!SD.begin(53))
  {
    Serial.println("initialization failed ReadWrite!");
    return;
  }
}

void pass_test_start() // Начало проверки пароля
{
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.clrScr();
  drawButtons1();
  // Вывод строки "Введите пароль!"
  myGLCD.setColor(255, 0, 0);
  myGLCD.print(txt12, CENTER, 192);// Введите пароль!"
  delay(300);
  myGLCD.print("                   ", CENTER, 192);
  delay(300);
  myGLCD.print(txt12, CENTER, 192);// Введите пароль!"

}
void pass_test()
{

  pass = 0;
  pass1 = 0;
  pass2 = 0;
  pass3 = 0;

  //считать пароль pass_user из памяти
  int  stCurrentLen_pass_user = i2c_eeprom_read_byte( deviceaddress, adr_pass_user - 2); //считать длину пароля  из памяти
  //	 Serial.print("stCurrentLen_pass - ");// контроль пароля -удалить
  //     Serial.println(stCurrentLen_pass);// контроль пароля -удалить

  // if ((stCurrentLen_pass == 0) stCurrentLen_pass_user = 1))

  for (int z = 0; z < stCurrentLen_pass_user; z++)
  {
    pass_user[z] = i2c_eeprom_read_byte( deviceaddress, adr_pass_user + z);
  }
  pass_user[stCurrentLen_pass_user]  =  '\0';

  // Проверка пароля пользователя
  if (strcmp(pass_user, stLast) == 0)
  {
    pass1 = 1;
    //  pass2 = 0;
    //  pass3 = 0;
  }
  if (strcmp(pass_user, stLast) != 0)
  {
    pass1 = 0;
  }
  // Окончание проверки пароля пользователя


  //Восстановление длины строки пароля администратора
  int stCurrentLen_pass_admin = i2c_eeprom_read_byte( deviceaddress, adr_pass_admin - 2); //считать длину пароля  из памяти
  // Восстановление пароля администратора
  for (int z = 0; z < stCurrentLen_pass_admin; z++)
  {
    pass_admin[z] = i2c_eeprom_read_byte( deviceaddress, adr_pass_admin + z);
  }
  pass_admin[stCurrentLen_pass_admin]  =  '\0';

  // Проверка пароля администратора
  if (strcmp(pass_admin, stLast) == 0)
  {
    //  pass1 = 0;
    pass2 = 1;
    // pass3 = 0;
  }
  if (strcmp(pass_admin, stLast) != 0)
  {
    pass2 = 0;
  }

  // Окончание проверки пароля администратора

  // Проверка пароля супер администратора

  if (strcmp(pass_super_admin, stLast) == 0)
  {
    // pass1 = 0;
    // pass2 = 0;
    pass3 = 1;

  }
  if (strcmp(pass_super_admin, stLast) != 0)
  {
    pass3 = 0;
  }

  // Окончание проверки пароля супер администратора

  if (( pass1 == 1) || ( pass2 == 1) || ( pass3 == 1))

  {
    pass = 1;
  }
  else
  {
    pass = 0;
  }
}
void system_clear_start()
{
  /*
  	if (ret == 1)
  		{
  			ret = 0;
  			return;
  		}
  */
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.clrScr();
  // txt_set_pass_user  Вывод строки "Введите пароль пользователя!"
  myGLCD.setColor(255, 255, 255);

  myGLCD.print(txt_system_clear1, CENTER, 70);       // Внимание !
  myGLCD.print(txt_system_clear2, CENTER, 100);      // Вся информация будет
  myGLCD.setColor(255, 0, 0);
  myGLCD.print(txt_system_clear3, CENTER, 130);       // УДАЛЕНА

  delay(500);
  myGLCD.print("                        ", CENTER, 70);
  myGLCD.print("                        ", CENTER, 100);
  myGLCD.print("                        ", CENTER, 130);
  delay(500);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print(txt_system_clear1, CENTER, 70);           // Внимание !
  myGLCD.print(txt_system_clear2, CENTER, 100);          // Вся информация будет
  myGLCD.setColor(255, 0, 0);
  myGLCD.print(txt_system_clear3, CENTER, 130);          // УДАЛЕНА

  delay(3000);
  myGLCD.clrScr();

  myGLCD.setBackColor(0, 0, 255);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (10, 130, 150, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10, 130, 150, 180);
  myGLCD.print(txt_botton_otmena, 30, 147);//"Отмена"

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (160, 130, 300, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (160, 130, 300, 180);
  myGLCD.print(txt_botton_clear, 190, 147);// "Сброс"

  while (true)
  {
    if (myTouch.dataAvailable())
    {
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();


      if ((y >= 130) && (y <= 180)) // Upper row
      {
        if ((x >= 10) && (x <= 150)) // Button: "Отмена"
        {
          break;
        }
        if ((x >= 160) && (x <= 300)) // Button: "Сброс"
        {
          myGLCD.setColor(0, 0, 255);
          myGLCD.fillRoundRect (0, 70, 319, 105);
          myGLCD.setColor(255, 255, 255);
          myGLCD.drawRoundRect (0, 70, 319, 105);
          if (eeprom_clear == 1)

          { // write a 0 to all 512 bytes of the EEPROM
            for (int i = 0; i < 319; i++)
            {
              i2c_eeprom_write_byte(deviceaddress, i, 0);
              myGLCD.print(">", i, 80); // "Сброс Ok!"
              //delay (10);
            }
          }
          else
          {
            for (int i = 0; i < 319; i++)
            {
              myGLCD.print("<", i, 80); // "Сброс Ok!"
              //delay (10);
            }

          }
          Serial.println("Removing elektro.txt...");
          SD.remove("elektro.txt");

          if (SD.exists("elektro.txt"))
          {
            Serial.println("elektro.txt exists.");
          }
          else
          {
            Serial.println("elektro.txt doesn't exist.");
          }
          //	 n_str_electro = 0; // Устанавливаем № строки 1
          // разбираем
          //hi=highByte(n_str_electro);
          //low=lowByte(n_str_electro);
          //// тут мы эти hi,low можем сохранить EEPROM
          //i2c_eeprom_write_byte(deviceaddress,adr_n_str_electro, hi);
          //i2c_eeprom_write_byte(deviceaddress,adr_n_str_electro+1, low);

          // gaz.txt

          Serial.println("Removing gaz.txt...");
          SD.remove("gaz.txt");

          if (SD.exists("gaz.txt"))
          {
            Serial.println("gaz.txt exists.");
          }
          else
          {
            Serial.println("gaz.txt doesn't exist.");
          }

          //   n_str_gaz = 0; // Устанавливаем № строки 1
          //  // разбираем
          //hi=highByte(n_str_gaz);
          //low=lowByte(n_str_gaz);
          //// тут мы эти hi,low можем сохранить EEPROM
          //i2c_eeprom_write_byte(deviceaddress,adr_n_str_gaz, hi);
          //i2c_eeprom_write_byte(deviceaddress,adr_n_str_gaz+1, low);

          // colwater.txt
          Serial.println("Removing colwater.txt...");
          SD.remove("colwater.txt");

          if (SD.exists("colwater.txt"))
          {
            Serial.println("colwater.txt exists.");
          }
          else
          {
            Serial.println("colwater.txt doesn't exist.");
          }

          //   n_str_colwater = 0; // Устанавливаем № строки 1
          //  // разбираем
          //hi=highByte(n_str_colwater);
          //low=lowByte(n_str_colwater);
          //// тут мы эти hi,low можем сохранить EEPROM
          //i2c_eeprom_write_byte(deviceaddress,adr_n_str_colwater, hi);
          //i2c_eeprom_write_byte(deviceaddress,adr_n_str_colwater+1, low);


          // hotwater.txt
          Serial.println("Removing hotwater.txt...");
          SD.remove("hotwater.txt");

          if (SD.exists("hotwater.txt"))
          {
            Serial.println("hotwater.txt exists.");
          }
          else
          {
            Serial.println("hotwater.txt doesn't exist.");
          }

          //   n_str_hotwater = 0; // Устанавливаем № строки 1
          //  // разбираем
          //hi=highByte(n_str_hotwater);
          //low=lowByte(n_str_hotwater);
          //// тут мы эти hi,low можем сохранить EEPROM
          //i2c_eeprom_write_byte(deviceaddress,adr_n_str_hotwater, hi);
          //i2c_eeprom_write_byte(deviceaddress,adr_n_str_hotwater+1, low);
          myGLCD.print("C\x96poc OK!", 100, 80);// "Сброс Ok!"
          delay (1000);
          break;

        }

      }
    }
  }
  eeprom_clear = 0;
}


void set_n_user_start()
{
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.clrScr();
  drawButtons1();
  // Вывод строки "Введите номер пользователя!"
  myGLCD.setColor(255, 0, 0);
  myGLCD.print(txt_n_user, CENTER, 192);// Введите номер пользователя!
  delay(300);
  myGLCD.print("                       ", CENTER, 192);
  delay(300);
  myGLCD.print(txt_n_user, CENTER, 192);// Введите номер пользователя!
  klav123();
  if (ret == 1)
  {
    ret = 0;
    return;
  }

  strcpy(temp_stLast, stLast);

  myGLCD.setColor(255, 0, 0);
  myGLCD.print("                         ", CENTER, 192);
  myGLCD.print(txt_rep_user, CENTER, 192);// Введите номер пользователя!
  delay(300);
  myGLCD.print("                         ", CENTER, 192);
  delay(300);
  myGLCD.print(txt_rep_user, CENTER, 192);// Введите номер пользователя!

  klav123();
  if (ret == 1)
  {
    ret = 0;
    return;
  }

  if (strcmp(temp_stLast, stLast) == 0)
  {
    stCurrentLen1 = i2c_eeprom_read_byte( deviceaddress, adr_stCurrentLen1); // Чтение номера пользователя

    for (x = 0; x < stCurrentLen1 + 1; x++)
    {
      i2c_eeprom_write_byte(deviceaddress, adr_n_user + x, stLast[x]);
    }
    i2c_eeprom_write_byte(deviceaddress, adr_n_user - 2, stCurrentLen1);

  }
  if (strcmp(temp_stLast, stLast) != 0)
  {
    myGLCD.print("                      ", CENTER, 192);
    myGLCD.print(txt_err_pass_user, CENTER, 192);// Ошибка ввода!
    delay(1500);
  }

}
void set_n_telef()
{
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.clrScr();
  drawButtons1();
  // Вывод строки "Введите N телефона."
  myGLCD.setColor(255, 0, 0);
//  myGLCD.print(txt_info_n_device1, CENTER, 192);// Введите N телефона.
  delay(300);
  myGLCD.print("                         ", CENTER, 192);
  delay(300);
//  myGLCD.print(txt_info_n_device1, CENTER, 192);// Введите N телефона.
  klav123();
  if (ret == 1)
  {
    ret = 0;
    return;
  }

  strcpy(temp_stLast, stLast);
  myGLCD.setColor(255, 0, 0);
  myGLCD.print("                         ", CENTER, 192);
//  myGLCD.print(txt_info_n_device2, CENTER, 192);// Введите N телефона.
  delay(300);
  myGLCD.print("                        ", CENTER, 192);
  delay(300);
//  myGLCD.print(txt_info_n_device2, CENTER, 192);// Введите N телефона.

  klav123();
  if (ret == 1)
  {
    ret = 0;
    return;
  }

  if (strcmp(temp_stLast, stLast) == 0)
  {
    // stCurrentLen1 = i2c_eeprom_read_byte( deviceaddress,adr_stCurrentLen1);// Чтение номера пользователя

    for (x = 0; x < 20; x++)
    {
      i2c_eeprom_write_byte(deviceaddress, adr_n_telef + x, 0);
    }

    for (x = 0; x < stCurrentLen1 + 1; x++)
    {
      i2c_eeprom_write_byte(deviceaddress, adr_n_telef + x, stLast[x]);
    }

    i2c_eeprom_write_byte(deviceaddress, adr_n_telef - 2, stCurrentLen1);

  }

  if (strcmp(temp_stLast, stLast) != 0 )
  {
    myGLCD.print("                        ", CENTER, 192);
    myGLCD.print(txt_err_pass_user, CENTER, 192);// Ошибка ввода!
    delay(1500);
  }


}

void set_pass_user_start()
{
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.clrScr();
  drawButtons1();
  // txt_set_pass_user  Вывод строки "Введите пароль пользователя!"
  myGLCD.setColor(255, 0, 0);
  myGLCD.print(txt_set_pass_user, CENTER, 192);// Введите пароль пользователя!
  delay(300);
  myGLCD.print("                         ", CENTER, 192);
  delay(300);
  myGLCD.print(txt_set_pass_user, CENTER, 192);// Введите пароль пользователя!

  klav123();
  if (ret == 1)
  {
    ret = 0;
    return;
  }

  //проверка верности пароля

  strcpy(temp_stLast, stLast);

  // txt_set_pass_user  Вывод строки "Введите пароль пользователя!"
  myGLCD.setColor(255, 0, 0);
  myGLCD.print(txt_rep_pass_user, CENTER, 192);// Повтор пароль пользователя!
  delay(300);
  myGLCD.print("                        ", CENTER, 192);
  delay(300);
  myGLCD.print(txt_rep_pass_user, CENTER, 192);// Повтор пароль пользователя!

  klav123();
  if (ret == 1)
  {
    ret = 0;
    return;
  }

  if (strcmp(temp_stLast, stLast) == 0)
  {

    for (x = 0; x < stCurrentLen1 + 1; x++)
    {
      i2c_eeprom_write_byte(deviceaddress, adr_pass_user + x, stLast[x]);
    }
    i2c_eeprom_write_byte(deviceaddress, adr_pass_user - 2, stCurrentLen1);
    myGLCD.print("                      ", CENTER, 192);
    myGLCD.print(txt_pass_ok, RIGHT, 208);
    delay(1500);
  }

  if (strcmp(temp_stLast, stLast) != 0)
  {
    myGLCD.print("                      ", CENTER, 192);
    myGLCD.print(txt_err_pass_user, CENTER, 192);// Ошибка ввода!
    delay(1500);
  }


}
void set_pass_admin_start()
{
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.clrScr();
  drawButtons1();
  // txt_set_pass_admin  Вывод строки "Введите пароль админ!"
  myGLCD.setColor(255, 0, 0);
  myGLCD.print(txt_set_pass_admin, CENTER, 192);// Введите пароль админ!
  delay(300);
  myGLCD.print("                       ", CENTER, 192);
  delay(300);
  myGLCD.print(txt_set_pass_admin, CENTER, 192);// Введите пароль админ!
  //char* temp_stLast = "";
  klav123();
  if (ret == 1)
  {
    ret = 0;
    return;
  }
  //проверка верности пароля

  strcpy(temp_stLast, stLast);

  // txt_set_pass_admin  Вывод строки "Введите пароль админ!"
  myGLCD.setColor(255, 0, 0);
  myGLCD.print(txt_rep_pass_admin, CENTER, 192);// Повтор пароль админ!
  delay(300);
  myGLCD.print("                       ", CENTER, 192);
  delay(300);
  myGLCD.print(txt_rep_pass_admin, CENTER, 192);// Повтор пароль админ!
  klav123();

  if (ret == 1)
  {
    ret = 0;
    return;
  }
  if (strcmp(temp_stLast, stLast) == 0)
  {
    for (x = 0; x < stCurrentLen1 + 1; x++)
    {
      i2c_eeprom_write_byte(deviceaddress, adr_pass_admin + x, stLast[x]);
    }
    i2c_eeprom_write_byte(deviceaddress, adr_pass_admin - 2, stCurrentLen1);
    myGLCD.print("                       ", CENTER, 192);
    myGLCD.print(txt_pass_ok, RIGHT, 208);
    delay(1500);
  }

  else
  {
    myGLCD.print("                       ", CENTER, 192);
    myGLCD.print(txt_err_pass_user, CENTER, 192);// Ошибка ввода!
    delay(1500);
  }

}


void print_info()                  // Вывод информации со счетчиков
{
  int x = 115;
  int y = 15;
  int i = x + 145;
  int z = 16;

  myGLCD.clrScr();   // Очистить экран CENTER

  read_data_eeprom();

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (2, 2, 318, 25);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (2, 2, 318, 25);
  myGLCD.setBackColor(0, 0, 255);

  clock_read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.print(txt_data, CENTER, y - 10);


  if (date < 10)
  {
    myGLCD.printNumI(date, x - 20, y - 10);
  }
  else
  {
    myGLCD.printNumI(date, x - 35, y - 10);
  }


  switch (mon)
  {
    case 1:
      myGLCD.print (txt_mount1, x, y - 10);
      i = x + 5 + (6 * z);
      break;
    case 2:
      myGLCD.print (txt_mount2, x, y - 10);
      i = x + 5 + (7 * z);
      break;
    case 3:
      myGLCD.print (txt_mount3, x, y - 10);
      i = x + 5 + (5 * z);
      break;
    case 4:
      myGLCD.print (txt_mount4, x, y - 10);
      i = x + 5 + (6 * z);
      break;
    case 5:
      myGLCD.print (txt_mount5, x, y - 10);
      i = x + 5 + (3 * z);
      break;
    case 6:
      myGLCD.print (txt_mount6, x, y - 10);
      i = x + 5 + (4 * z);
      break;
    case 7:
      myGLCD.print (txt_mount7, x, y - 10);
      i = x + 5 + (4 * z);
      break;
    case 8:
      myGLCD.print (txt_mount8, x, y - 10);
      i = x + 5 + (6 * z);
      break;
    case 9:
      myGLCD.print (txt_mount9, x, y - 10);
      i = x + 5 + (8 * z);
      break;
    case 10:
      myGLCD.print (txt_mount10, x, y - 10);
      i = x + 5 + (7 * z);
      break;
    case 11:
      myGLCD.print (txt_mount11, x, y - 10);
      i = x + 5 + (6 * z);
      break;
    case 12:
      myGLCD.print (txt_mount12, x, y - 10);
      i = x + 5 + (7 * z);
      break;
  }
  myGLCD.printNumI(year, i, y - 10);

  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 255, 0);
  myGLCD.print(txt_pred, LEFT, y + 20);
//  myGLCD.print(txt_tek, CENTER, y + 20);
//  myGLCD.print(txt_summa , RIGHT, y + 20);

  myGLCD.setColor(255, 127, 0);
  myGLCD.drawLine(1, y + 38, 319, y + 38);
  myGLCD.setColor(255, 255, 0);
  myGLCD.print(txt_radiacia, LEFT, y + 40);
  myGLCD.setColor(255, 255, 255);
  /*				myGLCD.printNumI(count_electro_old, LEFT, y+60);
  				myGLCD.printNumI(count_electro_ok, CENTER, y+60);
  				myGLCD.printNumI(count_electro_ok - count_electro_old , RIGHT, y+60);
  */
  myGLCD.setColor(255, 127, 0);
  myGLCD.drawLine(1, y + 78, 319, y + 78);

  myGLCD.setColor(150, 150, 255);
  //	myGLCD.setColor(VGA_BLUE);
  myGLCD.print(txt_gaz, LEFT, y + 80);
  myGLCD.setColor(255, 255, 255);
  /*			myGLCD.printNumI(count_gaz_old, LEFT, y+100);
  			myGLCD.printNumI(count_gaz_ok, CENTER, y+100);
  			myGLCD.printNumI(count_gaz_ok - count_gaz_old , RIGHT, y+100);*/

  myGLCD.setColor(255, 127, 0);
  myGLCD.drawLine(1, y + 118, 319, y + 118);

  myGLCD.setColor(0, 255, 0);
  myGLCD.print(txt_pressure, LEFT, y + 120);
  myGLCD.setColor(255, 255, 255);
  /*					myGLCD.printNumI(count_colwater_old, LEFT, y+140);
  					myGLCD.printNumI(count_colwater_ok, CENTER, y+140);
  					myGLCD.printNumI(count_colwater_ok - count_colwater_old , RIGHT, y+140);*/

  myGLCD.setColor(255, 127, 0);
  myGLCD.drawLine(1, y + 158, 319, y + 158);

  //myGLCD.setColor(255, 0, 0);
  myGLCD.setColor(VGA_RED);
  myGLCD.print(txt_elevation, LEFT, y + 160);
  myGLCD.setColor(255, 255, 255);
  /*					myGLCD.printNumI(count_hotwater_old, LEFT, y+180);
  					myGLCD.printNumI(count_hotwater_ok, CENTER, y+180);
  					myGLCD.printNumI(count_hotwater_ok - count_hotwater_old , RIGHT, y+180);
  */
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (2, 216, 318, 238);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (2, 216, 318, 238);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print(txt_return, CENTER, 218);// Завершить просмотр

  while (true)
  {
    all_alarm();
    if (myTouch.dataAvailable())
    {
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();

      if ((y >= 2) && (y <= 240)) // Upper row
        //if ((y>=216) && (y<=240))  // Upper row
      {
        if ((x >= 2) && (x <= 319)) // Выход
        {
          waitForIt(10, 10, 60, 60);
          return;
        }
      }
    }
  }
}

void print_up() // Печать верхней строчки над меню
{
  myGLCD.setColor(0, 255, 0);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("                      ", CENTER, 0);
  switch (m2)
  {
    case 1:
      myGLCD.print(txt_info1, CENTER, 0);
      break;
    case 2:
      myGLCD.print(txt_info2, CENTER, 0);
      break;
    case 3:
      myGLCD.print(txt_info3, CENTER, 0);
      break;
    case 4:
      myGLCD.print(txt_info4, CENTER, 0);
      break;
    case 5:
      myGLCD.print(txt_info5, CENTER, 0);
      break;
  }
}

void radiotraffic()
{
  myGLCD.clrScr();                                          // Очистить экран CENTER
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (2, 2, 318, 25);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (2, 2, 318, 25);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.print(txt_info_n_user, CENTER, 5);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (2, 196, 318, 238);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (2, 196, 318, 238);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (299, 59, 313, 73);                 // Индикатор питания счетчика Гейгера
  myGLCD.drawRoundRect (299, 99, 313, 113);                // Индикатор питания датчика газа
  myGLCD.print(txt_file_save, CENTER, 207);                // Записать в файл
  stop_info = false;

  while (true)
  {
    myGLCD.setBackColor(0, 0, 0);
    timeout = false;
    Mirf.setTADDR((byte *)&"fly10");                // Устанавливаем адрес передачи
    myGLCD.print("fly10", LEFT, 40);
  //  Serial.println("Request millis()");            // Запрашиваем число милисекунд, прошедших с последней перезагрузки сервера:

    command = 1;
    myGLCD.printNumI(command, 226, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    timestamp = millis();                          // Запомнили время отправки:
    waitanswer();                                  // Запускаем профедуру ожидания ответа
    exit_file_save();                              // Проверка состояния кнопок
	if(stop_info == true)
	{
		stop_info = false;
		return;
	}
    command = 2;
    myGLCD.printNumI(command, 226, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    // Запомнили время отправки:
    timestamp = millis();
    // Запускаем профедуру ожидания ответа
    waitanswer();

    if (geiger_ready == true)
    {
      Serial.print("cpm = ");

      command = 3;
      myGLCD.printNumI(command, 226, 40);
      myGLCD.print("->", 240, 40);
      Mirf.send((byte *)&command);
      delay(100);
      myGLCD.print("    ", 210, 40);
      timestamp = millis();                      // Запомнили время отправки:
      waitanswer();                              // Запускаем процедуру ожидания ответа
      exit_file_save();                              // Проверка состояния кнопок
	  if(stop_info == true)
       {
	   	 stop_info = false;
		 return;
	   }
      Serial.print("uSv/h = ");

      command = 4;
      myGLCD.printNumI(command, 226, 40);
      myGLCD.print("->", 240, 40);
      Mirf.send((byte *)&command);
      delay(100);
      myGLCD.print("    ", 210, 40);
      // Запомнили время отправки:
      timestamp = millis();
      // Запускаем профедуру ожидания ответа
      waitanswer();
      exit_file_save();                              // Проверка состояния кнопок
	  if(stop_info == true)
       {
	   	 stop_info = false;
		 return;
	   }
    }
    exit_file_save();                                // Проверка состояния кнопок
	  if(stop_info == true)
       {
	   	 stop_info = false;
		 return;
	   }
    command = 5;
    myGLCD.printNumI(command, 226, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    // Запомнили время отправки:
    timestamp = millis();
    // Запускаем профедуру ожидания ответа
    waitanswer();
	exit_file_save();                                // Проверка состояния кнопок
	if(stop_info == true)
    {
	   	stop_info = false;
		return;
	}
    command = 6;
    myGLCD.printNumI(command, 226, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    // Запомнили время отправки:
    timestamp = millis();
    waitanswer();                                    // Запускаем профедуру ожидания ответа
    exit_file_save();                              // Проверка состояния кнопок
	if(stop_info == true)
    {
	   	stop_info = false;
		return;
	}
	command = 7;
    myGLCD.printNumI(command, 226, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    // Запомнили время отправки:
    timestamp = millis();                             
    // Запускаем профедуру ожидания ответа
    waitanswer();                                   // Запускаем профедуру ожидания ответа                                     
    exit_file_save();                              // Проверка состояния кнопок
	if(stop_info == true)
    {
	   	stop_info = false;
		return;
	}
	command = 8;
    myGLCD.printNumI(command, 226, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    // Запомнили время отправки:
    timestamp = millis();
    waitanswer();                                   // Запускаем профедуру ожидания ответа 
    exit_file_save();                              // Проверка состояния кнопок
	if(stop_info == true)
    {
	   	stop_info = false;
		return;
	}
	command = 9;
    myGLCD.printNumI(command, 226, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    timestamp = millis();                   // Запомнили время отправки:
    waitanswer();                           // Запускаем профедуру ожидания ответа
    exit_file_save();                              // Проверка состояния кнопок
	if(stop_info == true)
    {
	   	stop_info = false;
		return;
	}
    command = 10;
    myGLCD.printNumI(command, 210, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    timestamp = millis();                  // Запомнили время отправки:
    waitanswer();                          // Запускаем профедуру ожидания ответа
    exit_file_save();                              // Проверка состояния кнопок
	if(stop_info == true)
    {
	   	stop_info = false;
		return;
	}
    command = 11;
    myGLCD.printNumI(command, 210, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    timestamp = millis();                  // Запомнили время отправки:
    waitanswer();                          // Запускаем профедуру ожидания ответа
    exit_file_save();                              // Проверка состояния кнопок
	if(stop_info == true)
    {
	   	stop_info = false;
		return;
	}
	command = 14;                          // Состояние питания датчика газа
	st_Power_gaz = 0;
    myGLCD.printNumI(command, 210, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    timestamp = millis();                  // Запомнили время отправки:
    waitanswer();                          // Запускаем профедуру ожидания ответа
	if (st_Power_gaz == 2 )                           // Питание включено
	{
		myGLCD.setColor(0, 255, 0);
    	myGLCD.fillRoundRect  (300, 100, 312, 112);   // Индикатор питания датчика газа
		myGLCD.setColor(255, 255, 255);
	}
	else if (st_Power_gaz == 1 )                      // Питание отключено
	{
		myGLCD.setColor(255, 0, 0);
	    myGLCD.fillRoundRect  (300, 100, 312, 112);   // Индикатор питания датчика газа
		myGLCD.setColor(255, 255, 255);
	}
	else                                              // Данные не получены
	{ 
   		myGLCD.setColor(0, 0, 0);
	    myGLCD.fillRoundRect  (300, 100, 312, 112);   // Индикатор питания датчика газа
		myGLCD.setColor(255, 255, 255);
	}
    exit_file_save();                              // Проверка состояния кнопок
	if(stop_info == true)
    {
	   	stop_info = false;
		return;
	}
	command = 17;                          // Состояние питания датчика Гейгера
	st_PowerGeiger = 0;
    myGLCD.printNumI(command, 210, 40);
    myGLCD.print("->", 240, 40);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 40);
    timestamp = millis();                  // Запомнили время отправки:
    waitanswer();                          // Запускаем профедуру ожидания ответа
    exit_file_save();                              // Проверка состояния кнопок
	if(stop_info == true)
    {
	   	stop_info = false;
		return;
	}
	if (st_PowerGeiger == 2 )                           // Питание включено
	{
		myGLCD.setColor(0, 255, 0);
    	myGLCD.fillRoundRect  (300, 60, 312, 72);     // Индикатор питания счетчика Гейгера
		myGLCD.setColor(255, 255, 255);
	}
	else if (st_PowerGeiger == 1 )                      // Питание отключено
	{
		myGLCD.setColor(255, 0, 0);
	    myGLCD.fillRoundRect  (300, 60, 312, 72);     // Индикатор питания счетчика Гейгера
		myGLCD.setColor(255, 255, 255);
	}
	else                                              // Данные не получены
	{ 
   		myGLCD.setColor(0, 0, 0);
	    myGLCD.fillRoundRect  (300, 60, 312, 72);     // Индикатор питания счетчика Гейгера
		myGLCD.setColor(255, 255, 255);
	}

    delay(10);
  }
}

void waitanswer()
{
  // Немного плохого кода:
  // Устанавливаем timeout в ИСТИНУ
  // Если ответ будет получен, установим переменную в ЛОЖЬ
  // Если ответа не будет - считаем ситуацию выходом по таймауту
  timeout = true;
  while (millis() - timestamp < TIMEOUT && timeout)  // Ждём ответ или таймута ожидания
  {
    if (!Mirf.isSending() && Mirf.dataReady())
    {
      myGLCD.setBackColor(0, 0, 0);
      myGLCD.print("      ", 100, 40);
      myGLCD.printNumI(millis() - timestamp, 100, 40);
      myGLCD.setBackColor(0, 0, 0);
      myGLCD.print("<-", 280, 40);
      delay(200);
      myGLCD.print("  ", 280, 40);
      timeout = false;
      Mirf.getData((byte *)&data);    // Принимаем пакет данные в виде массива байт в переменную data:
      switch (command)
      {
        case 1:
          myGLCD.print("\x81""a""\x9C"" V =        ", LEFT, 100); //Газ V =
          myGLCD.printNumI(data, 120, 100);
          break;
        case 2:
          if (data == 1)
          {
            geiger_ready = true;
          }
          else
          {
            geiger_ready = false;
          }
          break;
        case 3:
          Serial.println(data);
          myGLCD.print("cpm   =        ", LEFT, 60);      
          myGLCD.printNumI(data, 120, 60);
          break;
        case 4:
          data_f = data;
          data_f = data_f / 10000;
          Serial.println(data_f , 4);
          myGLCD.print("uSv/h =        ", LEFT, 80);
          myGLCD.printNumF(data_f, 4, 120, 80);
          break;
        case 5:
          myGLCD.print("Te""\xA1\xA3"".C=   ", LEFT, 120);                 //Темп.С =
          myGLCD.printNumF(data * 0.1, 1, 120, 120);
          break;
        case 6:
          myGLCD.print("P   mmHq", 190, 120);                              //Давл.Ра =
          myGLCD.printNumI(data, 204, 120);
          break;
        case 7:
          data_f = data;
          data_f = data_f / 1000000;
          myGLCD.setFont(SmallFont);
          myGLCD.print("LAT =           ", 5, 160);                         // 
          myGLCD.printNumF(data_f, 6, 50, 160);
          myGLCD.setFont(BigFont);
          break;
        case 8:
          data_f = data;
          data_f = data_f / 1000000;
          myGLCD.setFont(SmallFont);
          myGLCD.print("LON =           ", 140, 160);                       // 
          myGLCD.printNumF(data_f, 6, 190, 160);
          myGLCD.setFont(BigFont);
          break;
        case 9:
          myGLCD.setFont(SmallFont);
          myGLCD.print("B""\xAB""co""\xA4""a =     ", 5, 143);              //Высота =
		  data = 1000;
          myGLCD.printNumI(data, 75, 143);
		  myGLCD.setFont(BigFont);
          break;
        case 10:
          myGLCD.setFont(SmallFont);
          myGLCD.print("\x82\x9D""c""\xA4"". =     ", 140, 143);            //Дист. =
		  data = 1000;
          myGLCD.printNumI(data, 205, 143);
		  myGLCD.setFont(BigFont);
          break;
        case 11:
		  myGLCD.setFont(SmallFont);
          myGLCD.print("Sat =      ", 5, 175); //Дист. =
          if (data == 255)
          {
            myGLCD.printNumI(0, 50, 175);
          }
          else
          {
            myGLCD.printNumI(data, 50, 175);
          }
          myGLCD.setFont(BigFont);
          break;
        case 12:
          if (data == 1)
          {
            st_Power_gaz = 1;
          }
          else if (data == 2)
          {
            st_Power_gaz = 2;
          }
          else
          {
            st_Power_gaz = 0;
          }
          break;
        case 13:
          if (data == 1)
          {
            st_Power_gaz = 1;
          }
          else if (data == 2)
          {
            st_Power_gaz = 2;
          }
          else
          {
            st_Power_gaz = 0;
          }
          break;
        case 14:
          if (data == 1)
          {
            st_Power_gaz = 1;
          }
          else if (data == 2)
          {
            st_Power_gaz = 2;
          }
          else
          {
            st_Power_gaz = 0;
          }
          break;
        case 15:
          if (data == 1)
          {
            st_PowerGeiger = 1;
          }
          else if (data == 2)
          {
            st_PowerGeiger = 2;
          }
          else
          {
            st_PowerGeiger = 0;
          }
          break;
        case 16:
          if (data == 1)
          {
            st_PowerGeiger = 1;
          }
          else if (data == 2)
          {
            st_PowerGeiger = 2;
          }
          else
          {
            st_PowerGeiger = 0;
          }
          break;
        case 17:
          if (data == 1)
          {
            st_PowerGeiger = 1;
          }
          else if (data == 2)
          {
            st_PowerGeiger = 2;
          }
          else
          {
            st_PowerGeiger = 0;
          }
          break;
        case 18:
          break;
        case 19:
          DOM_LAT = data * 1000000;
          break;
        case 20:
          DOM_LON = data * 1000000;
          break;
      }
      data = 0;
    }
  }
  if (timeout)
  {
    // ответа не пришло
    myGLCD.setBackColor(0, 0, 0);
    for (byte i = 0; i < 2; i++)
    {
      myGLCD.print("X ", 280, 40);
      delay(500);
      myGLCD.print("  ", 280, 40);
    }
    Serial.println("Timeout");
  }
}

void exit_file_save()
{
  if (myTouch.dataAvailable())
  {
	myTouch.read();
	x = myTouch.getX();
	y = myTouch.getY();	
	if ((x >= 2) && (x <= 318))            //
		{
		if ((y >= 2) && (y <= 190))        // Выход
		{
			waitForIt(2, 2, 318, 190);
			stop_info = true;                // Остановить прием информации
		}
		if ((y >= 196) && (y <= 238))      // ОТКЛ
		{
			waitForIt(2, 196, 318, 238);
			save_file = !save_file;
			if(save_file == true)                                // Записать данные в файл
			{
			  if(start_save_file == false)                      // Записать имя файла
			   {
				   start_save_file = true;
				   stop_save_file  = true;
				   myGLCD.setBackColor(0, 0, 255);
				   myGLCD.print("                  ", CENTER, 5);
				   myGLCD.print("Save file", CENTER, 5);
				   myGLCD.setBackColor(0, 0, 0);

			   }
				myGLCD.setColor(255, 0, 0);
				myGLCD.fillRoundRect (3, 197, 317, 237);
				myGLCD.setColor(255, 255, 255);
				myGLCD.setBackColor(255, 0, 0);
				myGLCD.print(txt_file_stop, CENTER, 207);      // Записать в файл
				myGLCD.setBackColor(0, 0, 0);

			}
			else
			{
				myGLCD.setColor(0, 0, 255);
				myGLCD.fillRoundRect (3, 197, 317, 237);
				myGLCD.setColor(255, 255, 255);
				myGLCD.setBackColor(0, 0, 255);
				myGLCD.print(txt_file_save, CENTER, 207);      // Записать в файл
				myGLCD.setBackColor(0, 0, 0);
				if(	stop_save_file = true)                     // Закрыть файл  
				{
	               stop_save_file  = false;
				   start_save_file = false;
				   myGLCD.setBackColor(0, 0, 255);
				   myGLCD.print(txt_info_n_user, CENTER, 5);
				   myGLCD.setBackColor(0, 0, 0);
				}

			}
		}
	}
  }
}

void read_data_eeprom()// Чтение состояния счетчиков из памяти
{
  /*
  						   byte y[4];    //Чтение из памяти текущих данных счетчика
  						   y[3]= i2c_eeprom_read_byte( deviceaddress, 3+adr_count_electro_ok);
  						   y[2]= i2c_eeprom_read_byte( deviceaddress, 2+adr_count_electro_ok);
  						   y[1]= i2c_eeprom_read_byte( deviceaddress, 1+adr_count_electro_ok);
  						   y[0]= i2c_eeprom_read_byte( deviceaddress, 0+adr_count_electro_ok);
  						   count_electro_ok = (unsigned long&) y;  // Сложить восстановленные текущие данные в count_electro_ok

  						   y[4];                             //Чтение из памяти текущих данных счетчика
  						   y[3]= i2c_eeprom_read_byte( deviceaddress, 3+adr_count_electro_old);
  						   y[2]= i2c_eeprom_read_byte( deviceaddress, 2+adr_count_electro_old);
  						   y[1]= i2c_eeprom_read_byte( deviceaddress, 1+adr_count_electro_old);
  						   y[0]= i2c_eeprom_read_byte( deviceaddress, 0+adr_count_electro_old);
  						   count_electro_old = (unsigned long&) y;   // Сложить восстановленные текущие данные в count_electro_old

  						   y[4]; //Чтение из памяти текущих данных счетчика
  						   y[3]= i2c_eeprom_read_byte( deviceaddress, 3+adr_count_gaz_ok);
  						   y[2]= i2c_eeprom_read_byte( deviceaddress, 2+adr_count_gaz_ok);
  						   y[1]= i2c_eeprom_read_byte( deviceaddress, 1+adr_count_gaz_ok);
  						   y[0]= i2c_eeprom_read_byte( deviceaddress, 0+adr_count_gaz_ok);
  						   count_gaz_ok = (unsigned long&) y;  // Сложить восстановленные текущие данные в count_electro_ok

  						   y[4];                                //Чтение из памяти текущих данных счетчика
  						   y[3]= i2c_eeprom_read_byte( deviceaddress, 3+adr_count_gaz_old);
  						   y[2]= i2c_eeprom_read_byte( deviceaddress, 2+adr_count_gaz_old);
  						   y[1]= i2c_eeprom_read_byte( deviceaddress, 1+adr_count_gaz_old);
  						   y[0]= i2c_eeprom_read_byte( deviceaddress, 0+adr_count_gaz_old);
  						   count_gaz_old = (unsigned long&) y;       // Сложить восстановленные текущие данные в count_electro_old

  						   y[4]; //Чтение из памяти текущих данных счетчика
  						   y[3]= i2c_eeprom_read_byte( deviceaddress, 3+adr_count_colwater_ok);
  						   y[2]= i2c_eeprom_read_byte( deviceaddress, 2+adr_count_colwater_ok);
  						   y[1]= i2c_eeprom_read_byte( deviceaddress, 1+adr_count_colwater_ok);
  						   y[0]= i2c_eeprom_read_byte( deviceaddress, 0+adr_count_colwater_ok);
  						   count_colwater_ok = (unsigned long&) y;  // Сложить восстановленные текущие данные в count_colwater_ok

  						   y[4];                                //Чтение из памяти текущих данных счетчика
  						   y[3]= i2c_eeprom_read_byte( deviceaddress, 3+adr_count_colwater_old);
  						   y[2]= i2c_eeprom_read_byte( deviceaddress, 2+adr_count_colwater_old);
  						   y[1]= i2c_eeprom_read_byte( deviceaddress, 1+adr_count_colwater_old);
  						   y[0]= i2c_eeprom_read_byte( deviceaddress, 0+adr_count_colwater_old);
  						   count_colwater_old = (unsigned long&) y;       // Сложить восстановленные текущие данные в count_colwater_old

  						   y[4]; //Чтение из памяти текущих данных счетчика
  						   y[3]= i2c_eeprom_read_byte( deviceaddress, 3+adr_count_hotwater_ok);
  						   y[2]= i2c_eeprom_read_byte( deviceaddress, 2+adr_count_hotwater_ok);
  						   y[1]= i2c_eeprom_read_byte( deviceaddress, 1+adr_count_hotwater_ok);
  						   y[0]= i2c_eeprom_read_byte( deviceaddress, 0+adr_count_hotwater_ok);
  						   count_hotwater_ok = (unsigned long&) y;  // Сложить восстановленные текущие данные в count_hotwater_ok

  						   y[4];                                //Чтение из памяти текущих данных счетчика count_hotwater_old
  						   y[3]= i2c_eeprom_read_byte( deviceaddress, 3+adr_count_hotwater_old);
  						   y[2]= i2c_eeprom_read_byte( deviceaddress, 2+adr_count_hotwater_old);
  						   y[1]= i2c_eeprom_read_byte( deviceaddress, 1+adr_count_hotwater_old);
  						   y[0]= i2c_eeprom_read_byte( deviceaddress, 0+adr_count_hotwater_old);
  						   count_hotwater_old = (unsigned long&) y;       // Сложить восстановленные текущие данные в count_hotwater_old
  */
}
void format_memory()
{

  if (int x = i2c_eeprom_read_byte(deviceaddress, 510) != 5)

  { // write a 0 to all 500 bytes of the EEPROM
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("Format!", CENTER, 80); // "Format!"
    delay (500);
    //myGLCD.clrScr();
    for (int i = 0; i < 500; i++)
    {
      i2c_eeprom_write_byte(deviceaddress, i, 0);
    }
    i2c_eeprom_write_byte(deviceaddress, 510, 5);
    myGLCD.clrScr();
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("Format OK!", CENTER, 80); // "Format Ok!"
    delay (500);
    myGLCD.clrScr();
  }

}


int bcd2bin(int temp)//BCD  to decimal
{
  int a, b, c;
  a = temp;
  b = 0;
  if (a >= 16)
  {
    while (a >= 16)
    {
      a = a - 16;
      b = b + 10;
      c = a + b;
      temp = c;
    }
  }
  return temp;
}
/*
  int bin_to_bcd(int temp)//decimal to BCD
  {
	int a,b,c;
	a=temp;
	b=0;
	if(a>=10)
	{
		while(a>=10)
		{
			a=a-10;
			b=b+16;
			c=a+b;
			temp=c;
		}
	}
	return temp;
  }

  void clock_save()
  {
  Wire.beginTransmission(0x68);//DS1307 write the initial time
  Wire.write(0);
  Wire.requestFrom(0x68, 7);
  Wire.write(bin_to_bcd(sec));
  Wire.write(bin_to_bcd(min));
  Wire.write(bin_to_bcd(hour));
  Wire.write(bin_to_bcd(dow));
  Wire.write(bin_to_bcd(date));
  Wire.write(bin_to_bcd(mon));
  Wire.write(bin_to_bcd(year));
  Wire.endTransmission();

  }
*/
void clock_read()
{
  Wire.beginTransmission(0x68);//Send the address of DS1307
  Wire.write(0);//Sending address
  Wire.endTransmission();//The end of the IIC communication
  Wire.requestFrom(0x68, 7);//IIC communication is started, you can continue to access another address (address auto increase) and the number of visits
  sec = bcd2bin(Wire.read());//read time
  min = bcd2bin(Wire.read());
  hour = bcd2bin(Wire.read());
  dow = Wire.read();
  date = bcd2bin(Wire.read());
  mon = bcd2bin(Wire.read());
  year = bcd2bin(Wire.read()) + 2000;
  delay(10);
  //  Wire.endTransmission();//The end of the IIC communication
}
void clock_print_serial()
{
  /*
    Serial.print(date, DEC);
    Serial.print('/');
    Serial.print(mon, DEC);
    Serial.print('/');
    Serial.print(year, DEC);//Serial display time
    Serial.print(' ');
    Serial.print(hour, DEC);
    Serial.print(':');
    Serial.print(min, DEC);
    Serial.print(':');
    Serial.print(sec, DEC);
    Serial.println();
    Serial.print(" week: ");
    Serial.print(dow, DEC);
    Serial.println();
  */
}

void scanI2CBus(byte from_addr, byte to_addr, void(*callback)(byte address, byte result) )
{
  /*
    byte rc;
    byte data1 = 0; // not used, just an address to feed to twi_writeTo()
    for( byte addr = from_addr; addr <= to_addr; addr++ )
    {
    rc = twi_writeTo(addr, &data1,0, 0, 1);
    if(rc==0) callback( addr, rc );
    }
  */
}
void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data )
{
  int rdata = data;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(rdata);
  Wire.endTransmission();
  delay(20);
}
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length )
{
  /*
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddresspage >> 8)); // MSB
    Wire.write((int)(eeaddresspage & 0xFF)); // LSB
    byte c;
    for ( c = 0; c < length; c++)
    Wire.write(data[c]);
    Wire.endTransmission();
  */
}
byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress )
{

  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress, 1);
  if (Wire.available()) rdata = Wire.read();
  //delay(10);
  return rdata;

}
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length )
{

  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress, length);
  int c = 0;
  for ( c = 0; c < length; c++ )
    if (Wire.available()) buffer[c] = Wire.read();

}

void i2c_test()
{
  /*

    Serial.println("--------  EEPROM Test  ---------");
    char somedata[] = "this data from the eeprom i2c"; // data to write
    i2c_eeprom_write_page(0x50, 0, (byte *)somedata, sizeof(somedata)); // write to EEPROM
    delay(100); //add a small delay
    Serial.println("Written Done");
    delay(10);
    Serial.print("Read EERPOM:");
    byte b = i2c_eeprom_read_byte(0x50, 0); // access the first address from the memory
    char addr=0; //first address

    while (b!=0)
    {
    Serial.print((char)b); //print content to serial port
    if (b!=somedata[addr]){
     e1=0;
     break;
     }
    addr++; //increase address
    b = i2c_eeprom_read_byte(0x50, addr); //access an address from the memory
    }
    Serial.println();
  */
}
void i2c_test1()
{
  /*
    for( int n = 0; n<500; n++)
    {
  	i2c_eeprom_write_byte(0x50, n,0);

    }

    for(unsigned int x=0;x<255;x++)
    {
  	int  b = i2c_eeprom_read_byte(0x50, x); //access an address from the memory
  	delay(10);
  	Serial.print(x); //print content to serial port
  	Serial.print(" - "); //print content to serial port
  	Serial.println(b); //print content to serial port
    }
  */
}

void drawDisplay()
{
  // Clear screen
  myGLCD.clrScr();

  // Draw Clockface
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(0, 0, 0);
  for (int i = 0; i < 5; i++)
  {
    myGLCD.drawCircle(clockCenterX, clockCenterY, 119 - i);
  }
  for (int i = 0; i < 5; i++)
  {
    myGLCD.drawCircle(clockCenterX, clockCenterY, i);
  }

  myGLCD.setColor(192, 192, 255);
  myGLCD.print("3", clockCenterX + 92, clockCenterY - 8);
  myGLCD.print("6", clockCenterX - 8, clockCenterY + 95);
  myGLCD.print("9", clockCenterX - 109, clockCenterY - 8);
  myGLCD.print("12", clockCenterX - 16, clockCenterY - 109);
  for (int i = 0; i < 12; i++)
  {
    if ((i % 3) != 0)
      drawMark(i);
  }
  clock_read();
  drawMin(min);
  drawHour(hour, min);
  drawSec(sec);
  oldsec = sec;

  // Draw calendar
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect(240, 0, 319, 85);
  myGLCD.setColor(0, 0, 0);
  for (int i = 0; i < 7; i++)
  {
    myGLCD.drawLine(249 + (i * 10), 0, 248 + (i * 10), 3);
    myGLCD.drawLine(250 + (i * 10), 0, 249 + (i * 10), 3);
    myGLCD.drawLine(251 + (i * 10), 0, 250 + (i * 10), 3);
  }

  // Draw SET button
  myGLCD.setColor(64, 64, 128);
  myGLCD.fillRoundRect(260, 200, 319, 239);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect(260, 200, 319, 239);
  myGLCD.setBackColor(64, 64, 128);
  myGLCD.print("SET", 266, 212);
  myGLCD.setBackColor(0, 0, 0);

  myGLCD.setColor(64, 64, 128);
  myGLCD.fillRoundRect(260, 140, 319, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect(260, 140, 319, 180);
  myGLCD.setBackColor(64, 64, 128);
  myGLCD.print("RET", 266, 150);
  myGLCD.setBackColor(0, 0, 0);

}
void drawMark(int h)
{
  float x1, y1, x2, y2;

  h = h * 30;
  h = h + 270;

  x1 = 110 * cos(h * 0.0175);
  y1 = 110 * sin(h * 0.0175);
  x2 = 100 * cos(h * 0.0175);
  y2 = 100 * sin(h * 0.0175);

  myGLCD.drawLine(x1 + clockCenterX, y1 + clockCenterY, x2 + clockCenterX, y2 + clockCenterY);
}
void drawSec(int s)
{
  float x1, y1, x2, y2;
  int ps = s - 1;

  myGLCD.setColor(0, 0, 0);
  if (ps == -1)
    ps = 59;
  ps = ps * 6;
  ps = ps + 270;

  x1 = 95 * cos(ps * 0.0175);
  y1 = 95 * sin(ps * 0.0175);
  x2 = 80 * cos(ps * 0.0175);
  y2 = 80 * sin(ps * 0.0175);

  myGLCD.drawLine(x1 + clockCenterX, y1 + clockCenterY, x2 + clockCenterX, y2 + clockCenterY);

  myGLCD.setColor(255, 0, 0);
  s = s * 6;
  s = s + 270;

  x1 = 95 * cos(s * 0.0175);
  y1 = 95 * sin(s * 0.0175);
  x2 = 80 * cos(s * 0.0175);
  y2 = 80 * sin(s * 0.0175);

  myGLCD.drawLine(x1 + clockCenterX, y1 + clockCenterY, x2 + clockCenterX, y2 + clockCenterY);
}
void drawMin(int m)
{
  float x1, y1, x2, y2, x3, y3, x4, y4;
  int pm = m - 1;

  myGLCD.setColor(0, 0, 0);
  if (pm == -1)
    pm = 59;
  pm = pm * 6;
  pm = pm + 270;

  x1 = 80 * cos(pm * 0.0175);
  y1 = 80 * sin(pm * 0.0175);
  x2 = 5 * cos(pm * 0.0175);
  y2 = 5 * sin(pm * 0.0175);
  x3 = 30 * cos((pm + 4) * 0.0175);
  y3 = 30 * sin((pm + 4) * 0.0175);
  x4 = 30 * cos((pm - 4) * 0.0175);
  y4 = 30 * sin((pm - 4) * 0.0175);

  myGLCD.drawLine(x1 + clockCenterX, y1 + clockCenterY, x3 + clockCenterX, y3 + clockCenterY);
  myGLCD.drawLine(x3 + clockCenterX, y3 + clockCenterY, x2 + clockCenterX, y2 + clockCenterY);
  myGLCD.drawLine(x2 + clockCenterX, y2 + clockCenterY, x4 + clockCenterX, y4 + clockCenterY);
  myGLCD.drawLine(x4 + clockCenterX, y4 + clockCenterY, x1 + clockCenterX, y1 + clockCenterY);

  myGLCD.setColor(0, 255, 0);
  m = m * 6;
  m = m + 270;

  x1 = 80 * cos(m * 0.0175);
  y1 = 80 * sin(m * 0.0175);
  x2 = 5 * cos(m * 0.0175);
  y2 = 5 * sin(m * 0.0175);
  x3 = 30 * cos((m + 4) * 0.0175);
  y3 = 30 * sin((m + 4) * 0.0175);
  x4 = 30 * cos((m - 4) * 0.0175);
  y4 = 30 * sin((m - 4) * 0.0175);

  myGLCD.drawLine(x1 + clockCenterX, y1 + clockCenterY, x3 + clockCenterX, y3 + clockCenterY);
  myGLCD.drawLine(x3 + clockCenterX, y3 + clockCenterY, x2 + clockCenterX, y2 + clockCenterY);
  myGLCD.drawLine(x2 + clockCenterX, y2 + clockCenterY, x4 + clockCenterX, y4 + clockCenterY);
  myGLCD.drawLine(x4 + clockCenterX, y4 + clockCenterY, x1 + clockCenterX, y1 + clockCenterY);
}
void drawHour(int h, int m)
{
  float x1, y1, x2, y2, x3, y3, x4, y4;
  int ph = h;

  myGLCD.setColor(0, 0, 0);
  if (m == 0)
  {
    ph = ((ph - 1) * 30) + ((m + 59) / 2);
  }
  else
  {
    ph = (ph * 30) + ((m - 1) / 2);
  }
  ph = ph + 270;

  x1 = 60 * cos(ph * 0.0175);
  y1 = 60 * sin(ph * 0.0175);
  x2 = 5 * cos(ph * 0.0175);
  y2 = 5 * sin(ph * 0.0175);
  x3 = 20 * cos((ph + 5) * 0.0175);
  y3 = 20 * sin((ph + 5) * 0.0175);
  x4 = 20 * cos((ph - 5) * 0.0175);
  y4 = 20 * sin((ph - 5) * 0.0175);

  myGLCD.drawLine(x1 + clockCenterX, y1 + clockCenterY, x3 + clockCenterX, y3 + clockCenterY);
  myGLCD.drawLine(x3 + clockCenterX, y3 + clockCenterY, x2 + clockCenterX, y2 + clockCenterY);
  myGLCD.drawLine(x2 + clockCenterX, y2 + clockCenterY, x4 + clockCenterX, y4 + clockCenterY);
  myGLCD.drawLine(x4 + clockCenterX, y4 + clockCenterY, x1 + clockCenterX, y1 + clockCenterY);

  myGLCD.setColor(255, 255, 0);
  h = (h * 30) + (m / 2);
  h = h + 270;

  x1 = 60 * cos(h * 0.0175);
  y1 = 60 * sin(h * 0.0175);
  x2 = 5 * cos(h * 0.0175);
  y2 = 5 * sin(h * 0.0175);
  x3 = 20 * cos((h + 5) * 0.0175);
  y3 = 20 * sin((h + 5) * 0.0175);
  x4 = 20 * cos((h - 5) * 0.0175);
  y4 = 20 * sin((h - 5) * 0.0175);

  myGLCD.drawLine(x1 + clockCenterX, y1 + clockCenterY, x3 + clockCenterX, y3 + clockCenterY);
  myGLCD.drawLine(x3 + clockCenterX, y3 + clockCenterY, x2 + clockCenterX, y2 + clockCenterY);
  myGLCD.drawLine(x2 + clockCenterX, y2 + clockCenterY, x4 + clockCenterX, y4 + clockCenterY);
  myGLCD.drawLine(x4 + clockCenterX, y4 + clockCenterY, x1 + clockCenterX, y1 + clockCenterY);
}
void printDate()
{

  clock_read();
  myGLCD.setFont(BigFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);

  myGLCD.print(str[dow - 1], 256, 8);
  if (date < 10)
    myGLCD.printNumI(date, 272, 28);
  else
    myGLCD.printNumI(date, 264, 28);

  myGLCD.print(str_mon[mon - 1], 256, 48);
  myGLCD.printNumI(year, 248, 65);
}
void clearDate()
{
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRect(248, 8, 312, 81);
}
void AnalogClock()

{
  int x, y;

  drawDisplay();
  printDate();

  clock_read();

  while (true)
  {
    all_alarm();
    if (oldsec != sec)
    {
      if ((sec == 0) and (min == 0) and (hour == 0))
      {
        clearDate();
        printDate();
      }
      if (sec == 0)
      {
        drawMin(min);
        drawHour(hour, min);
      }
      drawSec(sec);
      oldsec = sec;
    }

    if (myTouch.dataAvailable())
    {
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();
      if (((y >= 200) && (y <= 239)) && ((x >= 260) && (x <= 319))) //установка часов
      {
        myGLCD.setColor (255, 0, 0);
        myGLCD.drawRoundRect(260, 200, 319, 239);
        setClock();
      }

      if (((y >= 140) && (y <= 180)) && ((x >= 260) && (x <= 319))) //Возврат
      {
        myGLCD.setColor (255, 0, 0);
        myGLCD.drawRoundRect(260, 140, 319, 180);
        myGLCD.clrScr();
        myGLCD.setFont(BigFont);
        break;
      }
    }

    delay(10);
    clock_read();
  }

}
//
//void info_nomer_user()
//{
//  myGLCD.clrScr();   // Очистить экран CENTER
//  myGLCD.setColor(0, 0, 255);
//  myGLCD.fillRoundRect (2, 2, 318, 25);
//  myGLCD.setColor(255, 255, 255);
//  myGLCD.drawRoundRect (2, 2, 318, 25);
//  myGLCD.setColor(255, 255, 255);
//  myGLCD.setBackColor(0, 0, 255);
//  myGLCD.print(txt_info_n_user, CENTER, 5);
//
//  stCurrentLen_user = i2c_eeprom_read_byte( deviceaddress, adr_n_user - 2); // Чтение номера пользователя
//  if (stCurrentLen_user > 20)
//  {
//    stCurrentLen_user = 0;
//  }
//  for (int z = 0; z < stCurrentLen_user + 1; z++)
//  {
//    n_user[z] = 0;
//  }
//  for (int z = 0; z < stCurrentLen_user; z++)
//  {
//    n_user[z] = i2c_eeprom_read_byte( deviceaddress, adr_n_user + z);
//  }
//
//  myGLCD.setBackColor(0, 0, 0);
//  myGLCD.print(txt_info_n_user1, CENTER, 30);
//  myGLCD.print(n_user, CENTER, 50);//
//
//  stCurrentLen_telef = i2c_eeprom_read_byte( deviceaddress, adr_n_telef - 2); // Чтение номера пользователя
//  if (stCurrentLen_telef > 11)
//  {
//    stCurrentLen_telef = 0;
//  }
//
//  for (int z = 0; z < stCurrentLen_telef + 1; z++)
//  {
//    n_telefon[z] = 0;
//  }
//
//  for (int z = 0; z < stCurrentLen_telef; z++)
//  {
//    n_telefon[z] = i2c_eeprom_read_byte( deviceaddress, adr_n_telef + z);
//  }
//
//  myGLCD.setBackColor(0, 0, 0);
////  myGLCD.print(txt_info_n_telef, CENTER, 70);
//  myGLCD.print(n_telefon, CENTER, 90);//
//  myGLCD.setColor(255, 255, 255);
//
//  myGLCD.setColor(0, 0, 255);
//  myGLCD.fillRoundRect (2, 216, 318, 238);
//  myGLCD.setColor(255, 255, 255);
//  myGLCD.drawRoundRect (2, 216, 318, 238);
//  myGLCD.setBackColor(0, 0, 255);
//  myGLCD.setColor(255, 255, 255);
//  myGLCD.print(txt_return, CENTER, 218);// Завершить просмотр
//
//  while (true)
//  {
//    delay(10);
//    myGLCD.setColor(255, 255, 255);//
//    myGLCD.setBackColor(0, 0, 0);
//    myGLCD.print("      ", CENTER, 120);//
//
//    if (myTouch.dataAvailable())
//    {
//      myTouch.read();
//      x = myTouch.getX();
//      y = myTouch.getY();
//
//      if ((y >= 2) && (y <= 240)) // Upper row
//      {
//        if ((x >= 2) && (x <= 319)) // Выход
//        {
//
//          waitForIt(10, 10, 60, 60);
//          return;
//        }
//      }
//    }
//  }
//}
void time_flag_start()
{
  timeF = millis();
  if (timeF > 60000) flag_time = 1;
}
void test_power()
{
  currentTime = millis();                           // считываем время, прошедшее с момента запуска программы
  if (currentTime >= (loopTime + time_power))
  { // сравниваем текущий таймер с переменной loopTime + 1 секунда
    loopTime = currentTime;                          // в loopTime записываем новое значение
    myGLCD.setFont(SmallFont);
    myGLCD.setColor(0, 255, 0);
    int power = analogRead(A3);
    // Serial.println(power);
    power60 = power * (5.0 / 1023.0 * 2);
    //  Serial.println(power60);
    if (power60 > 5.8) myGLCD.print("\xB0", 290, 20);
    else if (power60 <= 5.8 && power60 > 5.6) myGLCD.print("\xB1", 290, 20);
    else if (power60 <= 5.6 && power60 > 5.4) myGLCD.print("\xB2", 290, 20);
    else if (power60 <= 5.4 && power60 > 5.2) myGLCD.print("\xB3", 290, 20);

    else if (power60 <= 5.2)
    {
      myGLCD.setColor(255, 0, 0);
      myGLCD.print("\xB4", 290, 20);
    }
    myGLCD.printNumF(power60, 2, 280, 35);
    myGLCD.setColor(255, 255, 255);
    power = analogRead(A1);
    power50 = power * (5.0 / 1023.0 * 2);
    myGLCD.printNumF(power50, 2, 280, 45);
    power = analogRead(A2);
    power33 = power * (5.0 / 1023.0);
    myGLCD.printNumF(power33, 2, 280, 55);
    myGLCD.setFont(BigFont);
  }
}
void menu_gaz()
{
  st_Power_gaz = 0;
  myGLCD.clrScr();                                // Очистить экран CENTER
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (2, 2, 318, 25);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (2, 2, 318, 25);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.print(txt_gaz, CENTER, 5);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (2, 60, 318, 90);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (2, 60, 318, 90);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (20, 110, 150, 150);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (20, 110, 150, 150);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (170, 110, 300, 150);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (170, 110, 300, 150);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (95, 170, 225, 210);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (95, 170, 225, 210);

  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.print(txt_On, 58, 122);
  myGLCD.print(txt_Off, 200, 122);
  myGLCD.print(txt_Exit, 120, 180);

  info_power_gaz();

  while (true)
  {
    if (myTouch.dataAvailable())
    {
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();

      if ((y >= 110) && (y <= 150))       //
      {
        if ((x >= 20) && (x <= 150))    // ВКЛ
        {
          waitForIt(20, 110, 150, 150);
          radio_send(12);
          delay(500);
          info_power_gaz();
        }
        if ((x >= 170) && (x <= 300))  // ОТКЛ
        {
          waitForIt(170, 110, 300, 150);
          radio_send(13);
          delay(500);
          info_power_gaz();
        }
      }
      if ((y >= 170) && (y <= 210))       //
      {
        if ((x >= 95) && (x <= 225))  // ОТКЛ
        {
          waitForIt(95, 170, 225, 210);
          break;
          //  return;
        }
      }
    }
  }
}


void info_power_gaz()
{
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 0, 0);
  radio_send(14);                                       // Запросить состояние питания датчика газа
  delay(500);
  if (st_Power_gaz == 2 )                           // Питание включено
  {
    myGLCD.setBackColor(0, 0, 255);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print(txt_gazOn, CENTER, 67);          // Завершить просмотр
  }
  else if (st_Power_gaz == 1 )                       // Питание отключено
  {
    myGLCD.setBackColor(0, 0, 255);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print(txt_gazOff, CENTER, 67);          // Завершить просмотр
  }
  else
  { // Данные не получены
    myGLCD.setBackColor(0, 0, 255);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print(txt_gazNo, CENTER, 67);          // Завершить просмотр
  }
}
void menu_Geiger()
{
  st_PowerGeiger = 0;
  myGLCD.clrScr();                                // Очистить экран CENTER
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (2, 2, 318, 25);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (2, 2, 318, 25);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.print(txt_geiger, CENTER, 5);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (2, 60, 318, 90);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (2, 60, 318, 90);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (20, 110, 150, 150);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (20, 110, 150, 150);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (170, 110, 300, 150);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (170, 110, 300, 150);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (95, 170, 225, 210);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (95, 170, 225, 210);

  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.print(txt_On, 58, 122);
  myGLCD.print(txt_Off, 200, 122);
  myGLCD.print(txt_Exit, 120, 180);

  info_power_geiger();

  while (true)
  {
    if (myTouch.dataAvailable())
    {
      myTouch.read();
      x = myTouch.getX();
      y = myTouch.getY();

      if ((y >= 110) && (y <= 150))         //
      {
        if ((x >= 20) && (x <= 150))        // ВКЛ
        {
          waitForIt(20, 110, 150, 150);
          radio_send(15);
          delay(500);
          info_power_geiger();
        }
        if ((x >= 170) && (x <= 300))       // ОТКЛ
        {
          waitForIt(170, 110, 300, 150);
          radio_send(16);
          delay(500);
          info_power_geiger();
        }
      }
      if ((y >= 170) && (y <= 210))        //
      {
        if ((x >= 95) && (x <= 225))       // ОТКЛ
        {
          waitForIt(95, 170, 225, 210);
          break;
        }
      }
    }
  }
}
void info_power_geiger()
{
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 0, 0);
  radio_send(17);                                       // Запросить состояние питания датчика газа
  delay(500);
  if (st_PowerGeiger == 2 )                           // Питание включено
  {
    myGLCD.setBackColor(0, 0, 255);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print(txt_geigerOn, CENTER, 67);          // Завершить просмотр
  }
  else if (st_PowerGeiger == 1 )                       // Питание отключено
  {
    myGLCD.setBackColor(0, 0, 255);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print(txt_geigerOff, CENTER, 67);          // Завершить просмотр
  }
  else
  { // Данные не получены
    myGLCD.setBackColor(0, 0, 255);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print(txt_gazNo, CENTER, 67);          // Завершить просмотр
  }
}

void radio_send(int command_rf)
{
  command = command_rf;
  Mirf.setTADDR((byte *)&"fly10");
  Mirf.send((byte *)&command);    // команда отправлена
  delay(100);
  timestamp = millis();           // Запомнили время отправки:
  waitanswer();                   // Запускаем профедуру ожидания ответа
  if (myTouch.dataAvailable()) return;
}

void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial1.end();
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);
  myTouch.InitTouch();
  // myTouch.setPrecision(PREC_MEDIUM);
  myTouch.setPrecision(PREC_HI);
  myButtons.setTextFont(BigFont);
  myButtons.setSymbolFont(Dingbats1_XL);
  myGLCD.fillScr(255, 255, 255);
  myGLCD.drawBitmap(60, 20, 100, 100, rvsn2, 2);
  Wire.begin();
  if (!RTC.begin())
  {
    Serial.println("RTC failed");
    while (1);
  };
  // set date time callback function
  SdFile::dateTimeCallback(dateTime);
  // Запускает таймер и получает загружаемое значение таймера.
  // timerLoadValue=SetupTimer2(44100);
  // timerLoadValue=SetupTimer2(10100);
  flag_time = 0;
  format_memory();
  myGLCD.setBackColor(0, 0, 255);
  pinMode(53, OUTPUT);     // SD Card change this to 53 on a mega
  pinMode(48, OUTPUT);
  pinMode(49, OUTPUT);
  digitalWrite(48, HIGH);
  digitalWrite(49, HIGH);

  InitializingSD();

  if (!SD.begin(53))
  {
    Serial.println("initialization failed ReadWrite!");
  }
  Serial.println("initialization done.");
  // Настройка радиоканала
  Mirf.cePin = 8;
  Mirf.csnPin = 9;
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();

  Mirf.setRADDR((byte*)ADDR);
  Mirf.payload = sizeof(unsigned long);
  Mirf.config();
  //radio_send(12);
  //delay(1500);
  //Serial.println(st_Power_gaz);
  //radio_send(14);
  //delay(1500);
  //Serial.println(st_Power_gaz);
  //radio_send(13);
  //delay(1500);
  //Serial.println(st_Power_gaz);
  //radio_send(14);
  //delay(1500);
  //Serial.println(st_Power_gaz);
  //	  ReadWriteSD();
  myGLCD.clrScr();

}

void loop()
{
  draw_Glav_Menu();
  swichMenu();
}

