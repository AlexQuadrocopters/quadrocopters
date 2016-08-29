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
#include <OneWire.h>
#include <RTClib.h>
#include "I2Cdev.h"
#include <SdFat.h>
#include <SdFatUtil.h>
#include <Mirf.h>
#include <MirfHardwareSpiDriver.h>
#include <nRF24L01.h>

extern "C" {
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

//------------------------------------------------------------------------------------------------------

int deviceaddress =    0x50;
unsigned int eeaddress = 0;

int sys_N = 1;                         //

uint8_t sec   = 0;         //Initialization time
uint8_t min   = 0;
uint8_t hour  = 0;
uint8_t dow   = 1;
uint8_t day   = 1;
uint8_t month = 1;
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
//++++++++++++++++++++++ Работа с файлами +++++++++++++++++++++++++++++++++++++++


//SdVolume volume;
//SdFile root;
#define chipSelect SS                          // Настройка выбора SD
SdFat sd;
File myFile;
SdFile file;

Sd2Card card;
uint32_t cardSizeBlocks;
uint16_t cardCapacityMB;

// cache for SD block
cache_t cache;
// serial output steam
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// созданы переменные, использующие функции библиотеки SD utility library functions: +++++++++++++++
// Change spiSpeed to SPI_FULL_SPEED for better performance
// Use SPI_QUARTER_SPEED for even slower SPI bus speed
const uint8_t spiSpeed = SPI_HALF_SPEED;


//++++++++++++++++++++ Назначение имени файла ++++++++++++++++++++++++++++++++++++++++++++
//const uint32_t FILE_BLOCK_COUNT = 256000;
// log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "160101"
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char fileName[13]            = FILE_BASE_NAME "00.TXT";

//*********************Работа с именем файла ******************************

//byte file_name_count = 0;
char str_day_file[3];
char str_day_file0[3];
char str_day_file10[3];
char str_mon_file[3];
char str_mon_file0[3];
char str_mon_file10[3];
char str_year_file[3];

char str0[10];
char str1[10];
char str2[10];
char list_files_tab[100][13];        // Структура = list_files_tab[номер файла в списке][имя файла]
uint32_t size_files_tab[100] ;
int set_files = 0;
uint32_t const ERASE_SIZE = 262144L;
#define sdErrorMsg(msg) sdErrorMsg_P(PSTR(msg));
uint32_t cardSize;
uint32_t eraseSize;
//++++++++++++++++++ SD Format ++++++++++++++++++++++++++++++++++
// MBR information
uint8_t partType;
uint32_t relSector;
uint32_t partSize;

// Fake disk geometry
uint8_t numberOfHeads;
uint8_t sectorsPerTrack;

// FAT parameters
uint16_t reservedSectors;
uint8_t sectorsPerCluster;
uint32_t fatStart;
uint32_t fatSize;
uint32_t dataStart;

// constants for file system structure
uint16_t const BU16 = 128;
uint16_t const BU32 = 8192;

//  strings needed in file system structures
char noName[] = "NO NAME    ";
char fat16str[] = "FAT16   ";
char fat32str[] = "FAT32   ";



int  stCurrentLen_pass = 0;              // Длина вводимой строки
char pass_user[20];                      // Строка с паролем пользователя

char pass_super_admin[8] = {'1', '4', '0', '7', '5', '4', '\0'}; // Пароль супер Админ.
char pass_admin[20] ;                     // Строка с паролем Админ
int pass = 0;                             // Признак правильности введенного общего пароля
int pass1 = 0;                            // Признак правильности введенного пароля пользователя
int pass2 = 0;                            // Признак правильности введенного пароля Админ
int pass3 = 0;                            // Признак правильности введенного пароля Супер Админ
int eeprom_clear = 0;
int adr_variant_sys          = 241;       //

//byte hi;                                  // Старший байт для преобразования числа
//byte low;                                 // Младший байт для преобразования числа

char n_user[20];                          // Переменная хранения № пользователя
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
int stCurrentLen_admin = 0;        // Переменная  хранения длины введенной строки пароля администратора
char stLast[20] = "";              // Данные в введенной строке строке.
int ret = 0;                       // Признак прерывания операции
int lenStr = 0;                    // Длина строки

int st_Power_gaz       = 0;        // Состояние источника питания датчика газа
int st_PowerGeiger     = 0;        // Состояние источника питания датчика Гейгера
bool stop_info         = false;    // Остановить прием информации
bool save_file         = false;    // Записать данные в файл
bool start_save_file   = false;    // Открыть файл для записи    
bool stop_save_file    = false;    // Закрыть файл для записи  

// ++++++++++++++++++  Переменные измерений +++++++++++++++++++++++++++++++++
unsigned long count_strok = 0;               // Счетчик строк в файле
int cpm                     = 0;               // Счетчик Гейгера               
float uSv_h                 = 0;               // Счетчик Гейгера 
int temp_C                  = 0;               // Температура С 
int gaz_measure             = 0;               // Величина измеренной загазованности
int gaz_porog               = 0;               // Уровень порога газа
int P_mmHq                  = 0;               // Давление
int distance                = 0;               // Дистанция до объекта
int altitudeP               = 0;               // Высота по давлению
int f_altitude              = 0;               // Высота по GPS
int altitudeDom             = 0;               // Высота местности
int f_course                = 0;               // Направление на объект
int speed_kmph              = 0;               // Скорость движения
double gps_location_lat     = 0.0;             // Координата фактическая
double gps_location_lng     = 0.0;             // Координата фактическая
double DOM_LAT              = 55.954994;       // Координата домашняя
double DOM_LON              = 37.231121;       // Координата домашняя
float data_f                = 0;
int gound_m                 = 218;             // Высота местности над уровнем моря
int satellites              = 0;               // Количество спутников
double distanceToDOM        = 0;               // Расстояние до объекта
double courseToDOM          = 0;               // Направление на объект
int gps_date_value          = 0;
int gps_date_year           = 0;
int gps_date_month          = 0;
int gps_date_day            = 0;
int gps_time_value          = 0;
int gps_time_hour           = 0;
int gps_time_minute         = 0;
int gps_time_second         = 0;
int gps_time_centisecond    = 0;
int gps_speed_value         = 0;
int gps_speed_knots         = 0;
int gps_speed_mph           = 0;
int gps_speed_mps           = 0;
int gps_speed_kmph          = 0;
int gps_course_value        = 0;
int gps_course_deg          = 0;
int gps_altitude_value      = 0;
int gps_altitude_meters     = 0;
int gps_altitude_miles      = 0;
int gps_altitude_kilometers = 0;
int gps_altitude_feet       = 0;
int gps_satellites_value    = 0;
int gps_hdop_value          = 0;


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
char  txt_menu3_1[]            = "Pa""\x96""o""\xA4""a c SD";                       // Работа с SD
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
char  txt_info11[]             = "ESC->PUSH Display"; 
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
char  txt_file_save[]          = "\x85""a""\xA3\x9D""ca""\xA4\xAC"" ""\x97"" ""\xA5""a""\x9E\xA0";   // Записать в файл
char  txt_file_stop[]          = "Oc""\xA4""a""\xA2""o""\x97\x9D\xA4\xAC"" ""\x9C""a""\xA3\x9D""c""\xAC"; // Остановить запись
char  txt_info_n_user[]        = "\x89""p""\x9D""e""\xA1"" ""\x9D\xA2\xA5""op""\xA1""a""\xA6\x9D\x9D"; // Прием информации
char  txt_SD_menu1[]           = "\x89poc\xA1o\xA4p \xA5""a\x9E\xA0""a";                                                                 //
char  txt_SD_menu2[]           = "\x86\xA2\xA5o SD";                                                                   //
char  txt_SD_menu3[]           = "\x8Bop\xA1""a\xA4 SD";                                                                 //
char  txt_SD_menu4[]           = "B\x91XO\x82";           



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
        menu_Geiger(); // если верно - выполнить пункт меню ПИТАНИЕ датчика Гейгера
        myGLCD.clrScr();
        myButtons.drawButtons();
        print_up();
      }
      if (pressed_button == but4 && m2 == 1)
      {
        menu_gaz(); // если верно - выполнить пункт меню ПИТАНИЕ датчика газа
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
		myGLCD.clrScr();
		Draw_menu_SD();
		menu_SD();
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

		myGLCD.clrScr();

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

void waitForIt(int x1, int y1, int x2, int y2)
{
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
  myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
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
  //myGLCD.setFont(BigFont);
  //myGLCD.setBackColor(0, 0, 0);
  //myGLCD.clrScr();
  //// txt_set_pass_user  Вывод строки "Введите пароль пользователя!"
  //myGLCD.setColor(255, 255, 255);

  //myGLCD.print(txt_system_clear1, CENTER, 70);       // Внимание !
  //myGLCD.print(txt_system_clear2, CENTER, 100);      // Вся информация будет
  //myGLCD.setColor(255, 0, 0);
  //myGLCD.print(txt_system_clear3, CENTER, 130);       // УДАЛЕНА

  //delay(500);
  //myGLCD.print("                        ", CENTER, 70);
  //myGLCD.print("                        ", CENTER, 100);
  //myGLCD.print("                        ", CENTER, 130);
  //delay(500);
  //myGLCD.setColor(255, 255, 255);
  //myGLCD.print(txt_system_clear1, CENTER, 70);           // Внимание !
  //myGLCD.print(txt_system_clear2, CENTER, 100);          // Вся информация будет
  //myGLCD.setColor(255, 0, 0);
  //myGLCD.print(txt_system_clear3, CENTER, 130);          // УДАЛЕНА

  //delay(3000);
  //myGLCD.clrScr();

  //myGLCD.setBackColor(0, 0, 255);
  //myGLCD.setColor(0, 0, 255);
  //myGLCD.fillRoundRect (10, 130, 150, 180);
  //myGLCD.setColor(255, 255, 255);
  //myGLCD.drawRoundRect (10, 130, 150, 180);
  //myGLCD.print(txt_botton_otmena, 30, 147);//"Отмена"

  //myGLCD.setColor(0, 0, 255);
  //myGLCD.fillRoundRect (160, 130, 300, 180);
  //myGLCD.setColor(255, 255, 255);
  //myGLCD.drawRoundRect (160, 130, 300, 180);
  //myGLCD.print(txt_botton_clear, 190, 147);// "Сброс"

  //while (true)
  //{
  //  if (myTouch.dataAvailable())
  //  {
  //    myTouch.read();
  //    x = myTouch.getX();
  //    y = myTouch.getY();


  //    if ((y >= 130) && (y <= 180)) // Upper row
  //    {
  //      if ((x >= 10) && (x <= 150)) // Button: "Отмена"
  //      {
  //        break;
  //      }
  //      if ((x >= 160) && (x <= 300)) // Button: "Сброс"
  //      {
  //        myGLCD.setColor(0, 0, 255);
  //        myGLCD.fillRoundRect (0, 70, 319, 105);
  //        myGLCD.setColor(255, 255, 255);
  //        myGLCD.drawRoundRect (0, 70, 319, 105);
  //        if (eeprom_clear == 1)

  //        { // write a 0 to all 512 bytes of the EEPROM
  //          for (int i = 0; i < 319; i++)
  //          {
  //            i2c_eeprom_write_byte(deviceaddress, i, 0);
  //            myGLCD.print(">", i, 80); // "Сброс Ok!"
  //            //delay (10);
  //          }
  //        }
  //        else
  //        {
  //          for (int i = 0; i < 319; i++)
  //          {
  //            myGLCD.print("<", i, 80); // "Сброс Ok!"
  //            //delay (10);
  //          }

  //        }
  //        Serial.println("Removing elektro.txt...");
  //        SD.remove("elektro.txt");

  //        if (SD.exists("elektro.txt"))
  //        {
  //          Serial.println("elektro.txt exists.");
  //        }
  //        else
  //        {
  //          Serial.println("elektro.txt doesn't exist.");
  //        }
  //        //	 n_str_electro = 0; // Устанавливаем № строки 1
  //        // разбираем
  //        //hi=highByte(n_str_electro);
  //        //low=lowByte(n_str_electro);
  //        //// тут мы эти hi,low можем сохранить EEPROM
  //        //i2c_eeprom_write_byte(deviceaddress,adr_n_str_electro, hi);
  //        //i2c_eeprom_write_byte(deviceaddress,adr_n_str_electro+1, low);

  //        // gaz.txt

  //        Serial.println("Removing gaz.txt...");
  //        SD.remove("gaz.txt");

  //        if (SD.exists("gaz.txt"))
  //        {
  //          Serial.println("gaz.txt exists.");
  //        }
  //        else
  //        {
  //          Serial.println("gaz.txt doesn't exist.");
  //        }

  //        //   n_str_gaz = 0; // Устанавливаем № строки 1
  //        //  // разбираем
  //        //hi=highByte(n_str_gaz);
  //        //low=lowByte(n_str_gaz);
  //        //// тут мы эти hi,low можем сохранить EEPROM
  //        //i2c_eeprom_write_byte(deviceaddress,adr_n_str_gaz, hi);
  //        //i2c_eeprom_write_byte(deviceaddress,adr_n_str_gaz+1, low);

  //        // colwater.txt
  //        Serial.println("Removing colwater.txt...");
  //        SD.remove("colwater.txt");

  //        if (SD.exists("colwater.txt"))
  //        {
  //          Serial.println("colwater.txt exists.");
  //        }
  //        else
  //        {
  //          Serial.println("colwater.txt doesn't exist.");
  //        }

  //        //   n_str_colwater = 0; // Устанавливаем № строки 1
  //        //  // разбираем
  //        //hi=highByte(n_str_colwater);
  //        //low=lowByte(n_str_colwater);
  //        //// тут мы эти hi,low можем сохранить EEPROM
  //        //i2c_eeprom_write_byte(deviceaddress,adr_n_str_colwater, hi);
  //        //i2c_eeprom_write_byte(deviceaddress,adr_n_str_colwater+1, low);


  //        // hotwater.txt
  //        Serial.println("Removing hotwater.txt...");
  //        SD.remove("hotwater.txt");

  //        if (SD.exists("hotwater.txt"))
  //        {
  //          Serial.println("hotwater.txt exists.");
  //        }
  //        else
  //        {
  //          Serial.println("hotwater.txt doesn't exist.");
  //        }

  //        //   n_str_hotwater = 0; // Устанавливаем № строки 1
  //        //  // разбираем
  //        //hi=highByte(n_str_hotwater);
  //        //low=lowByte(n_str_hotwater);
  //        //// тут мы эти hi,low можем сохранить EEPROM
  //        //i2c_eeprom_write_byte(deviceaddress,adr_n_str_hotwater, hi);
  //        //i2c_eeprom_write_byte(deviceaddress,adr_n_str_hotwater+1, low);
  //        myGLCD.print("C\x96poc OK!", 100, 80);// "Сброс Ok!"
  //        delay (1000);
  //        break;

  //      }

  //    }
  //  }
  //}
  //eeprom_clear = 0;
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
  myGLCD.drawRoundRect (299, 70, 313, 84);                 // Индикатор питания счетчика Гейгера
  myGLCD.drawRoundRect (299, 89, 313, 103);                // Индикатор питания датчика газа
  myGLCD.print(txt_file_save, CENTER, 207);                // Записать в файл
  stop_info = false;

  while (true)
  {
    myGLCD.setBackColor(0, 0, 0);
    timeout = false;
    Mirf.setTADDR((byte *)&"fly10");            // Устанавливаем адрес передачи
    myGLCD.print("fly10", LEFT, 35);

	send_command(1);                            // Флаг готовности Счетчика Гейгера
    waitanswer();                               // Запускаем профедуру ожидания ответа
    exit_file_save();                           // Проверка состояния кнопок
	if(stop_info == true)
	{
		stop_info = false;
		return;
	}

	if (geiger_ready == true)                   // Если данные со счетчика Гейгера готовы - запрашиваем их
    {
		send_command(2);                        // Показания Счетчика Гейгера отправлены
		waitanswer(); 
		Serial.print("cpm = ");
		exit_file_save();                       // Проверка состояния кнопок
		if(stop_info == true)
		{
			stop_info = false;
			return;
		}
		send_command(3);                        // Показания Счетчика Гейгера отправлены
		waitanswer();                           // Запускаем процедуру ожидания ответа
		exit_file_save();                       // Проверка состояния кнопок
	  if(stop_info == true)
       {
	   	 stop_info = false;
		 return;
	   }
      Serial.print("uSv/h = ");
	}

	  send_command(4);                          // Состоянеи ключа включения питания счетчика Гейгера
		waitanswer();
			if (st_PowerGeiger == 1)
			{
				myGLCD.setColor(255, 0, 0);
			    myGLCD.fillRoundRect  (300, 71, 312, 83);     // Индикатор питания счетчика Гейгера
			    myGLCD.setColor(255, 255, 255);
			}
			else if (st_PowerGeiger == 2)
			{
				myGLCD.setColor(0, 255, 0);
				myGLCD.fillRoundRect  (300, 71, 312, 83);     // Индикатор питания счетчика Гейгера
				myGLCD.setColor(255, 255, 255);
			}
			else
			{
				myGLCD.setColor(0, 0, 0);
				myGLCD.fillRoundRect  (300, 71, 312, 83);     // Индикатор питания счетчика Гейгера
				myGLCD.setColor(255, 255, 255);
			}
		exit_file_save();                       // Проверка состояния кнопок
		if(stop_info == true)
		{
		stop_info = false;
		return;
		}
	
	 send_command(5);                           // Анализатор Газа
		waitanswer(); 
		exit_file_save();                       // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	 send_command(6);                           // Состоянеи ключа включения питания датчика газа
		waitanswer();                           // Запускаем профедуру ожидания ответа
		if (st_Power_gaz == 1)
		{
			myGLCD.setColor(255, 0, 0);
			myGLCD.fillRoundRect  (300, 90, 312, 102);   // Индикатор питания датчика газа
			myGLCD.setColor(255, 255, 255);
		}
		else if (st_Power_gaz == 2)
		{
			myGLCD.setColor(0, 255, 0);
			myGLCD.fillRoundRect  (300, 90, 312, 102);   // Индикатор питания датчика газа
			myGLCD.setColor(255, 255, 255);
		}
		else
		{
			myGLCD.setColor(0, 0, 0);
			myGLCD.fillRoundRect  (300, 90, 312, 102);   // Индикатор питания датчика газа
			myGLCD.setColor(255, 255, 255);
		}
		exit_file_save();                       // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	send_command(7);                            // Паказания температуры от BMP085
		waitanswer();                           // Запускаем профедуру ожидания ответа                                     
		exit_file_save();                       // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	send_command(8);                            // Показания давления от BMP085
		waitanswer();                           // Запускаем профедуру ожидания ответа 
		exit_file_save();                       // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	send_command(9);                            // Показания Высота от датчика давления BMP085
		waitanswer();                           // Запускаем профедуру ожидания ответа
		exit_file_save();                       // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	send_command(10);                          // Высота в метрах со спутника
	    waitanswer();                          // Запускаем профедуру ожидания ответа
	    exit_file_save();                      // Проверка состояния кнопок
		if(stop_info == true)
	    {
		   	stop_info = false;
			return;
		}
	send_command(11);                          // Передать  координаты LAT
		waitanswer();                          // Запускаем профедуру ожидания ответа
		exit_file_save();                      // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	send_command(12);                          // Передать  координаты LON
		waitanswer();                          // Запускаем профедуру ожидания ответа
		exit_file_save();                      // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}

	send_command(13);                          // Передать местные координаты DOM_LAT
		waitanswer();                          // Запускаем профедуру ожидания ответа
		exit_file_save();                      // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	send_command(14);                          // Передать местные координаты DOM_LON
		waitanswer();                          // Запускаем профедуру ожидания ответа
		exit_file_save();                      // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	send_command(15);                          // Количество спутников 
		waitanswer();                          // Запускаем профедуру ожидания ответа
		exit_file_save();                      // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	send_command(16);                          // Расстояние до объекта
		waitanswer();                          // Запускаем профедуру ожидания ответа
		exit_file_save();                      // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	send_command(17);                          // Направление на объект
		waitanswer();                          // Запускаем профедуру ожидания ответа
		exit_file_save();                      // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
		}
	send_command(18);                          // Скорость объекта метров в час
		waitanswer();                          // Запускаем профедуру ожидания ответа
		exit_file_save();                      // Проверка состояния кнопок
		if(stop_info == true)
		{
	   		stop_info = false;
			return;
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
      myGLCD.print("      ", 90, 35);
      myGLCD.printNumI(millis() - timestamp, 90, 35);
      myGLCD.setBackColor(0, 0, 0);
      myGLCD.print("<-", 280, 35);
      delay(200);
      myGLCD.print("  ", 280, 35);
      timeout = false;
      Mirf.getData((byte *)&data);    // Принимаем пакет данные в виде массива байт в переменную data:
      switch (command)
      {

	  	case 1:             // Флаг готовности Счетчика Гейгера
			if (data == 1)
			{
				geiger_ready = true;
			}
			else
			{
				geiger_ready = false;
			} 
			break;
		case 2:
			cpm = data;
			Serial.println(cpm);
			myGLCD.print("cpm   =        ", LEFT, 53);      
			myGLCD.printNumI(cpm, 120, 53);
			break;
		case 3:
			uSv_h = data;
			uSv_h = uSv_h / 10000;
			Serial.println(uSv_h , 4);
			myGLCD.print("uSv/h =        ", LEFT, 70);
			myGLCD.printNumF(uSv_h, 4, 120, 70);
			break;
		case 4:
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
		case 5:                                        // Анализатор Газа
            myGLCD.print("\x81""a""\x9C"" V =        ", LEFT, 87); //Газ V =
            myGLCD.printNumI(data, 120, 87);
			break;
		case 6:                                       // Состоянеи ключа включения питания датчика газа
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
		case 7:  // Паказания температуры от BMP085
			temp_C = data;                                                   // от датчика давления BMP085
			myGLCD.setFont(SmallFont);
			myGLCD.print("Te""\xA1\xA3"".C =    ", 5, 108);                 // Темп.С =
			myGLCD.printNumF(temp_C * 0.1, 1, 75, 108);
			myGLCD.setFont(BigFont);
			break;
		case 8:   // Показания давления от BMP085
			P_mmHq = data;                                                   // от датчика давления BMP085
			myGLCD.setFont(SmallFont);
			myGLCD.print("P   mmHq", 120, 108);                              // Давл.Ра = 
			myGLCD.printNumI(data, 135, 108);          // Показания давления от BMP085
			myGLCD.setFont(BigFont);
			break;  
		case 9:// Показания Высота от датчика давления BMP085
			altitudeP = data;
			myGLCD.setFont(SmallFont);
			myGLCD.print("B""\xAB""co""\xA4""a =      ", 200, 108);              // Высота =
			myGLCD.printNumI(altitudeP, 270, 108);                             // Высота от датчика давления BMP085
			myGLCD.setFont(BigFont);
			break;
		case 10: // Высота в метрах со спутника
		 //  data = gps_altitude_meters;                // Высота в метрах со спутника
			break;
		case 11:                                        // Передать  координаты LAT
			gps_location_lat = data;
			gps_location_lat = gps_location_lat / 1000000;
			myGLCD.setFont(SmallFont);
			myGLCD.print("LAT =           ", 5, 130);                         // 
			myGLCD.printNumF(gps_location_lat, 6, 50, 130);
			myGLCD.setFont(BigFont);
			break;
		case 12:                                           // Передать  координаты LON
			gps_location_lng = data;
			gps_location_lng = gps_location_lng / 1000000;
			myGLCD.setFont(SmallFont);
			myGLCD.print("LON =           ", 140, 130);                       // 
			myGLCD.printNumF(gps_location_lng, 6, 190, 130);
			myGLCD.setFont(BigFont);
			break;
		case 13:                                             // Передать местные координаты DOM_LAT
			DOM_LAT = data;
			DOM_LAT = DOM_LAT / 1000000;
			myGLCD.setFont(SmallFont);
			myGLCD.print("LAT =           ", 5, 147);                         // 
			myGLCD.printNumF(DOM_LAT, 6, 50, 147);
			myGLCD.setFont(BigFont);
			break;
		case 14:                                             // Передать местные координаты DOM_LON
			DOM_LON = data;
			DOM_LON = DOM_LON / 1000000;
			myGLCD.setFont(SmallFont);
			myGLCD.print("LON =           ", 140, 147);                       // 
			myGLCD.printNumF(DOM_LON, 6, 190, 147);
			myGLCD.setFont(BigFont);
			break;
		case 15:                                              // Количество спутников
			gps_satellites_value = data;
			myGLCD.setFont(SmallFont);
			myGLCD.print("Sat =   ", 5, 164);                              // Количество спутников
			if (gps_satellites_value == 255)
			{
			gps_satellites_value = 0;
			}
			myGLCD.printNumI(gps_satellites_value, 50, 164);
			myGLCD.setFont(BigFont);
			break;
		case 16:                                              // Расстояние до объекта
			distanceToDOM = data;
			myGLCD.setFont(SmallFont);
			myGLCD.print("Dist =     ", 70, 164);   
			myGLCD.printNumI(distanceToDOM, 125, 164);
			myGLCD.setFont(BigFont);
			break;
		case 17: // Направление на объект
		    data = gps_course_deg;                  // Направление на объект
			break;
		case 18:// Скорость объекта метров в час
		    data =  gps_speed_mph;                  // Скорость объекта метров в час
			break;
		case 19:                                    // Включить питание датчика газа
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
		case 20:                                   // Отключить питание датчика газа
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
		case 21:                                   // Включить питание датчика Гейгера
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
		case 22:                                   // Отключить питание датчика Гейгера
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
		case 23:                                   // Зафиксировать местные координаты
			DOM_LAT = gps_location_lat;
			DOM_LON = gps_location_lng;
			data = 1;
			break;
		case 24:                                  
			gps_date_value = data;
			break;
		case 25:
			gps_date_year = data;
			break;                                  
		case 26:
			gps_date_month = data;
			break;
		case 27:
			gps_date_day = data;
			break;
		case 28:                                  
			gps_time_value = data;
			break;
		case 29:
			gps_time_hour = data;
			break;                                  
		case 30:
			gps_time_minute = data;
			break;
		case 31:
			gps_time_second = data;
			break;
		case 32:                                  
			gps_speed_value = data;
			break;
		case 33:
			gps_speed_mph = data;
			break;                                  
		case 34:
			gps_speed_mps = data;
			break;
		case 35:
			gps_speed_kmph = data;
			break;
		case 36:                                  
			gps_course_value = data;
			break;
		case 37:
			gps_altitude_value = data;
			break;                                  
		case 38:
			gps_altitude_kilometers = data;
			break;
		case 39:
			gps_altitude_miles = data;
			break;
		case 40:
			gps_altitude_feet = data;
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
      myGLCD.print("X ", 280, 35);
      delay(500);
      myGLCD.print("  ", 280, 35);
    }
    Serial.println("Timeout");
  }
}
void send_command(int _command)
{
    command = _command;
	myGLCD.print("   ", 212, 35);
	if(command < 10)
	{
		myGLCD.printNumI(command, 226, 35);
	}
	else
	{
	   myGLCD.printNumI(command, 212, 35);
	}
    myGLCD.print("->", 240, 35);
    Mirf.send((byte *)&command);
    delay(100);
    myGLCD.print("    ", 210, 35);
    timestamp = millis();                          // Запомнили время отправки:
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
				   FileOpen();
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
				   FileClose();
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
	DateTime now = RTC.now();
	sec          = now.second();//read time
	min          = now.minute();
	hour         = now.hour();
	dow          = now.dayOfWeek();
    day          = now.day();
	month        = now.month();
	year         = now.year();
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
  if (day < 10)
    myGLCD.printNumI(day, 272, 28);
  else
    myGLCD.printNumI(day, 264, 28);

  myGLCD.print(str_mon[month - 1], 256, 48);
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
          radio_send(19);
          delay(500);
          info_power_gaz();
        }
        if ((x >= 170) && (x <= 300))  // ОТКЛ
        {
          waitForIt(170, 110, 300, 150);
          radio_send(20);
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
        }
      }
    }
  }
}

