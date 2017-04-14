#include <RTClib.h>


#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"

// A simple data logger for the Arduino analog pins

// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  1000 // mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to 
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

int thermo0=A0;
int thermo1=A1;
int thermo2=A2;
int thermo3=A3;


RTC_DS1307 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  

  while(1);
}

void setup(void)
{
  Serial.begin(9600);
  Serial.println();
  
  
#if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available());
#endif //WAIT_TO_START

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  

  logfile.println("millis,voltage0,voltage1,voltage2,voltage3");    
#if ECHO_TO_SERIAL
  Serial.println("millis,voltage0,voltage1,voltage2,voltage3");
#endif //ECHO_TO_SERIAL
 
  // If you want to set the aref to something other than 5v
  
}

void loop(void)
{
  DateTime now;

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print(", ");    
#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print(", ");  
#endif

  analogRead(thermo0);
  delay(20); 
  float volt0 = analogRead(thermo0);  

  analogRead(thermo1);
  delay(20); 
  float volt1 = analogRead(thermo1); 

  analogRead(thermo2);
  delay(20); 
  float volt2 = analogRead(thermo2); 

  analogRead(thermo3);
  delay(20); 
  float volt3 = analogRead(thermo3); 
  
   volt0=volt0*5/1024;
   volt1=volt1*5/1024;
   volt2=volt2*5/1024;
   volt3=volt3*5/1024;
  
  
     
  logfile.print(volt0);
  logfile.print(", ");    
  logfile.print(volt1);
  logfile.print(", ");    
  logfile.print(volt2);
  logfile.print(", ");    
  logfile.print(volt3);
#if ECHO_TO_SERIAL
    
  Serial.print(volt0);
  Serial.print(", ");    
  Serial.print(volt1);
  Serial.print(", ");    
  Serial.print(volt2);
  Serial.print(", ");    
  Serial.print(volt3);
#endif //ECHO_TO_SERIAL

  logfile.println();
#if ECHO_TO_SERIAL
  Serial.println();
#endif // ECHO_TO_SERIAL

  
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
 logfile.flush();
}


