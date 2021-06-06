/* Macros */
#define MACSTR "%2X:%2X:%2X:%2X:%2X:%2X" //sprintf string for MAC address
#define MAC2STR(a) (a)[5], (a)[4], (a)[3], (a)[2], (a)[1], (a)[0] //expands MAC byte array into constituent bytes

/* Globals */
WiFiServer server(80);
