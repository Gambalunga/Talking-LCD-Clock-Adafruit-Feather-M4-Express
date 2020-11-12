// Based on the Clock example using a seven segment display & DS1307 real-time clock.
//
// Must have the Adafruit RTClib library installed too!  See:
//   https://github.com/adafruit/RTClib
//
// Designed specifically to work with the Adafruit LED 7-Segment backpacks
// and DS1307 real-time clock breakout (note this sketch uses the Adalogger Featherwing- see below):
// ----> http://www.adafruit.com/products/881
// ----> http://www.adafruit.com/products/880
// ----> http://www.adafruit.com/products/879
// ----> http://www.adafruit.com/products/878,
// please support Adafruit and open-source hardware by purchasing
// products from Adafruit!
//
// Original example sketch was written by Tony DiCola for Adafruit Industries.
// ----> https://www.adafruit.com/products/264
//
// Adafruit invests time and resources providing this open source code
// Released under a MIT license: https://opensource.org/licenses/MIT
/*
  Modified and added to by Peter Bradley
  This version uses an Adafruit Feather M4 Express and the Adalogger Featherwing which has the PCF8523 RTC
  Uses the Daylight Savings Time library by Andy Doro which was further modified for EU DST
  ----> https://github.com/andydoro/DST_RTC
  An Adafruit Mono 2.5W Class D Audio Amplifier - PAM8302 was added to provide sound for an alarm when connected to the DAC pin A0.
  ----> https://www.adafruit.com/product/2130
  Audio library used is: Audio - Adafruit Fork https://github.com/adafruit/Audio
  Buttons were added to advance and retard the time for the alarm and the clock (if necessary)
  and a switch to turn the alarm on and off.
  This version also uses a library of recorded spoken numbers and words to speak the time when a button is pressed for 1 second.
  One second is required because of the delay of one second used in every iteration of the loop.
*/

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <RTClib.h>
#include <DST_RTC.h> // Daylight Savings Time Library (Modified for Europe)
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include <Audio.h>    // Audio - Adafruit Fork https://github.com/adafruit/Audio
#include <play_fs_wav.h>

AudioPlayFSWav           playWav1;
AudioOutputAnalogStereo  audioOutput;    // Dual DACs
//AudioConnection          patchCord1(playWav1, 0, audioOutput, 1); //For stero uncomment and comment the line below
AudioConnection          patchCord1(playWav1, 1, audioOutput, 0);  // Mono - output both on the same dac comment this and uncomment above for stereo
AudioConnection          patchCord2(playWav1, 1, audioOutput, 0);

// Clock parameters **************************************
// Set to false to display time in 12 hour format, or true to use 24 hour:
#define TIME_24_HOUR      false

// Set to false if the alram button is not connected:
#define Alarm false

// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS   0x70

// Create display and DS1307 objects.  These are global variables that
// can be accessed from both the setup and loop function below.
Adafruit_7segment clockDisplay = Adafruit_7segment();

RTC_PCF8523 rtc;

DST_RTC dst_rtc; // DST object

// Define US or EU rules for DST comment out as required. More countries could be added with different rules in DST_RTC.cpp
//  const char rulesDST[] = "US"; // US DST rules
const char rulesDST[] = "EU";   // EU DST rules

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int hours = 0;
int minutes = 0;
int seconds = 0;
unsigned long time_now; // used to store current millisecond

// Remember if the colon was drawn on the display so it can be blinked
// on and off every second.
bool blinkColon = false;

int buttonTalk = 6; // connect pin to ground via push button to pull pin down used to trigger playTime()
int buttonAdv = 9; // connect pin to ground via push button to pull pin down used to fast advance minutes.
int buttonRev = 10; // connect pin to ground via push button to pull pin down used to fast reverse minutes.
int switchAlarm = 11;
int switchAmp = 12;  // connect to SD on the amplifier
int t;            // time of button press, used to speed up advance and retard loops
int displayValue;
int clockValue;
int alarmValue;
bool displayDelay = true;
DateTime theTime;
DateTime now;

// Audio SPI Flash parameters ***************************
#define SDCARD_CS_PIN    10

#if defined(EXTERNAL_FLASH_USE_QSPI)
Adafruit_FlashTransport_QSPI flashTransport;

#elif defined(EXTERNAL_FLASH_USE_SPI)
Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

#else
#error No QSPI/SPI flash are defined on your board variant.h !
#endif

Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatFileSystem QSPIFS;
SdFat SD;
bool SDOK = false, QSPIOK = false;