void info_power_gaz()
{
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(0, 0, 0);
  radio_send(6);                                       // Запросить состояние питания датчика газа
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
          radio_send(21);
          delay(500);
          info_power_geiger();
        }
        if ((x >= 170) && (x <= 300))       // ОТКЛ
        {
          waitForIt(170, 110, 300, 150);
          radio_send(22);
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
  radio_send(4);                                       // Запросить состояние питания датчика газа
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

//++++++++++++++++++++++++++ Работа с файлами и SD ++++++++++++++++++++
void preob_num_str() // Программа формирования имени файла, состоящего из текущей даты и счетчика файлов
{
  DateTime now = RTC.now();
  day   = now.day();
  month = now.month();
  year  = now.year();
  int year_temp = year - 2000;
  itoa (year_temp, str_year_file, 10);                                       // Преобразование даты год в строку ( 10 - десятичный формат)

  if (month < 10)
  {
    itoa (0, str_mon_file0, 10);                                        //  Преобразование даты месяц  в строку ( 10 - десятичный формат)
    itoa (month, str_mon_file10, 10);                                   //  Преобразование числа в строку ( 10 - десятичный формат)
    sprintf(str_mon_file, "%s%s", str_mon_file0, str_mon_file10);       // Сложение 2 строк
  }
  else
  {
    itoa (month, str_mon_file, 10);                                     // Преобразование числа в строку ( 10 - десятичный формат)
  }
  if (day < 10)
  {
    itoa (0, str_day_file0, 10);                                        // Преобразование числа в строку ( 10 - десятичный формат)
    itoa (day, str_day_file10, 10);                                     // Преобразование числа в строку ( 10 - десятичный формат)
    sprintf(str_day_file, "%s%s", str_day_file0, str_day_file10);       // Сложение 2 строк
  }
  else
  {
    itoa (day, str_day_file, 10);                                          // Преобразование числа в строку ( 10 - десятичный формат)
  }

  sprintf(str1, "%s%s", str_year_file, str_mon_file);                        // Сложение 2 строк
  sprintf(str2, "%s%s", str1, str_day_file);                                 // Сложение 2 строк
  sprintf(fileName, "%s%s", str2, "00.TXT");                                 // Получение имени файла в file_name
}
#define sdError(msg) sdError_F(F(msg))

void sdError_F(const __FlashStringHelper* str) {
  cout << F("error: ");
  cout << str << endl;
  myGLCD.clrScr();
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("Error: ", CENTER, 80);
  myGLCD.print(str, CENTER, 120);
  if (card.errorCode()) {
	cout << F("SD error: ") << hex << int(card.errorCode());
	cout << ',' << int(card.errorData()) << dec << endl;
  }
	myGLCD.setColor(VGA_LIME);
	myGLCD.print(txt_info11,CENTER, 200);
	myGLCD.setColor(255, 255, 255);
	while (!myTouch.dataAvailable()){}
	while (myTouch.dataAvailable()){}

 // while (1);
}
void Draw_menu_SD()
{
	myGLCD.clrScr();
	myGLCD.setFont(BigFont);
	myGLCD.setBackColor(0, 0, 255);
	for (int x=0; x<4; x++)
		{
			myGLCD.setColor(0, 0, 255);
			myGLCD.fillRoundRect (30, 20+(50*x), 290,60+(50*x));
			myGLCD.setColor(255, 255, 255);
			myGLCD.drawRoundRect (30, 20+(50*x), 290,60+(50*x));
		}
	myGLCD.print( txt_SD_menu1, CENTER, 30);     // 
	myGLCD.print( txt_SD_menu2, CENTER, 80);      
	myGLCD.print( txt_SD_menu3, CENTER, 130);     
	myGLCD.print( txt_SD_menu4, CENTER, 180);      
}
void menu_SD()
{
	while (Serial.read() >= 0) {} // Удалить все символы из буфера
	char c;
	while (true)
		{
		delay(10);
		if (myTouch.dataAvailable())
			{
				myTouch.read();
				int	x=myTouch.getX();
				int	y=myTouch.getY();

				if ((x>=30) && (x<=290))       // Upper row
					{
					if ((y>=20) && (y<=60))    // Button: 1
						{
							waitForIt(30, 20, 290, 60);
							myGLCD.clrScr();
							myFile = sd.open("/");
							printDirectory(myFile, 0);
							Draw_menu_SD();
						}
					if ((y>=70) && (y<=110))   // Button: 2
						{
							waitForIt(30, 70, 290, 110);
							SD_info();
							Draw_menu_SD();
						}
					if ((y>=120) && (y<=160))  // Button: 3
						{
							waitForIt(30, 120, 290, 160);
							myGLCD.clrScr();
							menu_formatSD();
							Draw_menu_SD();
						}
					if ((y>=170) && (y<=220))  // Button: 4
						{
							waitForIt(30, 170, 290, 210);
							break;
						}
				}
			}
		}
}

void printDirectory(File dir, int numTabs) 
{
	char* par;
	char ext_files[3];  
	char ext_bin[] = "TXT";
	bool view_on = true;
	int count_files = 1;
	int max_count_files = 1;
	int max_count_files1 = 1;
	int min_count_files = 1;
	int count_page = 1;
	int max_count_page = 1;
	int max_count_page1 = 1;
	int count_string = 0;
	int icount = 1;
	int icount_end = 1;
	int y_fcount_start = 1;
	int y_fcount_stop = 12;
	int y_fcount_step = 1;
	int old_fcount_start = 5;
	myGLCD.clrScr();
	myGLCD.setBackColor( 0, 0, 0);
	myGLCD.setFont( SmallFont);
	myGLCD.setColor (255, 255,255);

	for( icount = 0 ;icount < 100; icount++) // Очистить память списка
	{
		for(int i = 0 ;i < 13; i ++)
		{
			list_files_tab[icount][i] = ' ';
			size_files_tab[count_files] = 0;
		}
	}


   while(true)                              // Заполнить список файлов
   {
	 File entry =  dir.openNextFile();
	 if (! entry) 
		 {
		   // no more files
		   break;
		 }
	 entry.getName(list_files_tab[count_files], 13);
	 size_files_tab[count_files] = entry.size();
	 entry.close();
	 count_files++;
   }

	myGLCD.setFont( SmallFont);
	myGLCD.setColor(0, 0, 255);
	myGLCD.fillRoundRect (5, 214, 315, 239);          // Кнопка "ESC -> PUSH"
	myGLCD.fillRoundRect (90, 189, 130, 209);         // Кнопка "<<"
	myGLCD.fillRoundRect (140, 189, 180, 209);        // Вывод номера страницы
	myGLCD.fillRoundRect (190, 189, 230, 209);        // Кнопка ">>"
	myGLCD.fillRoundRect (150, 60, 300, 120);         // Кнопка "Просмотр файла"
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect (5, 214, 315, 239);
	myGLCD.drawRoundRect (90, 189, 130, 209);
	myGLCD.drawRoundRect (140, 189, 180, 209);
	myGLCD.drawRoundRect (190, 189, 230, 209);
	myGLCD.drawRoundRect (2, 2, 318, 186);
	myGLCD.drawRoundRect (150, 60, 300, 120);        // Кнопка "Просмотр файла"
	myGLCD.setBackColor(0, 0, 0);
	myGLCD.print("Page N ",30, 193);
	myGLCD.setBackColor(0, 0, 255);
	myGLCD.print(txt_info11,CENTER, 221);            // Кнопка "ESC -> PUSH"
	myGLCD.setColor(VGA_YELLOW);
	myGLCD.print("<<",101, 193);
	myGLCD.print(">>",203, 193);
	myGLCD.setFont(BigFont);
	myGLCD.print("\x89poc\xA1o\xA4p",160, 70);       // Кнопка "Просмотр"
	myGLCD.print("\xA5""a\x9E\xA0""a",185, 90);      // Кнопка "файла"
	myGLCD.setFont( SmallFont);
	int count_str = 1;
	count_string = 0;

	for( icount = 1;icount < count_files; icount++)  //Вывод списка файдов на экран
	{
		myGLCD.setBackColor(0, 0, 0);
		myGLCD.setColor(255, 255, 255);
		myGLCD.printNumI(icount,7, count_string+5);
		myGLCD.print(list_files_tab[icount],35, count_string+5);
		count_string +=15;
		count_str ++;
				 
		if ( count_str >12)
		{
			if (icount != count_files-1)
			{
				myGLCD.setColor(0, 0, 0);
				myGLCD.fillRoundRect (3, 3, 140, 185);
				count_string = 0;
				count_str = 1;
				count_page++;
			}
		}
	}
	max_count_files = count_files;
	max_count_page =  count_page;
	max_count_files1 = count_files;
	max_count_page1 =  count_page;
	myGLCD.setColor(VGA_YELLOW);
	myGLCD.setBackColor(0, 0, 255);
	// Вывод количества страниц списка файлов
	if (count_page < 10) 
		{
			myGLCD.print("    ",146, 193);
			myGLCD.printNumI(count_page,157, 193);
		}
	if (count_page >= 10 & count_page <100 )
		{
			myGLCD.print("    ",146, 193);
			myGLCD.printNumI(count_page,153, 193);
		}
	if (count_page >= 100 ) myGLCD.printNumI(count_page,148 , 193);

	while (true)
		{

		if (myTouch.dataAvailable())
			{
				myTouch.read();
				int	x=myTouch.getX();
				int	y=myTouch.getY();

				if ((y>=214) && (y<=239))            // 
					{
					if ((x>=95) && (x<=315))         // Выход
						{
							waitForIt(5, 214, 315, 239);
							break;
						}
					}

				if ((y>=189) && (y<=209))            // 
					{
					if ((x>= 90) && (x<=130))        // Кнопки перелистывания страниц "<<"
						{
							waitForIt(90, 189, 130, 209);
							myGLCD.setColor(VGA_YELLOW);
							myGLCD.setBackColor(0, 0, 255);
							count_page--;
							if (count_page < 1) count_page = 1;
							if (count_page < 10) 
								{
									myGLCD.setColor(VGA_YELLOW);
									myGLCD.print("    ",146, 193);
									myGLCD.printNumI(count_page,157, 193);
								}
							if (count_page >= 10 & count_page <100 )
								{
									myGLCD.setColor(VGA_YELLOW);
									myGLCD.print("    ",146, 193);
									myGLCD.printNumI(count_page,153, 193);
								}
							if (count_page >= 100 ) myGLCD.printNumI(count_page,148 , 193);

							max_count_files = count_page * 12;
							min_count_files = max_count_files - 12;
							if (min_count_files <0 ) min_count_files = 0;
							if (max_count_files > count_files ) max_count_files = count_files-1;
							count_string = 0;
							myGLCD.setColor(0, 0, 0);
							myGLCD.fillRoundRect (3, 3, 140, 185);
							for( icount = min_count_files+1; icount < max_count_files+1; icount++)
							   {
								   myGLCD.setBackColor(0, 0, 0);
								   myGLCD.setColor(255, 255, 255);
								   myGLCD.print("   ",7, count_string+5);
								   myGLCD.printNumI(icount,7, count_string+5);
								   myGLCD.print(list_files_tab[icount],35, count_string+5);
								   count_string +=15;
							   }
						}
					if ((x>=190) && (x<=230))     // Кнопки перелистывания страниц "<<"
						{
							waitForIt(190, 189, 230, 209);
							myGLCD.setColor(VGA_YELLOW);
							myGLCD.setBackColor(0, 0, 255);
							count_page++;
							if (count_page > max_count_page) count_page = max_count_page;
							if (count_page < 10) 
								{
									myGLCD.setColor(VGA_YELLOW);
									myGLCD.print("    ",146, 193);
									myGLCD.printNumI(count_page,157, 193);
								}
							if (count_page >= 10 & count_page <100 )
								{
									myGLCD.setColor(VGA_YELLOW);
									myGLCD.print("    ",146, 193);
									myGLCD.printNumI(count_page,153, 193);
								}
							if (count_page >= 100 ) myGLCD.printNumI(count_page,148 , 193);

							max_count_files = count_page * 12;
							min_count_files = max_count_files - 12;
							if (min_count_files < 0 ) min_count_files = 0;
							if (max_count_files > count_files ) max_count_files = count_files-1;
							count_string = 0;
							myGLCD.setColor(0, 0, 0);
							myGLCD.fillRoundRect (3, 3, 140, 185);
							for( icount = min_count_files+1; icount < max_count_files+1; icount++)
							   {
								   myGLCD.setBackColor(0, 0, 0);
								   myGLCD.setColor(255, 255, 255);
								   myGLCD.print("   ",7, count_string+5);
								   myGLCD.printNumI(icount,7, count_string+5);
								   myGLCD.print(list_files_tab[icount],35, count_string+5);
								   count_string +=15;
							   }
						}
					}

					 y_fcount_start = 5;

					if ((x>= 30) && (x<=136))            //  Выбор файла из списка
						{
							if (count_page == max_count_page1)
								{
									y_fcount_stop = 11- ( (max_count_page1 * 12) - max_count_files1);
								}
							else
								{
									y_fcount_stop = 12;
								}

							for(y_fcount_step = 0; y_fcount_step < y_fcount_stop; y_fcount_step++)
								{
									if ((y>=y_fcount_start) && (y<=y_fcount_start+12))         // 
										{
											myGLCD.setColor(0, 0, 0);
											myGLCD.setBackColor(0, 0, 0);
											myGLCD.drawRoundRect (30, old_fcount_start, 136, old_fcount_start+12);
											waitForIt(30, y_fcount_start, 136, y_fcount_start+12);
											old_fcount_start = y_fcount_start;
											set_files = ((count_page-1) * 12)+y_fcount_step+1;
											myGLCD.setColor(VGA_YELLOW);
											myGLCD.print(list_files_tab[set_files],170, 30);     // номер файла в позиции "set_files"
											  for(int i=0;i<13;i++)
											  {
												  fileName[i] = list_files_tab[set_files][i];
											  }
											Serial.println(fileName);           // Структура = list_files_tab[номер файла в списке][имя файла]

											ext_files[0] = list_files_tab[set_files][9];         // Исключить просмотр BIN файлов
											ext_files[1] = list_files_tab[set_files][10];
											ext_files[2] = list_files_tab[set_files][11];

										if (ext_files[0] == 'B' && ext_files[1] == 'I' && ext_files[2] == 'N' )
											{
												view_on = false;
												myGLCD.setFont(BigFont);
												myGLCD.print("He\x97o\x9C\xA1o\x9B""e\xA2",146, 130);     
												myGLCD.setFont( SmallFont);
											}
										else
											{
												view_on = true;
												myGLCD.setFont(BigFont);
												myGLCD.print("          ",146, 130);     // номер файла в позиции "set_files"
												myGLCD.setFont( SmallFont);
											}
					   
										}
									 y_fcount_start += 15;
								 }
						}

					if ((x>= 150) && (x<=300))                                 //  
						{
						  if ((y>= 60) && (y<=120))                            //  Выбор
							{
								file_serial();
								waitForIt(150, 60, 300, 120);
							//	myGLCD.clrScr();
								myGLCD.clrScr();
								myGLCD.setFont( SmallFont);
								myGLCD.setColor(0, 0, 255);
								myGLCD.fillRoundRect (5, 214, 315, 239);       // Кнопка "ESC -> PUSH"
								myGLCD.fillRoundRect (90, 189, 130, 209);      // Кнопка "<<"
								myGLCD.fillRoundRect (140, 189, 180, 209);     // Вывод номера страницы
								myGLCD.fillRoundRect (190, 189, 230, 209);     // Кнопка ">>"
								myGLCD.fillRoundRect (150, 60, 300, 120);      // Кнопка "Просмотр файла"
								myGLCD.setColor(255, 255, 255);
								myGLCD.drawRoundRect (5, 214, 315, 239);
								myGLCD.drawRoundRect (90, 189, 130, 209);
								myGLCD.drawRoundRect (140, 189, 180, 209);
								myGLCD.drawRoundRect (190, 189, 230, 209);
								myGLCD.drawRoundRect (2, 2, 318, 186);
								myGLCD.drawRoundRect (150, 60, 300, 120);      // Кнопка "Просмотр файла"
								myGLCD.setBackColor(0, 0, 0);
								myGLCD.print("Page N ",30, 193);
								myGLCD.setBackColor(0, 0, 255);
								myGLCD.print(txt_info11,CENTER, 221);          // Кнопка "ESC -> PUSH"
								myGLCD.setColor(VGA_YELLOW);
								myGLCD.print("<<",101, 193);
								myGLCD.print(">>",203, 193);
								myGLCD.setFont(BigFont);
								myGLCD.print("\x89poc\xA1o\xA4p",160, 70);     // Кнопка "Просмотр"
								myGLCD.print("\xA5""a\x9E\xA0""a",185, 90);    // Кнопка "файла"
								myGLCD.setFont( SmallFont);
								myGLCD.setColor(VGA_YELLOW);
								myGLCD.setBackColor(0, 0, 255);
								if (count_page > max_count_page) count_page = max_count_page;
								if (count_page < 10) 
									{
										myGLCD.setColor(VGA_YELLOW);
										myGLCD.print("    ",146, 193);
										myGLCD.printNumI(count_page,157, 193);
									}
								if (count_page >= 10 & count_page <100 )
									{
										myGLCD.setColor(VGA_YELLOW);
										myGLCD.print("    ",146, 193);
										myGLCD.printNumI(count_page,153, 193);
									}
								if (count_page >= 100 ) myGLCD.printNumI(count_page,148 , 193);

								max_count_files = count_page * 12;
								min_count_files = max_count_files - 12;
								if (min_count_files < 0 ) min_count_files = 0;
								if (max_count_files > count_files ) max_count_files = count_files-1;
								count_string = 0;
								myGLCD.setColor(0, 0, 0);
								myGLCD.fillRoundRect (3, 3, 140, 185);
								myGLCD.setFont( SmallFont);

					        	for( icount = min_count_files+1; icount < max_count_files+1; icount++)  // Повторить вывод списка файлов
								   {
									   myGLCD.setBackColor(0, 0, 0);
									   myGLCD.setColor(255, 255, 255);
									   myGLCD.print("   ",7, count_string+5);
									   myGLCD.printNumI(icount,7, count_string+5);
									   myGLCD.print(list_files_tab[icount],35, count_string+5);
									   count_string +=15;
									 //  Serial.println(list_files_tab[icount]);
								   }
								/*file_serial();*/
							}
						}
				}
	   }
}
void file_serial()
{
		// Программа чтения данных из файла и отображения на дисплее
	int data;                    // Дата для сиволов
	uint32_t step_file = 0;      // Не применяется
	uint32_t  File_size;         // переменная хранения размера файла 

	myGLCD.clrScr();
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect (2, 2, 318, 186);
	myGLCD.setBackColor(0, 0, 0);

	myFile = sd.open(list_files_tab[set_files]);                                                     // Открыть выбранный файл
	File_size = myFile.fileSize();                                                                   // Получить размер файла 

	myGLCD.setFont( SmallFont);
	myGLCD.print("Pa\x9C\xA1""ep \xA5""a\x9E\xA0""a", 8, 138);                                     // "Размер файла"
	myGLCD.print("\x89o\x9C. \x97 \xA5""a\x9E\xA0""e             ", 8, 153);                       // "Поз. в файле"
	myGLCD.setFont(BigFont);
	myGLCD.print("\x89""epe\x99""a\xA7""a \xA5""a\x9E\xA0""a", CENTER, 60);                        // "Передача файла"
	myGLCD.print("\x97 COM \xA3op\xA4", CENTER, 80);                                               // "в СОМ порт"
	myGLCD.printNumI(File_size, 105, 135);                                                         //  Отобразить размер файла
	myGLCD.setColor(VGA_LIME);
	myGLCD.print(txt_info11,CENTER, 200);
	myGLCD.setColor(255,255,255);
	if (!myFile.isOpen())                                                                            // Прверка на ошибку открытия файла
		{
			Serial.println(F("No current root file"));
			myGLCD.print("No current file",CENTER, 100);
			return;
		}

  //char line[25];
  //int n;
 
  //// read lines from the file
  //while ((n = myFile.fgets(line, sizeof(line))) > 0) 
  //{
  //     cout << line;
  // 
  //}










	myFile.rewind();       
	
 while (myFile.available())
  {
	//  delayMicroseconds(50);
	
    Serial.write(myFile.read());
	Serial.flush();
	//delayMicroseconds(50);

  }
  delay(100);




	//while ((data = myFile.read()) >= 0 && !myTouch.dataAvailable())      
	//{
	//	Serial.write(data);
	//	myGLCD.printNumI(step_file, 105, 150);  
	//	//Serial.flush();
	//	step_file = myFile.position();  
	//	//myGLCD.printNumI(step_file, 105, 150);  
	//}
   if ((data = myFile.read()) >= 0)
	   {
		   while (!myTouch.dataAvailable()) {};  
		   while (myTouch.dataAvailable()) {};
       }
   delay(500);
 }

void Draw_menu_formatSD()
{
	myGLCD.clrScr();
	myGLCD.setBackColor(0, 0, 255);
	for (int x=0; x<4; x++)
		{
			myGLCD.setColor(0, 0, 255);
			myGLCD.fillRoundRect (30, 20+(50*x), 290,60+(50*x));
			myGLCD.setColor(255, 255, 255);
			myGLCD.drawRoundRect (30, 20+(50*x), 290,60+(50*x));
		}
	myGLCD.print( "\x89o\xA0\xA2o""e y\x99""a\xA0""e\xA2\x9D""e", CENTER, 30);     // 
	myGLCD.print( "\x8Bop\xA1""a\xA4\x9Dpo\x97""a\xA2\x9D""e", CENTER, 80);      
	myGLCD.print( "      ", CENTER, 130);     
	myGLCD.print( txt_SD_menu4, CENTER, 180);      
}
void menu_formatSD()
{
  if (!card.init(spiSpeed, chipSelect)) 
	  {
		cout << pstr(
		 "\nSD initialization failure!\n"
		 "Is the SD card inserted correctly?\n"
		 "Is chip select correct at the top of this sketch?\n");
		sdError("card.init failed");
		 myGLCD.print("File System failed", CENTER, 120);

	  }

  cardSizeBlocks = card.cardSize();
  if (cardSizeBlocks == 0) sdError("cardSize");
  cardCapacityMB = (cardSizeBlocks + 2047)/2048;

  cout << pstr("Card Size: ") << cardCapacityMB;
  cout << pstr(" MB, (MB = 1,048,576 bytes)") << endl;

  Draw_menu_formatSD();

		// discard any input
	//while (Serial.read() >= 0) {} // Удалить все символы из буфера

	char c;

	while (true)
		{
		delay(10);
		if (myTouch.dataAvailable())
			{
				myTouch.read();
				int	x=myTouch.getX();
				int	y=myTouch.getY();

				if ((x>=30) && (x<=290))       // Upper row
					{
					if ((y>=20) && (y<=60))    // Button: 1
						{
							waitForIt(30, 20, 290, 60);
							myGLCD.clrScr();
							eraseCard();
						    Draw_menu_formatSD();
						}
					if ((y>=70) && (y<=110))   // Button: 2
						{
							waitForIt(30, 70, 290, 110);
							myGLCD.clrScr();
							formatCard();
					  	    Draw_menu_formatSD();
						}
					if ((y>=120) && (y<=160))  // Button: 3
						{
							waitForIt(30, 120, 290, 160);
							myGLCD.clrScr();
					
							Draw_menu_formatSD();
						}
					if ((y>=170) && (y<=220))  // Button: 4
						{
							waitForIt(30, 170, 290, 210);
							break;
						}
				}
			}
	   }
}
void  SD_info()
{
  myGLCD.clrScr();
  myGLCD.setBackColor(0, 0, 0);
 // myGLCD.print(txt_info12, CENTER, 2);
  delay(400);  // catch Due reset problem

 // uint32_t t = millis();
   uint32_t t = micros();
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
 // if (!sd.cardBegin(SD_CS_PIN, SPI_HALF_SPEED)) 
  if (!sd.cardBegin(chipSelect, SPI_FULL_SPEED)) 
  {
	sdErrorMsg("\ncardBegin failed");
	myGLCD.print("cardBegin failed", LEFT, 100);
	//return;
  }
  t = micros() - t;
 //   t = millis() - t;
  cardSize = sd.card()->cardSize();
  if (cardSize == 0) {
	sdErrorMsg("cardSize failed");
	myGLCD.print("cardSize failed", LEFT, 120);
	//return;
  }
  cout << F("\ninit time: ") << t << " us" << endl;
  myGLCD.print("Init time : ", LEFT, 0);
  myGLCD.printNumI(t, RIGHT-60 , 0);
   myGLCD.print("us", RIGHT, 0);
  cout << F("\nCard type: ");
  myGLCD.print("Card type: ", LEFT, 20);
  switch (sd.card()->type()) {
  case SD_CARD_TYPE_SD1:
	cout << F("SD1\n");
	 myGLCD.print("SD1", RIGHT , 20);
	break;

  case SD_CARD_TYPE_SD2:
	cout << F("SD2\n");
	 myGLCD.print("SD2", RIGHT , 20);
	break;

  case SD_CARD_TYPE_SDHC:
	if (cardSize < 70000000) 
	{
	  cout << F("SDHC\n");
	   myGLCD.print("SDHC", RIGHT , 20);
	} else {
	  cout << F("SDXC\n");
	   myGLCD.print("SDXC", RIGHT , 20);
	}
	break;

  default:
	cout << F("Unknown\n");
	 myGLCD.print("Unknown", RIGHT , 20);
  }
  if (!cidDmp()) 
  {
	return;
  }
  if (!csdDmp()) 
  {
	return;
  }
  uint32_t ocr;
  if (!sd.card()->readOCR(&ocr)) 
  {
	sdErrorMsg("\nreadOCR failed");
	 myGLCD.print("readOCR failed", LEFT, 140);
	//return;
  }
  cout << F("OCR: ") << hex << ocr << dec << endl;
  if (!partDmp()) {
	return;
  }
  if (!sd.fsBegin()) {
	sdErrorMsg("\nFile System initialization failed.\n");
	myGLCD.print("File System failed", LEFT, 160);
	//return;
  }
  volDmp();

	myGLCD.setColor(VGA_LIME);
	myGLCD.print(txt_info11,CENTER, 200);
	myGLCD.setColor(255, 255, 255);
	while (!myTouch.dataAvailable()){}
	while (myTouch.dataAvailable()){}
//	Draw_menu_ADC1();

}
// write cached block to the card
uint8_t writeCache(uint32_t lbn) {
  return card.writeBlock(lbn, cache.data);
}
//------------------------------------------------------------------------------
// initialize appropriate sizes for SD capacity
void initSizes()
{
  if (cardCapacityMB <= 6) {
	sdError("Card is too small.");
  } else if (cardCapacityMB <= 16) {
	sectorsPerCluster = 2;
  } else if (cardCapacityMB <= 32) {
	sectorsPerCluster = 4;
  } else if (cardCapacityMB <= 64) {
	sectorsPerCluster = 8;
  } else if (cardCapacityMB <= 128) {
	sectorsPerCluster = 16;
  } else if (cardCapacityMB <= 1024) {
	sectorsPerCluster = 32;
  } else if (cardCapacityMB <= 32768) {
	sectorsPerCluster = 64;
  } else {
	// SDXC cards
	sectorsPerCluster = 128;
  }

  cout << F("Blocks/Cluster: ") << int(sectorsPerCluster) << endl;
  // set fake disk geometry
  sectorsPerTrack = cardCapacityMB <= 256 ? 32 : 63;

  if (cardCapacityMB <= 16) {
	numberOfHeads = 2;
  } else if (cardCapacityMB <= 32) {
	numberOfHeads = 4;
  } else if (cardCapacityMB <= 128) {
	numberOfHeads = 8;
  } else if (cardCapacityMB <= 504) {
	numberOfHeads = 16;
  } else if (cardCapacityMB <= 1008) {
	numberOfHeads = 32;
  } else if (cardCapacityMB <= 2016) {
	numberOfHeads = 64;
  } else if (cardCapacityMB <= 4032) {
	numberOfHeads = 128;
  } else {
	numberOfHeads = 255;
  }
}
//------------------------------------------------------------------------------
// zero cache and optionally set the sector signature
void clearCache(uint8_t addSig) {
  memset(&cache, 0, sizeof(cache));
  if (addSig) {
	cache.mbr.mbrSig0 = BOOTSIG0;
	cache.mbr.mbrSig1 = BOOTSIG1;
  }
}
//------------------------------------------------------------------------------
// zero FAT and root dir area on SD
void clearFatDir(uint32_t bgn, uint32_t count) 
{
  clearCache(false);
  if (!card.writeStart(bgn, count)) {
	//sdError("Clear FAT/DIR writeStart failed");
  }
  for (uint32_t i = 0; i < count; i++) {
	if ((i & 0XFF) == 0) {
	  cout << '.';
	}
	if (!card.writeData(cache.data)) {
	 // sdError("Clear FAT/DIR writeData failed");
	}
  }
  if (!card.writeStop()) {
	//sdError("Clear FAT/DIR writeStop failed");
  }
  cout << endl;
}
//------------------------------------------------------------------------------
// return cylinder number for a logical block number
uint16_t lbnToCylinder(uint32_t lbn) 
{
  return lbn / (numberOfHeads * sectorsPerTrack);
}
//------------------------------------------------------------------------------
// return head number for a logical block number
uint8_t lbnToHead(uint32_t lbn) 
{
  return (lbn % (numberOfHeads * sectorsPerTrack)) / sectorsPerTrack;
}
//------------------------------------------------------------------------------
// return sector number for a logical block number
uint8_t lbnToSector(uint32_t lbn) 
{
  return (lbn % sectorsPerTrack) + 1;
}
//------------------------------------------------------------------------------
// format and write the Master Boot Record
void writeMbr() 
{
  clearCache(true);
  part_t* p = cache.mbr.part;
  p->boot = 0;
  uint16_t c = lbnToCylinder(relSector);
  if (c > 1023) {
	sdError("MBR CHS");
  }
  p->beginCylinderHigh = c >> 8;
  p->beginCylinderLow = c & 0XFF;
  p->beginHead = lbnToHead(relSector);
  p->beginSector = lbnToSector(relSector);
  p->type = partType;
  uint32_t endLbn = relSector + partSize - 1;
  c = lbnToCylinder(endLbn);
  if (c <= 1023) {
	p->endCylinderHigh = c >> 8;
	p->endCylinderLow = c & 0XFF;
	p->endHead = lbnToHead(endLbn);
	p->endSector = lbnToSector(endLbn);
  } else {
	// Too big flag, c = 1023, h = 254, s = 63
	p->endCylinderHigh = 3;
	p->endCylinderLow = 255;
	p->endHead = 254;
	p->endSector = 63;
  }
  p->firstSector = relSector;
  p->totalSectors = partSize;
  if (!writeCache(0)) {
	sdError("write MBR");
  }
}
//------------------------------------------------------------------------------
// generate serial number from card size and micros since boot
uint32_t volSerialNumber() 
{
  return (cardSizeBlocks << 8) + micros();
}
//------------------------------------------------------------------------------
// format the SD as FAT16
void makeFat16() 
{
  uint32_t nc;
  for (dataStart = 2 * BU16;; dataStart += BU16) {
	nc = (cardSizeBlocks - dataStart)/sectorsPerCluster;
	fatSize = (nc + 2 + 255)/256;
	uint32_t r = BU16 + 1 + 2 * fatSize + 32;
	if (dataStart < r) {
	  continue;
	}
	relSector = dataStart - r + BU16;
	break;
  }
  // check valid cluster count for FAT16 volume
  if (nc < 4085 || nc >= 65525) {
	sdError("Bad cluster count");
  }
  reservedSectors = 1;
  fatStart = relSector + reservedSectors;
  partSize = nc * sectorsPerCluster + 2 * fatSize + reservedSectors + 32;
  if (partSize < 32680) {
	partType = 0X01;
  } else if (partSize < 65536) {
	partType = 0X04;
  } else {
	partType = 0X06;
  }
  // write MBR
  writeMbr();
  clearCache(true);
  fat_boot_t* pb = &cache.fbs;
  pb->jump[0] = 0XEB;
  pb->jump[1] = 0X00;
  pb->jump[2] = 0X90;
  for (uint8_t i = 0; i < sizeof(pb->oemId); i++) {
	pb->oemId[i] = ' ';
  }
  pb->bytesPerSector = 512;
  pb->sectorsPerCluster = sectorsPerCluster;
  pb->reservedSectorCount = reservedSectors;
  pb->fatCount = 2;
  pb->rootDirEntryCount = 512;
  pb->mediaType = 0XF8;
  pb->sectorsPerFat16 = fatSize;
  pb->sectorsPerTrack = sectorsPerTrack;
  pb->headCount = numberOfHeads;
  pb->hidddenSectors = relSector;
  pb->totalSectors32 = partSize;
  pb->driveNumber = 0X80;
  pb->bootSignature = EXTENDED_BOOT_SIG;
  pb->volumeSerialNumber = volSerialNumber();
  memcpy(pb->volumeLabel, noName, sizeof(pb->volumeLabel));
  memcpy(pb->fileSystemType, fat16str, sizeof(pb->fileSystemType));
  // write partition boot sector
  if (!writeCache(relSector)) {
	sdError("FAT16 write PBS failed");
  }
  // clear FAT and root directory
  clearFatDir(fatStart, dataStart - fatStart);
  clearCache(false);
  cache.fat16[0] = 0XFFF8;
  cache.fat16[1] = 0XFFFF;
  // write first block of FAT and backup for reserved clusters
  if (!writeCache(fatStart)
	  || !writeCache(fatStart + fatSize)) {
	sdError("FAT16 reserve failed");
  }
}
//------------------------------------------------------------------------------
// format the SD as FAT32
void makeFat32() 
{
  uint32_t nc;
  relSector = BU32;
  for (dataStart = 2 * BU32;; dataStart += BU32) {
	nc = (cardSizeBlocks - dataStart)/sectorsPerCluster;
	fatSize = (nc + 2 + 127)/128;
	uint32_t r = relSector + 9 + 2 * fatSize;
	if (dataStart >= r) {
	  break;
	}
  }
  // error if too few clusters in FAT32 volume
  if (nc < 65525) {
	sdError("Bad cluster count");
  }
  reservedSectors = dataStart - relSector - 2 * fatSize;
  fatStart = relSector + reservedSectors;
  partSize = nc * sectorsPerCluster + dataStart - relSector;
  // type depends on address of end sector
  // max CHS has lbn = 16450560 = 1024*255*63
  if ((relSector + partSize) <= 16450560) {
	// FAT32
	partType = 0X0B;
  } else {
	// FAT32 with INT 13
	partType = 0X0C;
  }
  writeMbr();
  clearCache(true);

  fat32_boot_t* pb = &cache.fbs32;
  pb->jump[0] = 0XEB;
  pb->jump[1] = 0X00;
  pb->jump[2] = 0X90;
  for (uint8_t i = 0; i < sizeof(pb->oemId); i++) {
	pb->oemId[i] = ' ';
  }
  pb->bytesPerSector = 512;
  pb->sectorsPerCluster = sectorsPerCluster;
  pb->reservedSectorCount = reservedSectors;
  pb->fatCount = 2;
  pb->mediaType = 0XF8;
  pb->sectorsPerTrack = sectorsPerTrack;
  pb->headCount = numberOfHeads;
  pb->hidddenSectors = relSector;
  pb->totalSectors32 = partSize;
  pb->sectorsPerFat32 = fatSize;
  pb->fat32RootCluster = 2;
  pb->fat32FSInfo = 1;
  pb->fat32BackBootBlock = 6;
  pb->driveNumber = 0X80;
  pb->bootSignature = EXTENDED_BOOT_SIG;
  pb->volumeSerialNumber = volSerialNumber();
  memcpy(pb->volumeLabel, noName, sizeof(pb->volumeLabel));
  memcpy(pb->fileSystemType, fat32str, sizeof(pb->fileSystemType));
  // write partition boot sector and backup
  if (!writeCache(relSector)
	  || !writeCache(relSector + 6)) {
	sdError("FAT32 write PBS failed");
  }
  clearCache(true);
  // write extra boot area and backup
  if (!writeCache(relSector + 2)
	  || !writeCache(relSector + 8)) {
	sdError("FAT32 PBS ext failed");
  }
  fat32_fsinfo_t* pf = &cache.fsinfo;
  pf->leadSignature = FSINFO_LEAD_SIG;
  pf->structSignature = FSINFO_STRUCT_SIG;
  pf->freeCount = 0XFFFFFFFF;
  pf->nextFree = 0XFFFFFFFF;
  // write FSINFO sector and backup
  if (!writeCache(relSector + 1)
	  || !writeCache(relSector + 7)) {
	sdError("FAT32 FSINFO failed");
  }
  clearFatDir(fatStart, 2 * fatSize + sectorsPerCluster);
  clearCache(false);
  cache.fat32[0] = 0x0FFFFFF8;
  cache.fat32[1] = 0x0FFFFFFF;
  cache.fat32[2] = 0x0FFFFFFF;
  // write first block of FAT and backup for reserved clusters
  if (!writeCache(fatStart)
	  || !writeCache(fatStart + fatSize)) {
	sdError("FAT32 reserve failed");
  }
}
//------------------------------------------------------------------------------
// flash erase all data
void eraseCard()
{
  cout << endl << F("Erasing\n");
  uint32_t firstBlock = 0;
  uint32_t lastBlock;
  uint16_t n = 0;

  do {
	lastBlock = firstBlock + ERASE_SIZE - 1;
	if (lastBlock >= cardSizeBlocks) {
	  lastBlock = cardSizeBlocks - 1;
	}
	if (!card.erase(firstBlock, lastBlock)) {
	  sdError("erase failed");
	}
	cout << '.';
	if ((n++)%32 == 31) {
	  cout << endl;
	}
	firstBlock += ERASE_SIZE;
  } while (firstBlock < cardSizeBlocks);
  cout << endl;

  if (!card.readBlock(0, cache.data)) {
	sdError("readBlock");
  }
  cout << hex << showbase << setfill('0') << internal;
  cout << F("All data set to ") << setw(4) << int(cache.data[0]) << endl;
  cout << dec << noshowbase << setfill(' ') << right;
  cout << F("Erase done\n");
}
//------------------------------------------------------------------------------
void formatCard() 
{
  cout << endl;
  cout << F("Formatting\n");
  initSizes();
  if (card.type() != SD_CARD_TYPE_SDHC) {
	cout << F("FAT16\n");
	makeFat16();
  } else {
	cout << F("FAT32\n");
	makeFat32();
  }
#if DEBUG_PRINT
  debugPrint();
#endif  // DEBUG_PRINT
  cout << F("Format done\n");
}

// store error strings in flash
#define sdErrorMsg(msg) sdErrorMsg_F(F(msg));
void sdErrorMsg_F(const __FlashStringHelper* str) 
{
  cout << str << endl;
  myGLCD.clrScr();
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("Error: ", CENTER, 80);
  myGLCD.print(str, CENTER, 120);

  if (sd.card()->errorCode()) {
	cout << F("SD errorCode: ");
	cout << hex << int(sd.card()->errorCode()) << endl;
	cout << F("SD errorData: ");
	cout << int(sd.card()->errorData()) << dec << endl;
  }
  delay(3000);
}
void sdErrorMsg_P(const char* str) 
{
  cout << pgm(str) << endl;
  myGLCD.clrScr();
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("Error: ", CENTER, 80);
  myGLCD.print(str, CENTER, 120);
  if (card.errorCode()) {
	cout << pstr("SD errorCode: ");
	cout << hex << int(card.errorCode()) << endl;
	cout << pstr("SD errorData: ");
	cout << int(card.errorData()) << dec << endl;
  }
	delay(2000);
}

//------------------------------------------------------------------------------
uint8_t cidDmp() 
{
  cid_t cid;
  if (!sd.card()->readCID(&cid)) {
	sdErrorMsg("readCID failed");
	return false;
  }
  cout << F("\nManufacturer ID: ");
  cout << hex << int(cid.mid) << dec << endl;
  cout << F("OEM ID: ") << cid.oid[0] << cid.oid[1] << endl;
  cout << F("Product: ");
  for (uint8_t i = 0; i < 5; i++) 
  {
	cout << cid.pnm[i];
  }
  cout << F("\nVersion: ");
  cout << int(cid.prv_n) << '.' << int(cid.prv_m) << endl;
  cout << F("Serial number: ") << hex << cid.psn << dec << endl;
  cout << F("Manufacturing date: ");
  cout << int(cid.mdt_month) << '/';
  cout << (2000 + cid.mdt_year_low + 10 * cid.mdt_year_high) << endl;
  cout << endl;
  return true;
}
//------------------------------------------------------------------------------
uint8_t csdDmp() {
  csd_t csd;
  uint8_t eraseSingleBlock;
  if (!sd.card()->readCSD(&csd)) {
	sdErrorMsg("readCSD failed");
	return false;
  }
  if (csd.v1.csd_ver == 0) {
	eraseSingleBlock = csd.v1.erase_blk_en;
	eraseSize = (csd.v1.sector_size_high << 1) | csd.v1.sector_size_low;
  } else if (csd.v2.csd_ver == 1) {
	eraseSingleBlock = csd.v2.erase_blk_en;
	eraseSize = (csd.v2.sector_size_high << 1) | csd.v2.sector_size_low;
  } else {
	cout << F("csd version error\n");
	return false;
  }
  eraseSize++;
  cout << F("cardSize: ") << 0.000512*cardSize;
   myGLCD.print("cardSize: ", LEFT, 40);
  myGLCD.printNumI(0.000512*cardSize, RIGHT-60 , 40);
   myGLCD.print("MB", RIGHT, 40);

  cout << F(" MB (MB = 1,000,000 bytes)\n");

  cout << F("flashEraseSize: ") << int(eraseSize) << F(" blocks\n");
  cout << F("eraseSingleBlock: ");
  if (eraseSingleBlock) {
	cout << F("true\n");
  } else {
	cout << F("false\n");
  }
  return true;
}
//------------------------------------------------------------------------------
// print partition table
uint8_t partDmp() {
  cache_t *p = sd.vol()->cacheClear();
  if (!p) {
	sdErrorMsg("cacheClear failed");
	return false;
  }
  if (!sd.card()->readBlock(0, p->data)) {
	sdErrorMsg("read MBR failed");
	return false;
  }
  for (uint8_t ip = 1; ip < 5; ip++) {
	part_t *pt = &p->mbr.part[ip - 1];
	if ((pt->boot & 0X7F) != 0 || pt->firstSector > cardSize) {
	  cout << F("\nNo MBR. Assuming Super Floppy format.\n");
	  return true;
	}
  }
  cout << F("\nSD Partition Table\n");
  cout << F("part,boot,type,start,length\n");
  for (uint8_t ip = 1; ip < 5; ip++) {
	part_t *pt = &p->mbr.part[ip - 1];
	cout << int(ip) << ',' << hex << int(pt->boot) << ',' << int(pt->type);
	cout << dec << ',' << pt->firstSector <<',' << pt->totalSectors << endl;
  }
  return true;
}
//------------------------------------------------------------------------------
void volDmp() 
{
  cout << F("\nVolume is FAT") << int(sd.vol()->fatType()) << endl;
  myGLCD.print("Volume is      FAT", LEFT, 60);
  int volFAT = sd.vol()->fatType();
  myGLCD.printNumI(volFAT, RIGHT , 60);
  cout << F("blocksPerCluster: ") << int(sd.vol()->blocksPerCluster()) << endl;
  cout << F("clusterCount: ") << sd.vol()->clusterCount() << endl;
  cout << F("freeClusters: ");
  uint32_t volFree = sd.vol()->freeClusterCount();
  cout <<  volFree << endl;
  float fs = 0.000512*volFree*sd.vol()->blocksPerCluster();
  cout << F("freeSpace: ") << fs << F(" MB (MB = 1,000,000 bytes)\n");
   myGLCD.print("freeSpace: ", LEFT, 80);
  myGLCD.printNumI(fs, RIGHT-60 , 80);
   myGLCD.print("MB", RIGHT, 80);
  cout << F("fatStartBlock: ") << sd.vol()->fatStartBlock() << endl;
  cout << F("fatCount: ") << int(sd.vol()->fatCount()) << endl;
  cout << F("blocksPerFat: ") << sd.vol()->blocksPerFat() << endl;
  cout << F("rootDirStart: ") << sd.vol()->rootDirStart() << endl;
  cout << F("dataStartBlock: ") << sd.vol()->dataStartBlock() << endl;
  if (sd.vol()->dataStartBlock() % eraseSize) {
	cout << F("Data area is not aligned on flash erase boundaries!\n");
	cout << F("Download and use formatter from www.sdsd.card()->org/consumer!\n");
  }
}

void FileOpen()
{
 // Serial.println("FileOpen");
  int temp_file_name = 0;
  preob_num_str();
  while (sd.exists(fileName))
  {
    if (fileName[BASE_NAME_SIZE + 1] != '9')
    {
      fileName[BASE_NAME_SIZE + 1]++;
    }
    else if (fileName[BASE_NAME_SIZE] != '9')
    {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    }
    else
    {
                                  // Флаг ошибки  открытия файла
    }
  }
  if (!myFile.open(fileName, O_CREAT | O_WRITE | O_EXCL)) //sdError("file.open");
  {
                               // Флаг ошибки  открытия файла
  }
  else
  {
    Serial.print(fileName);
   // Serial.println(F("  Open Ok!"));
    myFile.print ("Start measure  ");
    file_print_date();
    myFile.println ("");
    myFile.println ("");
    Serial.println(fileName);
  }
}
void FileClose()
{
  //Serial.println(fileName);
  myFile.println ("");
  myFile.print ("Stop measure  ");
  file_print_date();
  myFile.println ("");
  myFile.close();

  if (sd.exists(fileName))
  {
    Serial.println();
    Serial.print(fileName);
    //Serial.println("  Close  OK!.");
  }
  else
  {
    Serial.println();
    Serial.print(fileName);
    //Serial.println(" doesn't exist.");
  }
}

void file_print_date()  //программа  записи даты в файл
{
	DateTime now = RTC.now();
	myFile.print(now.day(), DEC);
	myFile.print('/');
	myFile.print(now.month(), DEC);
	myFile.print('/');
	myFile.print(now.year(), DEC);//Serial display time
	myFile.print(' ');
	myFile.print(now.hour(), DEC);
	myFile.print(':');
	myFile.print(now.minute(), DEC);
	myFile.print(':');
	myFile.print(now.second(), DEC);
}

void setup()
{
	Serial.begin(115200);
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
   //if (!sd.begin(chipSelect, SPI_FULL_SPEED)) 
	if (!sd.begin(chipSelect, SPI_HALF_SPEED)) 
		{
		sd.initErrorPrint();
		myGLCD.clrScr();
		myGLCD.setBackColor(0, 0, 0);
		myGLCD.setColor(255, 100, 0);
		myGLCD.print("Can't access SD card",CENTER, 40);
		myGLCD.print("Do not reformat",CENTER, 70);
		myGLCD.print("SD card problem?",CENTER, 100);
		myGLCD.setColor(VGA_LIME);
		myGLCD.print(txt_info11,CENTER, 200);
		myGLCD.setColor(255, 255, 255);
		while (myTouch.dataAvailable()){}
		delay(50);
		while (!myTouch.dataAvailable()){}
		delay(50);
		myGLCD.clrScr();
		myGLCD.print("Run Setup", CENTER,120);
		delay(500);
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