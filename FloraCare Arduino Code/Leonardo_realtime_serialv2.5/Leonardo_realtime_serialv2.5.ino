#include <Servo.h>
#include <Wire.h>
#include "RTClib.h"
#include <Time.h>
#include <TimeAlarms.h>

RTC_DS1307 rtc;
Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards
 
int pos = 0;    // variable to store the servo position 

#define SERVO_HEADER "Z"
 
void setup() {

    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
     myservo.attach(9);  // attaches the servo on pin 9 to the servo object 
    Serial.begin(9600);       // for debugging 
    Serial1.begin(115200);   
    Wire.begin();
    rtc.begin();
    if (! rtc.isrunning()) 
       Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled

//  rtc.adjust(DateTime(2015, 3, 17, 23, 33, 0)); 
    setSyncProvider(syncProvider); 

    myservo.write(0);
}

void loop() {

digitalClockDisplay();
 processSyncMessage();

if (timeStatus() == timeSet) {

  } else {
    Serial.println("The time has not been set.  Please run the Time");
    Serial.println("TimeRTCSet example, or DS1307RTC SetTime example.");
    Serial.println();
  }
}

void baskingStart ()
{
  for(pos = 0; pos <= 180; pos += 1) // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(30);
  } 
}
void baskingEnd ()
{
    myservo.write(0);
}
void processSyncMessage() {
  unsigned long servoswitch;
  if(Serial1.find(SERVO_HEADER)) {
    servoswitch = Serial1.parseInt();
       if (servoswitch == 2065199201) {
         baskingStart ();
         Serial.print("Servo is on");
     }
     else if (servoswitch == 2065199200) {
     baskingEnd ();
     }
}
}
void digitalClockDisplay(){
  Serial1.print("T");
  Serial1.print(rtc.now().unixtime());

  Serial1.print("/r");

}

time_t syncProvider()     //this does the same thing as RTC_DS1307::get()
{
  return rtc.now().unixtime();
}