bool Talk = false;


int Alarmhours = 8;      // default Alarm time if not changed
int Alarmminutes = 30;


void setup() {
  delay(1000);
  Serial.begin(115200);
  //  while (!Serial);  // wait for serial port to connect. Needed for native USB

  AudioMemory(8);

  pinMode(buttonTalk, INPUT_PULLUP);
  pinMode(buttonAdv, INPUT_PULLUP);
  pinMode(buttonRev, INPUT_PULLUP);
  pinMode(switchAlarm, INPUT_PULLUP);  // High is alarm on. Must be jumpered low if alarm and an Alarm button not used.
  pinMode(switchAmp, OUTPUT);    //  digital pin connected to SD on amp
  digitalWrite(switchAmp, LOW);  // put amp on standby


  Serial.print(Alarmhours);
  Serial.print(":");
  Serial.println(Alarmminutes);

  // Clock setup *************************************
  // Setup the display.
  clockDisplay.begin(DISPLAY_ADDRESS);
  clockDisplay.setBrightness(2);   // set brightness between 0 and 15
  rtc.begin();

  /************************************************************************************/
  // This line sets the RTC with an explicit date & time, for example to set
  // May 21, 2020 at 23:50:5 you would call:*/
  // rtc.adjust(DateTime(2020, 5, 21, 23, 50, 5));
  // Compile and load again with this line commented out or the clock will revert to this time on power up or reset
  // This funtion would only be used if the RTC has had the battery removed or discharged.
  /************************************************************************************/

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled only in the case that the clock has not already been set.
    rtc.adjust(DateTime(__DATE__, __TIME__));
    // DST? If we're in it, let's subtract an hour from the RTC time to keep our DST calculation correct. This gives us
    // Standard Time which our DST check will add an hour back to if we're in DST.
    now = rtc.now();
    if (dst_rtc.checkDST(now) == true) { // check whether we're in DST right now. If we are, subtract an hour.
      now = now.unixtime() - 3600;
    }
    rtc.adjust(now);
  }

  now = rtc.now(); // This is the standard time and the time the RTC is set to

  theTime = dst_rtc.calculateTime(now); // takes into account DST

  // Print time to control settings
  // Serial.println("Standard Time");
  // printTheTime(now);

  // Serial.println("Time adjusted for Daylight Saving Time");
  // printTheTime(theTime);

  SDOK = false;
  if (!(SD.begin(SDCARD_CS_PIN))) {
    Serial.println("Unable to access the SD card");
  } else {
    //  Serial.println("SD card OK!");
    SDOK = true;
  }
  /*// Commented out as we are not using the flash memeory
    // Initialize flash library and check its chip ID.
    if (!flash.begin()) {
    Serial.println("Error, failed to initialize flash chip!");
    while (1);
    }
    // First call begin to mount the filesystem.  Check that it returns true
    // to make sure the filesystem was mounted.
    if (!QSPIFS.begin(&flash)) {
    Serial.println("Failed to mount filesystem!");
    Serial.println("Was CircuitPython loaded on the board first to create the filesystem?");
    while (1);
    }
    else {
    Serial.println("Mounted filesystem!");
    QSPIOK = true;
    }*/

}    // end setup

