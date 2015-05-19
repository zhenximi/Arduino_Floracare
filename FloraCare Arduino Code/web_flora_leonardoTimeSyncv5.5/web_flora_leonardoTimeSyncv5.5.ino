#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <ShiftLCD.h>
#include <Servo.h>
#include<Time.h>
#include<TimeAlarms.h>

   
Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards
 
int pos = 0;    // variable to store the servo position 
 
 
// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   60

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,31,177); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80
File webFile;               // the web page file on the SD card
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer


//LCD 595 ShiftRe PIN lcd(SER(14),SCK(11),RCK(12))
ShiftLCD lcd(2, 5, 3);

void setup() {
    // disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    //lcd size
    lcd.begin(16, 2);
    myservo.attach(9);  // attaches the servo on pin 9 to the servo object 
    myservo.write(0);  // intialize servo position to o degree
    Serial.begin(115200);   // ----------------------------------   // for debugging
    
    // initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");
    // check for index.htm file
    if (!SD.exists("index.htm")) {
        Serial.println("ERROR - Can't find index.htm file!");
        return;  // can't find index file
    }
    Serial.println("SUCCESS - Found index.htm file.");//--------------------------------------//for debugging
    
    setSyncProvider( requestSync);  //set function to call when sync required
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
}

void loop() {
    //digitalClockDisplay();
    if (Serial.available()) {
      processSyncMessage();
    }

    digitalClockDisplay();      
    Alarm.delay(0); // wait one millisecond between clock display
    
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // limit the size of the stored received HTTP request
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                if (req_index < (REQ_BUF_SZ - 1)) {
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    // remainder of header follows below, depending on if
                    // web page or XML page is requested
                    // Ajax request - send XML file
                    if (StrContains(HTTP_req, "ajax_inputs")) {
                        // send rest of HTTP header
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
                        SetAlarm();
//                        // send XML file containing input states
//                        XML_response(client);
                    }
                    else {  // web page request
                        // send rest of HTTP header
                        client.println("Content-Type: text/html");
                        client.println("Connection: keep-alive");
                        client.println();
                        // send web page
                        webFile = SD.open("index.htm");        // open web page file
                        if (webFile) {
                            while(webFile.available()) {
                                client.write(webFile.read()); // send web page to client
                            }
                            webFile.close();
                        }
                    }
                    // display received HTTP request on serial port
                    Serial.print(HTTP_req);
                    Serial.print('\n');
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}


void SetAlarm (void)
{
  int w;
  int e;
  int h;
  int m;
  int A1; 
  int A2; 

if (StrContains(HTTP_req, "&timea")) {
    lcd.setCursor(0,1);
    lcd.print("WA:");
    
   // Serial.print(HTTP_req[23]);
   if (HTTP_req[22]!='0') {
     h = 10 * int(HTTP_req[22]-'0') + int(HTTP_req[23]-'0');
   }
   else {
     h = int(HTTP_req[23]-'0');
   }
   if (HTTP_req[25]!='0'){
        m = 10 * int(HTTP_req[25]-'0') + int(HTTP_req[26]-'0');
   }
   else {
       m = int(HTTP_req[26]-'0');
   }
   //lcd.print(h);
    w = int(HTTP_req[28]-'0');
    e = int(HTTP_req[30]-'0');
    lcd.print(HTTP_req[22]);
    lcd.print(HTTP_req[23]);
    lcd.print(HTTP_req[24]);
    lcd.print(HTTP_req[25]);
    lcd.print(HTTP_req[26]);
    //Alarm.timerRepeat(5, Repeats);
    switch (w) {
    case 1:
      switch (e) {
        case 1:
             A1 = Alarm.alarmRepeat(dowSunday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
          A1 = Alarm.alarmRepeat(dowSunday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;
    case 2:
      switch (e) {
        case 1:
             A1 = Alarm.alarmRepeat(dowMonday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
          A1 = Alarm.alarmRepeat(dowMonday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;
    case 3:
     switch (e) {
        case 1:
             A1 = Alarm.alarmRepeat(dowTuesday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
            A1 = Alarm.alarmRepeat(dowTuesday,h,m,0, baskingAlarm);  // h:m every day
          break;
     }
      break;
    case 4:
      switch (e) {
        case 1:
             A1 = Alarm.alarmRepeat(dowWednesday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
             A1 = Alarm.alarmRepeat(dowWednesday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;
    case 5:
      switch (e) {
        case 1:
            A1 = Alarm.alarmRepeat(dowThursday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
            A1 = Alarm.alarmRepeat(dowThursday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;
    case 6:
      switch (e) {
        case 1:
             A1 = Alarm.alarmRepeat(dowFriday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
          A1 = Alarm.alarmRepeat(dowFriday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;
    case 7:
      switch (e) {
        case 1:
             A1 = Alarm.alarmRepeat(dowSaturday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
             A1 = Alarm.alarmRepeat(dowSaturday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;

  }
  if (HTTP_req[21]=='0') {
    Alarm.disable(A1); 
  }
  }
if (StrContains(HTTP_req, "&timeb")) {
    
  lcd.setCursor(8,1);
    lcd.print("BA:");
       if (HTTP_req[22]!='0') {
     h = 10 * int(HTTP_req[22]-'0') + int(HTTP_req[23]-'0');
   }
   else {
     h = int(HTTP_req[23]-'0');
   }
   if (HTTP_req[25]!='0'){
        m = 10 * int(HTTP_req[25]-'0') + int(HTTP_req[26]-'0');
   }
   else {
   m = int(HTTP_req[26]-'0');
   }
   //lcd.print(h);
    w = int(HTTP_req[28]-'0');
    e = int(HTTP_req[30]-'0');
    lcd.print(HTTP_req[22]);
    lcd.print(HTTP_req[23]);
    lcd.print(HTTP_req[24]);
    lcd.print(HTTP_req[25]);
    lcd.print(HTTP_req[26]);
   //lcd.print(h);
    
    //Alarm.timerRepeat(5, Repeats);
    switch (w) {
    case 1:
      switch (e) {
        case 1:
             A2 = Alarm.alarmRepeat(dowSunday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
          A2 = Alarm.alarmRepeat(dowSunday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;
    case 2:
      switch (e) {
        case 1:
             A2 = Alarm.alarmRepeat(dowMonday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
          A2 = Alarm.alarmRepeat(dowMonday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;
    case 3:
     switch (e) {
        case 1:
             A2 = Alarm.alarmRepeat(dowTuesday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
          A2 = Alarm.alarmRepeat(dowTuesday,h,m,0, baskingAlarm);  // h:m every day
          break;
     }
      break;
    case 4:
      switch (e) {
        case 1:
             A2 = Alarm.alarmRepeat(dowWednesday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
          A2 = Alarm.alarmRepeat(dowWednesday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;
    case 5:
      switch (e) {
        case 1:
             A2 = Alarm.alarmRepeat(dowThursday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
          A2 = Alarm.alarmRepeat(dowThursday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;
    case 6:
      switch (e) {
        case 1:
             A2 = Alarm.alarmRepeat(dowFriday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
          A2 = Alarm.alarmRepeat(dowFriday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;
    case 7:
      switch (e) {
        case 1:
             A2 = Alarm.alarmRepeat(dowSaturday,h,m,0, waterAlarm);  // h:m every day
          break;
        case 2:
          A2 = Alarm.alarmRepeat(dowSaturday,h,m,0, baskingAlarm);  // h:m every day
          break;
      }
      break;

  }
  if (HTTP_req[21]=='0') {
    Alarm.disable(A2); 
  }
}
  }



// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);
    
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }

    return 0;
}

void digitalClockDisplay()
{
  // digital clock display of the time
  lcd.setCursor(0,0);
  lcd.print(hour());
  printDigits(minute());
  printDigits(second());
  //Serial.println(); 
}

void printDigits(int digits)
{
  lcd.print(":");
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits);
}



void waterAlarm(){
 // lcd.println(" "); 
  lcd.setCursor(0,1);
  lcd.print("I need water!"); 
  wateringStart();
   Alarm.timerOnce(30, wateringTime);
  //lcd.blink();
}

void baskingAlarm(){
  lcd.setCursor(0,1);
  lcd.print("I need sunshine!"); 
  baskingStart();
   Alarm.timerOnce(10, baskingTime);
}
  
void wateringTime(){
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Water!"); 
  wateringEnd();  
}
void wateringStart ()
{
  for(pos = 0; pos <= 150; pos += 1) // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(20);
  } 
}
void wateringEnd ()
{
    myservo.write(0);
}

void baskingTime(){
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Bask more~"); 
  baskingEnd();  
}
void baskingStart ()
{
  Serial.print("Z2065199201");
  Serial.print("\r");
} 
void baskingEnd ()
{
  Serial.print("Z2065199200");
  Serial.print("\r");
}

void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
       setTime(pctime); // Sync Arduino clock to the time received on the serial port
     }
  }
}



time_t requestSync()
{
  Serial.write(TIME_REQUEST);  
  return 0; // the time will be sent later in response to serial mesg
}
