/*
  Web client
 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield. 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 created 18 Dec 2009
 by David A. Mellis
 */
//////////////////////////////////////////////
// Get XML formatted data from the web.
// 1/6/08 Bob S. - Created
//
//  Assumptions: single XML line looks like:
//    <tag>data</tag> or <tag>data
//
// Get current weather observation for Raleigh from weather.gov in XML format
//
//////////////////////////////////////////////

#include <SPI.h>
#include <Ethernet.h>
#include <String.h>
//#include <Time.h>


// Max String length may have to be adjusted depending on data to be extracted
#define MAX_STRING_LEN  250
#define MAX_STRING_ROWS 50

// Church st station outbound: stop ID = 6998
// Church st station inbound: stop ID = 5726

// N outbound stop at Church and Duboce: stop ID = 4447
// N inbound stop at Church and Duboce: stop ID = 4448

// J outbound stop at Church and 14th: stop ID = 7316
// J inbound stop at Church and Duboce: stop ID = 4006

// F outbound stop at Church and 14th: stop ID = 5661
// F inbound stop at Church and Market: stop ID = 5662

/* ////// table to hold prediction times \\\\\\\\

_________________|_______|_________|_____________|______________|______________|______________|________________
stop_description | route | stop_ID | last_update | prediction_1 | prediction_2 | prediction_3 | prediction_URL

struct to hold it:

struct prediction {
  String route;
  char direction[3];
  String stop_description;
  
  int in_stop_ID;
  time_t in_last_update;
  int in_prediction_1;
  int in_prediction_2;
  int in_prediction_3;
  String in_prediction_URL;
  
  int out_stop_ID;
  time_t out_last_update;
  int out_prediction_1;
  int out_prediction_2;
  int out_prediction_3;
  String out_prediction_URL;
}

struct prediciton F, J, K, L, M, N, six, twentytwo, seventyone;

   ////// table to hold prediction times \\\\\\\\ */

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,1,177 };
byte server[] = { 74,125,39,104 }; // NextMUNI
byte gateway[] = { 192,168,1,176 }; // my macbook, sharing its internet connection
//byte nextmuni[] = { 64,124,123,57 }; // nextmuni API
byte nextmuni[] = { 192,168,1,176 }; // nextmuni API

char tagStr[MAX_STRING_LEN] = "";
//char dataStr[][MAX_STRING_LEN] = { "" };
char* dataStr = { "" };
char tmpStr[MAX_STRING_LEN] = "";

String tempRow;

//char endTag[3] = {'<', '/', '\0'};
char endTag[3] = {'/', '>', '\0'};
char LF=10;
char CR=13;
char LT=62; // > char
int len;
int rowCounter = 0;

typedef struct {
  String route;
//  char direction[3];
  String route_direction;
  String stop_description;
  
  int in_stop_ID;
//  time_t in_last_update;
  char in_prediction_1[3];
  char in_prediction_2[3];
  char in_prediction_3[3];
  String in_prediction_URL;
  
  int out_stop_ID;
//  time_t out_last_update;
  char out_prediction_1[3];
  char out_prediction_2[3];
  char out_prediction_3[3];
  String out_prediction_URL;
} prediction;

prediction F, J, K, L, M, N, six, twentytwo, seventyone;

// Flags to differentiate XML tags from document elements (ie. data)
boolean tagFlag = false;
boolean dataFlag = false;

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
//EthernetClient client(nextmuni, 80);
EthernetClient client;

void setup() {
  // start the Ethernet connection:
  Ethernet.begin(mac, ip, gateway);
  // start the serial library:
  Serial.begin(9600);
  // give the Ethernet shield a second to initialize:
  delay(800);
  connect_to_update();
}

void loop()
{

// find time of last update for the route
// if enough time has elapsed, process the route
// connect to the URL for the route
// extraction some number of predictions
// store predictions and timestamp of update into memory
// search for any special messages

// move to the next route
//  Serial.println("OK, ready");
//  get_update();

  if (client.available()) {
    serialEvent();
  } 

  if (!client.connected()) {
    client.stop();
    for(;;);
  }
}

void connect_to_update() {
  Serial.println("");
  Serial.println("Connecting...");
    // if you get a connection, report back via serial:
  if (client.connect(nextmuni, 80)) {
    Serial.println("Connected");
    // Make a HTTP request:
//    client.println("GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=4448 HTTP/1.0");
    client.println("GET /munixml.txt HTTP/1.0");
    client.println();
  }  else {
    Serial.println("connection failed");
  }
}