void loop() {
  displayTime();

  if (digitalRead(switchAlarm) == LOW) {  // Time adjustment mode
    while (digitalRead(buttonAdv) == LOW) {       //******************* minutes fast forward routine
      now = rtc.now();
      rtc.adjust(rtc.now() + TimeSpan(60));   // Add 1 minute to RTC
      displayTime();
      delay(500);
      if (digitalRead(buttonAdv) == HIGH) {
        break;        //  exit routine if button released
      }
    }    //****************** end minutes fast forward routine*/

    while (digitalRead(buttonRev) == LOW) {       //****************** minutes fast backward routine
      now = rtc.now();
      rtc.adjust(rtc.now() - TimeSpan(60));   // deduct 1 minute to RTC
      displayTime();
      delay(500);
      if (digitalRead(buttonRev) == HIGH) {
        break;        //  exit routine if button released
      }
    }    //****************** end minutes fast backward routine*/

  } //*************************** End time adjustment section

  // *************************** Speak the time section
  if (digitalRead(buttonTalk) == LOW) {  // playTime() activated
    time_now = millis();
    Talk = true;
    playTime();
  }
  if (millis() >= (time_now + 10000) && (Talk == true)) {
    digitalWrite(switchAmp, LOW);  // put amp on standby after 10secs.
    Talk = false;
  }
  // End Speak the time section *****************


  // *************************** Alarm section
  if (Alarm) {
    if (digitalRead(switchAlarm) == HIGH) {  // Alarm activated (alarm time always displayed in 24 hour mode)
      if (displayDelay == true) {  // show alarm time for 3 seconds
        alarmValue = Alarmhours * 100 + Alarmminutes;
        writeClockDisplay(alarmValue);
        // TalkAlarm(Alarmhours,Alarmminutes);    // to be done if talking alarm time is required.
        delay(3000);
        displayDelay = false;
      }
      while (digitalRead(buttonAdv) == LOW) {
        alarmAdvance();   //******************* Alarm fast forward routine
      }
      while (digitalRead(buttonRev) == LOW) {
        alarmRetard(); //****************** Alarm fast backward routine
      }
      // Play alarm
      if (Alarmhours == hours && Alarmminutes == minutes && seconds == 0) {
        playAlarm();
        time_now = millis();
      }
      if (millis() >= (time_now + 240000) && (digitalRead(switchAmp) == HIGH) ) {
        digitalWrite(switchAmp, LOW);  // put amp on standby after 4 minutes if not otherwise put on standby.
      }
    }
    if (digitalRead(switchAlarm) == LOW  && Talk == false) {  // Alarm mode off
      digitalWrite(switchAmp, LOW);  // put amp on standby
      displayDelay = true;  // reset dispayDelay for the next time that the alarm is turned on
    }
  }// End alarm mode *****************

  // Pause for a second for time to elapse.  This value is in milliseconds
  // so 1000 milliseconds = 1 second.
  delay(1000);
}        // end of loop

void displayTime() {
  now = rtc.now();
  theTime = dst_rtc.calculateTime(now); // takes into account DST
  hours = theTime.hour();
  minutes = theTime.minute();
  seconds = theTime.second(); //used to trigger the alarm
  // Show the time on the display by turning it into a numeric
  // value, like 3:30 turns into 330, by multiplying the hour by
  // 100 and then adding the minutes.
  clockValue = hours * 100 + minutes;

  // Do 24 hour to 12 hour format conversion when required.
  if (!TIME_24_HOUR) {
    // Handle when hours are past 12 by subtracting 12 hours (1200 value).
    if (hours > 12) {
      clockValue -= 1200;
    }
    // Handle hour 0 (midnight) being shown as 12.
    else if (hours == 0) {
      clockValue += 1200;
    }
  }
  // Now print the time value to the display.
  writeClockDisplay(clockValue);
}

void writeClockDisplay(int displayValue) {
  clockDisplay.print(displayValue, DEC);

  // Add zero padding when in 24 hour mode and it's midnight.
  // In this case the print function above won't have leading 0's
  // which can look confusing.  Go in and explicitly add these zeros.
  if (displayValue < 100) {
    // Pad hour 0.
    clockDisplay.writeDigitNum(1, 0);
    // Also pad when the 10's minute is 0 and should be padded.
    if (displayValue < 10) {
      clockDisplay.writeDigitNum(3, 0);  // was 'clockDisplay.writeDigitNum(2, 0);' which would be trying to write '0' to the colon.
    }
  }
  // Blink the colon by flipping its value every loop iteration
  // (which happens every second).
  blinkColon = !blinkColon;
  clockDisplay.drawColon(blinkColon);
  if ((digitalRead(switchAlarm) == HIGH) && (Alarm)) { // Alarm mode on and Alarm activated
    clockDisplay.writeDigitNum(4, displayValue % 10 , true); // Write last dot to show Alarm is on
  }
  // Now push out to the display the new values that were set above.
  clockDisplay.writeDisplay();
}

void alarmAdvance() {
  t = 0;
  for (Alarmminutes; Alarmminutes <= 60; Alarmminutes++) {
    if (Alarmminutes == 60) {
      Alarmminutes = 0;
      Alarmhours ++;
    }
    if (Alarmhours == 24) {
      Alarmhours = 0;
    }
    // Serial.print(Alarmhours);
    // Serial.print(":");
    // Serial.println(Alarmminutes);
    t++;
    if (t <= 15) {
      delay(500);
    }
    else {
      delay(50);
    }
    alarmValue = Alarmhours * 100 + Alarmminutes;
    writeClockDisplay(alarmValue);

    if (digitalRead(buttonAdv) == HIGH) {
      delay(2000); // delay 2 seconds showing alarm time
      break;        //  exit routine if button released
    }
  }
}   //****************** end alarm minutes fast forward routine*/

