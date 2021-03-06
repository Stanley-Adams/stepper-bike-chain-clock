/* getUTC
  returns time_t of time fetched from NTP server
*/
time_t getUTC()
{
  time_t t;

  Serial.println("--------------------");
  Serial.println("Synchronising time with NTP server...");
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    Serial.println("NTP packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // now convert NTP time into everyday time:
    t = secsSince1900 - SEVENTY_YEARS + (utcHoursOffset * SECS_PER_HOUR);
    Serial.print("Time fetched from server: ");
    Serial.println(timeToString(t));
    millisPreviousSync = millis();
    return t;
  }
}

/* timeToString
  Converts time_t variable to string in 12 hour format HH:MM:SS
*/
String timeToString(time_t t)
{
  char output[100];
  snprintf(output,100,"%02d:%02d:%02d",hourFormat12(t),minute(t),second(t));
  return output;
}


/* durationToString
  Converts unsigned long of count of seconds to duration string in format HH:MM:SS
*/
String durationToString(unsigned long t)
{
  char output[100];
  snprintf(output,100,"%02d:%02d:%02d",(int)numberOfHours(t),(int)numberOfMinutes(t),(int)numberOfSeconds(t));
  return output;
}


/* sendNTPpacket
  Send an NTP request to the time server at the given address
*/
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}

/* checkTimeSync
  Maintain wifi connection and time sync
*/
void checkTimeSync()
{
  connectWifi();//Re-establish WiFi if needed
  if (timeStatus() == timeNotSet)
  {
    setSyncProvider(getUTC);//Manual resync if needed
  }
}
