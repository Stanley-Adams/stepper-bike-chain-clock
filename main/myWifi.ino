/* connectWifi
  If Wi-Fi not connected, then try to connect
*/
void connectWifi()
{
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
}

/* listenForClient
  Waits for http client, builds http response and handles GET requests from client
*/
void listenForClient()
{
  time_t t;

  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("--------------------");
    Serial.println("New web client connected");
    // an HTTP request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        //read char by char HTTP request
        if (readString.length() < 100) {

          //store characters to string
          readString += c;
          //Serial.print(c);
        }

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<HTML>");
          client.println("<HEAD>");
          client.println("<TITLE>Arduino Clock</TITLE>");
          client.println("</HEAD>");
          client.println("<BODY>");

          // Output clock parameters to HTML
          client.print("Current position: ");
          client.print(currPosition);
          client.println("<br />");
          client.print("Expected position: ");
          client.print(expectedPosition);
          client.println("<br />");
          client.print("TimeZone offset from UTC: ");
          client.print(utcHoursOffset);
          client.println("<br />");
          client.print("Time Since NTP Sync: ");
          client.print(durationToString(secondsSinceSync));
          client.println("<br />");
          client.print("Time Status: ");
          int ts = timeStatus();
          if (ts == 0) client.print("TimeNotSet");
          if (ts == 1) client.print("TimeNeedsSync");
          if (ts == 2) client.print("TimeSet");
          client.println("<br />");
          client.print(wifiSSIDToString());
          client.println("<br />");
          client.print(wifiIPToString());
          client.println("<br />");
          client.print(wifiMACToString());
          client.println("<br />");
          client.print(wifiRSSIToString());
          client.println("<br />");

          t = now();
          client.print("Current time: ");
          client.print(timeToString(t));
          client.println("<br />");

          //Output HTML form so client can send get requests for processing
          client.println("<H1>Calibrate current position</H1>");
          client.println("<FORM ACTION='/' method=get >"); //uses IP/port of web page
          client.println("<label for='fhours'>Hours:</label>");
          client.println("<INPUT TYPE=TEXT ID='fhours' NAME='fhours' VALUE='' SIZE='25' MAXLENGTH='2'><br>");//Box to enter current hour pos
          client.println("<label for='fmins'>Mins:</label>");
          client.println("<INPUT TYPE=TEXT ID='fmins' NAME='fmins' VALUE='' SIZE='25' MAXLENGTH='2'><br>");//Box to enter current minute pos
          client.println("<label for='foffset'>TZ offset (hours):</label>");
          client.println("<INPUT TYPE=TEXT ID='foffset' NAME='foffset' VALUE='' SIZE='25' MAXLENGTH='2'><br>");//Box to enter current TZ offset in hours
          client.println("<INPUT TYPE=SUBMIT NAME='submit' VALUE='Submit'>");
          client.println("<INPUT TYPE=SUBMIT NAME='submitandsync' VALUE='Submit and Sync'>");//Button to trigger manual ntp sync
          client.println("</FORM>");
          client.println("");
          //javascript clears parameters from URL so client refresh doesn't resubmit old get request
          client.println("<script> window.history.replaceState(null, null, window.location.pathname);</script>");
          client.println("</BODY>");
          ("</HTML>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();

    //Process get request
    if (readString.length() > 0) {
      int hrs, mins, offset;
      bool timeSubmitted = false;
      
      Serial.print("Full input string is: ");
      Serial.println(readString); //prints string to serial port out

      //Process hours get request
      int pos1 = readString.indexOf("fhours=");
      int pos2 = readString.indexOf('&', pos1);
      newString = readString.substring(pos1 + 7, pos2);
      if (pos1 != -1 && (pos1 + 7 != pos2)) //If fhours param is found and has a value
      {
        Serial.print("Hour input string is: ");
        Serial.println(newString);

        hrs = newString.toInt();
        Serial.print("Hours converted to integer is: ");
        Serial.println(hrs);
        timeSubmitted = true;
      } else {
        Serial.println("Hour value not found in input string");
        timeSubmitted = false;
      }

      newString = ""; //clears variable for new input
      
      //Process minutes get request
      pos1 = readString.indexOf("fmins=");
      pos2 = readString.indexOf('&', pos1);
      newString = readString.substring(pos1 + 6, pos2);
      if (timeSubmitted && pos1 != -1 && (pos1 + 6 != pos2)) //If fhours submited, fmins param is found and has a value
      {
        Serial.print("Mins input string is: ");
        Serial.println(newString);
        mins = newString.toInt();
        Serial.print("Mins converted to integer is: ");
        Serial.println(mins);
        timeSubmitted = true;
      } else {
        Serial.println("Mins value not found in input string");
        timeSubmitted = false;
      }

      newString = ""; //clears variable for new input

      //Process tz offset get request
      pos1 = readString.indexOf("foffset=");
      pos2 = readString.indexOf('&', pos1);
      newString = readString.substring(pos1 + 8, pos2);
      if (pos1 != -1 && (pos1 + 8 != pos2)) //If foffset param is found and has a value
      {
        Serial.print("Offset input string is: ");
        Serial.println(newString);
        offset = newString.toInt();
        Serial.print("Offset converted to integer is: ");
        Serial.println(offset);
        utcHoursOffset = offset;
      } else {
        Serial.println("Offset value not found in input string");
      }

      //Process resync get request
      pos1 = readString.indexOf("submitandsync=");
      pos2 = readString.indexOf('Submit+and+Sync', pos1);
      if (pos1 != -1 && pos2 != -1)//If submitandsync isn't found or value is blank
      {
        Serial.println("Triggering manual resync of NTP time");
        setSyncProvider(getUTC);
      }

      readString = ""; //clears variable for new input
      newString = ""; //clears variable for new input

      //If hours and minute values both sent
      if (timeSubmitted == true)
      {
        if (hrs >= 0 && hrs <= 23 && mins >= 0 && mins <= 59)
        {
          currPosition = timeToPosition(hrs, mins);//Set current position based on time sent
        }
        else Serial.println("Invalid time submitted");
      }

    }
    Serial.println("client disconnected");
    Serial.println("--------------------");
  }
}

/*
wifiSSIDToString
Convert SSID value to a string to be outputted
 */
String wifiSSIDToString()
{
  char outBuf[100];

  snprintf(outBuf,100,"SSID: %s",WiFi.SSID());
  return outBuf;
}
/*
wifiIPToString
Convert IP value to a string to be outputted
 */
String wifiIPToString()
{
  char outBuf[30];
  char ipBuf[16];

  IPAddress ip = WiFi.localIP();
  sprintf(ipBuf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  
  snprintf(outBuf,30,"IP Address: %s",ipBuf);
  return outBuf;
}

/*
wifiMACToString
Convert MAC address value to a string to be outputted
 */
String wifiMACToString()
{
  char outBuf[31];
  byte mac[6]; 
  WiFi.macAddress(mac);
  
  snprintf(outBuf,31,"MAC Address: " MACSTR,MAC2STR(mac));
  return outBuf;
}

/*
wifiRSSIToString
Convert RSSI value to a string to be outputted
 */
String wifiRSSIToString()
{
  char outBuf[32];

  snprintf(outBuf,32,"Signal Strength (RSSI): %d dBm",WiFi.RSSI());
  return outBuf;
}