////////////////////////////
// Process each char from web
void serialEvent() {
   char inChar = client.read();

   if ( (inChar == 10) /* || (inChar == CR) /* || (inChar == LT) */) {
//     dataStr[rowCounter] = *tmpStr;
     tempRow = tmpStr;
     Serial.println(tempRow);
     if (tempRow.startsWith("<predictions", 0)) {
       Serial.println("Predictions header"); 
       int RouteTitleIndexStart = tempRow.indexOf("routeTitle"); //This gives the name of the MUNI line
       Serial.println(tempRow); /*
       
       int RouteTitleIndexEnd = tempRow.indexOf("routeTag"); //routeTag is the next tag after RouteTitle   
       String RouteTitle = tempRow.substring(RouteTitleIndexStart + 12, RouteTitleIndexEnd - 2);
       N.route = RouteTitle;
//       Serial.print("Route Title: "); Serial.println(N.route); */
     }
     
    if (tempRow.startsWith("  <prediction", 0)) {
//       Serial.println("Prediction: ");
       int predictionValueIndexStart = tempRow.indexOf("minutes");
       int predictionValueIndexEnd = tempRow.indexOf("isDeparture");
       String PredictionValue = tempRow.substring(predictionValueIndexStart + 9, predictionValueIndexEnd - 2);
       Serial.println("Prediction: ");
       Serial.println(PredictionValue);
       Serial.println(tempRow); /*
       
       
       
       Serial.print("Prediction: "); Serial.println(PredictionValue);     */ 
     }
     
     if (tempRow.startsWith("  <direction", 0)) {
       Serial.println("Direction: "); 
       Serial.println(tempRow); /*
       int directionTitleIndexStart = tempRow.indexOf("title");
       String route_direction = tempRow.substring(directionTitleIndexStart + 7, directionTitleIndexStart + 15);
       N.route_direction = route_direction.substring(0,2); */
     }
     
     // Clear all Strings
     clearStr(tmpStr);
   }
     
  else  {
   addChar(inChar, tmpStr);
  //     Serial.print(tmpStr);
  }

}

// Function to clear a String
void clearStr (char* str) {
   int len = strlen(str);
   for (int c = 0; c < len; c++) {
      str[c] = 0;
   }
}

//Function to add a char to a String and check its length
void addChar (char ch, char* str) {
      str[strlen(str)] = ch;
}

/* ////// example response \\\\\\\
HTTP/1.1 200 OK
Date: Tue, 18 Oct 2011 03:58:18 GMT
Server: Apache/2.2.4 (Unix) mod_ssl/2.2.4 OpenSSL/0.9.7m DAV/2 mod_jk/1.2.30
Access-Control-Allow-Origin: *
Content-Length: 1818
Vary: Accept-Encoding
Connection: close
Content-Type: text/xml

<?xml version="1.0" encoding="utf-8" ?> 
<body copyright="All data copyright San Francisco Muni 2011.">
<predictions agencyTitle="San Francisco Muni" routeTitle="N-Judah" routeTag="N" stopTitle="Duboce Ave &amp; Church St" stopTag="4448">
  <direction title="Inbound to Downtown/Caltrain">
  <prediction epochTime="1318910429586" seconds="130" minutes="2" isDeparture="false" dirTag="N__IB1" vehicle="1406" block="9715" tripTag="4621621" />
  <prediction epochTime="1318911298770" seconds="1000" minutes="16" isDeparture="false" dirTag="N__IB1" vehicle="1451" block="9704" tripTag="4621622" />
  <prediction epochTime="1318911948951" seconds="1650" minutes="27" isDeparture="false" dirTag="N__IB1" vehicle="1416" block="9706" tripTag="4621623" />
  <prediction epochTime="1318912769827" seconds="2471" minutes="41" isDeparture="false" affectedByLayover="true" dirTag="N__IB1" vehicle="1504" block="9707" tripTag="4621624" />
  <prediction epochTime="1318914509827" seconds="4211" minutes="70" isDeparture="false" affectedByLayover="true" dirTag="N__IB1" vehicle="1461" block="9711" tripTag="4621626" />
  </direction>
  <direction title="Inbound to Downtown/Caltrain (M-F)">
  <prediction epochTime="1318913452536" seconds="3153" minutes="52" isDeparture="false" affectedByLayover="true" dirTag="N__IBJU4K" vehicle="1489" block="9724" tripTag="4621625" />
  <prediction epochTime="1318913932536" seconds="3633" minutes="60" isDeparture="false" affectedByLayover="true" dirTag="N__IBJU4K" vehicle="1525" block="9709" tripTag="4621748" />
  </direction>
<message text="www.sfmta.com or&#10;311 for Muni info."/>
<message text="Effective 10/15&#10;N Judah will go&#10;to Caltrain&#10;on weekends"/>
<message text="PROOF OF PAYMENT&#10;is required when&#10;on a Muni vehicle&#10;or in a station."/>
</predictions>
</body>
*/
 

