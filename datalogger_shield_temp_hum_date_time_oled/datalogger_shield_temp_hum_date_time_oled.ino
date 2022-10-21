// Before flashing this script, you must set the time (if you didn't do it already).
// Hence, flash the example from the RTCLib library (ds1307 or softrtc).
// After that, the time is set, and if you have a battery it is kept into your Arduino.

// ## IMPORT LIBRARIES and instantiate objects

// for oled - library that uses reduced memory
//#include <Arduino.h>
#include <U8x8lib.h>

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8( /* reset=*/ U8X8_PIN_NONE);

// for date logging, date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"

RTC_DS1307 rtc;

// for temp sensor
#include <DHT.h>

#include <Adafruit_Sensor.h>

// for SD reader
#include <SPI.h>

#include <SD.h>

// DHT SENSOR: Initialize DHT sensor for normal 16 mHz Arduino:
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht = DHT(DHTPIN, DHTTYPE);

// SD MODULE: change this to match the CS pin of the SD module
const int chipSelect = 10;

// choose the LEDs pins (we use some free one)
//#define led1 3
//#define led2 4
////digitalWrite(led1, HIGH);

// ## SET USEFUL VARIABLES

// cannot be more than 8 chars by FAT standard
char filename[] = "hhmmss.txt"; // the filename array, I will change it with the date_time

// variables for date and time
char current_date[] = "yyyy/mm/dd";
char current_date_oled[] = "dd/mm/yyyy";
char current_time[] = "hh:mm:ss";

File myFile; // my log file

int id = 1; // to later increase the ID on the log file

int log_frequency = 1000; // log frequency in ms

// ## FUNCTIONS

// to save log file with correct date and time on the system
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}


void setup() {

  // Begin serial communication at a baud rate of 9600
  Serial.begin(9600);

  // initialise the oled
  u8x8.begin();

  #ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
  #endif

  rtc.begin();

//  rtc.adjust(DateTime(__DATE__, __TIME__));

  /* IMPORTANT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  Flash the first time with only the rtc.adjust(...) line to set the current time.
  Soon after, flash the code as it appears now with the "if", otherwise each time
  you will connect the device to something, it will try to set the date and time again,
  and usually, it will fail. */

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }

//      // check if rtc is being recognised
//      if (! rtc.begin()) {
//        Serial.println(F("Couldn't find RTC"));
//        Serial.flush();
//        abort();
//      }

  // let's start fetching the date_time to create the unique filename
  DateTime now = rtc.now(); // take the current time

  // create filename == hhmmss.txt (note: the filename cannot be > 8 chars by FAT standard)
  sprintf(filename, "%02d%02d%02d.txt", now.hour(), now.minute(), now.second());

  // Setup DHT sensor
  dht.begin();

  // initialise SD card
  Serial.print(F("Initializing SD card..."));

  if (!SD.begin()) {
    Serial.println(F("initialization failed!"));
    return;
  }
  Serial.println(F("initialization done."));

  // this is needed to show the correct date and time of saving on the PC
  SdFile::dateTimeCallback(dateTime);
  // initialise data file with headers
  myFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write the headers in it
  if (myFile) {
    Serial.print(F("Initialising data file..."));
    // write the headers in one line comma separated, every time the Arudino starts (ID, Datetime, Temp, Hum, Flag)
    myFile.print("ID");
    myFile.print(",");
    myFile.print("date");
    myFile.print(",");
    myFile.print("time");
    myFile.print(",");
    myFile.print("Temperature");
    myFile.print(",");
    myFile.println("Humidity");

    // close the file:
    myFile.close();

    Serial.println(F("Done, all good!"));
  } else {
    Serial.println(F("SOMETHING WENT WRONG - couldn't initilise the log file on the SD card"));
  }
}

void loop() {

  DateTime now = rtc.now(); // take the current time

  // Read the humidity (in %) from the DHT22 sensor
  float h = dht.readHumidity();
  // Read the temperature (in Celsius) from the DHT22 sensor
  float t = dht.readTemperature();

  // Check if everything's good with the reading from the DHT22:
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // write the headers (our columns) on the SD card log file
  myFile = SD.open(filename, FILE_WRITE);

  if (myFile) { // if the file opened okay, write to it:

    Serial.println("I'm writing"); // for debug purposes

    // create current_date variable
    sprintf(current_date, "%4i/%02d/%02d", now.year(), now.month(), now.day());

    // create current_time variable
    sprintf(current_time, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    // create current_date variable to display in oled
    sprintf(current_date_oled, "%02d/%02d/%4i", now.day(), now.month(), now.year());

    // log into file on the SD card
    myFile.print(id);
    myFile.print(",");
    myFile.print(current_date);
    myFile.print(",");
    myFile.print(current_time);
    myFile.print(",");
    myFile.print(t);
    myFile.print(",");
    myFile.println(h);

    // debug on serial
    //    Serial.print(id);Serial.print(",");
    //    Serial.print(current_date);Serial.print(",");
    //    Serial.print(current_time);Serial.print(",");
    //    Serial.print(t);Serial.print(",");
    //    Serial.println(h);

    myFile.close(); // close the log file

    id++; // increase the id number
  }

  // now set the font and then 
  // the cursor position
  // and print the text and variables to the oled
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);

  u8x8.setCursor(0, 0); // col, raw
  u8x8.print("Temp:");
  u8x8.setCursor(6, 0);
  u8x8.print(t);

  u8x8.setCursor(0, 2);
  u8x8.print("Hum:");
  u8x8.setCursor(6, 2);
  u8x8.print(h);

  u8x8.setCursor(0, 5);
  u8x8.print("Date:");
  u8x8.setCursor(6, 5);
  u8x8.print(current_date_oled);

  u8x8.setCursor(0, 7);
  u8x8.print("Time:");
  u8x8.setCursor(6, 7);
  u8x8.print(current_time);

  delay(log_frequency); // log and display data every X second/s
}