void alarmRetard() {
  t = 0;
  for (Alarmminutes; Alarmminutes >= -1; Alarmminutes--) {
    if (Alarmminutes <= -1) {
      Alarmminutes = 59;
      Alarmhours --;
    }
    if (Alarmhours <= -1) {
      Alarmhours = 23;
    }
    // Serial.print(Alarmhours);
    // Serial.print(":");
    // Serial.println(Alarmminutes);
    t++;
    if (t <= 15) {
      delay(500);
    }
    else {
      delay(50);
    }
    alarmValue = Alarmhours * 100 + Alarmminutes;
    writeClockDisplay(alarmValue);
    if (digitalRead(buttonRev) == HIGH) {
      delay(2000); // delay 2 seconds showing alarm time
      break;        //  exit routine if button released
    }
  }
}    //****************** end alarm minutes fast backward routine*/

void playAlarm() {
  digitalWrite(switchAmp, HIGH);  // put amp off standby
  playFile("Alarm.wav");
  // Serial.println("playing audio file");
}

/* void TalkAlarm(int Ahours, int Aminutes) {  // to be done if talking alarm time is required.
}    //****************** end TalkAlarm routine*/


void playTime() {
  digitalWrite(switchAmp, HIGH);  // put amp off standby
  now = rtc.now();
  theTime = dst_rtc.calculateTime(now); // takes into account DST
  hours = theTime.hour();
  minutes = theTime.minute();

  // Do 24 hour to 12 hour format conversion when required.
  if (!TIME_24_HOUR) {
    // Handle when hours are past 12 by subtracting 12 hours.
    if (hours > 12) {
      hours = hours - 12 ;
    }
    // Handle hour 0 (midnight) being shown as 12.
    else if (hours == 0) {
      hours = 12;
    }

  }

  char  hourWav[10] = {"00.wav"}; // construct series of filenames to speak hour and minute
  String hourString = String(hours) + ".wav";
  Serial.println(hourString);
  hourString.toCharArray(hourWav, 10);
  Serial.println(hourWav);

  char  minWav[10] = {"00.wav"}; // construct series of filenames to speak hour and minute
  String minString = String(minutes) + ".wav";
  minString.toCharArray(minWav, 10);

  playFile("timeis.wav"); // Example recorded voice output should be "The time is 4 hours and 45 minutes" for 4:45
  playFile(hourWav);
  //    playFile("hours.wav"); // Example recorded voice output should be "hours"
  //  playFile("and.wav"); // Example recorded voice output should be "and"
  if (minutes > 0 && minutes < 10) {
    playFile("oh.wav"); // this is used to have the more natural "The time is" "four" "oh" "five" for 4:05
  }

  playFile(minWav);
  //    playFile("minutes.wav"); // Example recorded voice output should be "minutes"
  /*  Serial.print("The time is: ");  // For debug
    Serial.print(hourWav);  // should print file name
    Serial.print(" hours and ");
    Serial.print(minWav);   // should print file name
    Serial.println(" minutes");*/
}

void playFile(const char *filename)
{
  // Serial.println(); Serial.print("Playing file: "); Serial.print(filename);

  File f;

  if (SDOK) {
    f = SD.open(filename);
  } else if (QSPIOK) {
    f = QSPIFS.open(filename);
  }
  // Start playing the file.  This sketch continues to
  // run while the file plays.
  if (!playWav1.play(f)) {
    Serial.println("Failed to play");
    return;
  }

  // A brief delay for the library read WAV info
  delay(5);

  // Simply wait for the file to finish playing.
  while (playWav1.isPlaying()) {
    //  Serial.print("."); // comment out after debug
    delay(100);
    // uncomment these lines if your audio shield
    // has the optional volume pot soldered
    //float vol = analogRead(15);
    //vol = vol / 1024;
    // sgtl5000_1.volume(vol);
  }
}


// print time to serial
void printTheTime(DateTime theTimeP) {
  Serial.print(theTimeP.year(), DEC);
  Serial.print('/');
  Serial.print(theTimeP.month(), DEC);
  Serial.print('/');
  Serial.print(theTimeP.day(), DEC);
  Serial.print(' ');
  Serial.print(theTimeP.hour(), DEC);
  Serial.print(':');
  Serial.print(theTimeP.minute(), DEC);
  Serial.print(':');
  Serial.print(theTimeP.second(), DEC);
  Serial.println();
}
