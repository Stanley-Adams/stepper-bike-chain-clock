#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

/* Macros */
#define SEVENTY_YEARS 2208988800UL // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
/* Globals */
int utcHoursOffset = 2;
int status = WL_IDLE_STATUS;
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key index number (needed only for WEP)

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP timestamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

WiFiUDP Udp;// A UDP instance to let us send and receive packets over UDP
