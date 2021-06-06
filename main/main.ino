#include "myStepper.h"
#include "arduino_secrets.h"
#include "myNtp.h"
#include "myWifi.h"
#include <TimeLib.h>

/* Macros */
#define SYNC_INTERVAL (SECS_PER_MIN * 5)
#define POSITIONS_PER_HOUR 6
#define POSITIONS_TOTAL (POSITIONS_PER_HOUR * 12)

/* Globals */
int currPosition = 0;
int expectedPosition = 0;
unsigned long secondsSinceSync = 0;
unsigned long millisPreviousSync = 0;
unsigned long millisPreviousSerialOutput = 0;
unsigned long millisCurrentSerialOutput = 0;
String readString, newString;


/* Setup
  Run once at power up
*/
void setup() {
  Serial.begin(9600);// Open serial communications and wait for port to open
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Starting...");

  stepper.setSpeed(MOTOR_RPM);//Set motor step speed

  Serial.print("Stepping Init: ");
  Serial.println(INIT_STEP);
  stepper.step(INIT_STEP); //Step a small amount, so reset button can be used for fine alignment

  //check for the WiFi module
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    while (true);// don't continue
  }

  //Check wifi firmware
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  connectWifi();
  Serial.println("Connected to WiFi");

  Udp.begin(localPort);//Open UDP port for NTP connecion
  server.begin();//Launch http web server

  //Output status of connection
  Serial.println(wifiSSIDToString());
  Serial.println(wifiIPToString());
  Serial.println(wifiMACToString());
  Serial.println(wifiRSSIToString());

  //Timelib synchronisation settings
  setSyncProvider(getUTC);//Sets getUTC function from ntp.ino for fetching time
  setSyncInterval(SYNC_INTERVAL);

}

/* loop
  Main continuous loop
*/
void loop() {

  time_t t = now();//Get current time
  secondsSinceSync = (millis() - millisPreviousSync) / 1000;//Calculate time since the last NTP sync in seconds

  if (timeStatus() != timeNotSet)//Determine what position the clock should be based on the time
  {
    expectedPosition = timeToPosition(hourFormat12(t), minute(t));
  }

  int positionDeficit = expectedPosition - currPosition;//Determine deficit - how many steps between current pos and expected pos
  positionDeficit = positionDeficit % POSITIONS_TOTAL;//Correct deficit
  if (positionDeficit < 0) positionDeficit += POSITIONS_TOTAL;//Correct negative deficit
  if (positionDeficit > 0)//If there is a deficit (non zero), step ahead and increment current position
  {
    stepTenMin();
    currPosition += 1;
    currPosition = currPosition % POSITIONS_TOTAL;
  }
  checkTimeSync();//Check if time needs syncing
  listenForClient();//Listen for HTTP web client

  millisCurrentSerialOutput = millis();//Measure time of current debug log

  if (millisCurrentSerialOutput - millisPreviousSerialOutput > 1000)//If more than 1 second since last debug log, output again
  {
    Serial.print("Current position: ");
    Serial.println(currPosition);

    Serial.print("Expected position: ");
    Serial.println(expectedPosition);

    Serial.print("Current time: ");
    Serial.println(timeToString(t));

    Serial.print("Time Status: ");
    int ts = timeStatus();
    if (ts == 0) Serial.println("TimeNotSet");
    if (ts == 1) Serial.println("TimeNeedsSync");
    if (ts == 2) Serial.println("TimeSet");

    Serial.print("Time since NTP Sync: ");
    Serial.println(durationToString(secondsSinceSync));

    Serial.print("UTC hours offset: ");
    Serial.println(utcHoursOffset);
    
    Serial.print("Position deficit: ");
    Serial.println(positionDeficit);
    Serial.println("--------------------");
    millisPreviousSerialOutput = millis();
  }
}