/*
  Web client
 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield. 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 created 18 Dec 2009
 by David A. Mellis
 */

#include <SPI.h>
#include <Ethernet.h>
#include <String.h>
#include "datamodel.h"

// Max String length may have to be adjusted depending on data to be extracted
#define MAX_STRING_LEN  250
#define MAX_STRING_ROWS 50

// Enter a MAC address and IP address for your controller below.
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//byte ip[] = { 192,168,1,177 };
IPAddress ip(192,168,1,177);
IPAddress myDns(8,8,8,8);
byte gateway[] = { 192,168,1,176 }; // my macbook, sharing its internet connection
byte subnet[] = { 255,255,255,0 };
//byte nextmuni[] = { 64,124,123,57 }; // nextmuni API, IP address resolved by webservices.nextbus.com
byte nextmuni[] = { 192,168,1,176 }; // nextmuni API simulated by local computer MAMP
//char nextmuni[] = "webservices.nextbus.com";

char tagStr[MAX_STRING_LEN] = "";
char* dataStr = { "" };
char tmpStr[MAX_STRING_LEN] = "";

String tempRow;

char endTag[3] = {'/', '>', '\0'};
char LF=10;
char CR=13;
char LT=62; // > char
int len;
int rowCounter = 0;

const int request_interval = 10000;  // delay between requests, 60 seconds
const int refresh_interval = 1000;  // delay between screen refreshes, 1 second
long last_refreshed = 0;            // last time text was written to display

prediction N, F, J, K, L, M, six, twentytwo, seventyone;
prediction* F_ptr = &F; prediction* J_ptr = &J; prediction* K_ptr = &K;
prediction* L_ptr = &L; prediction* M_ptr = &M; prediction* N_ptr = &N;
prediction* six_ptr = &six; prediction* twentytwo_ptr = &twentytwo; prediction* seventyone_ptr = &seventyone;

int num_predictions = 0;
int attempt_connect = 1;

EthernetClient client;

void setup() {
  Ethernet.begin(mac, ip, myDns, gateway, subnet); // start the Ethernet connection:
  Serial.begin(9600);
  delay(800); // give the Ethernet shield a second to initialize
}

void loop()
{
// find time of last update for the route, if needed, re-connect to the URL for the route, extraction some number of predictions
// store predictions and timestamp of update into memory, search for any special messages, move to the next route

  if (attempt_connect) {
    connect_to_update('N');
    delay(400);
    attempt_connect = 0;
//    String URL = URL_constructor(4448,'N');
  }
  
  if (client.connected()) {
    if (client.available()) {
       serialEvent(N_ptr);
    }
//    else client.stop();
  } 
  
  if (!client.connected()) {
    client.flush();
    client.stop();
//    Serial.println("Client stopped");
}
  
  if (millis() - N_ptr->last_attempt > request_interval) {
    attempt_connect = 1;
    Serial.println("trigger refresh");
  }
  else if (millis() - last_refreshed > refresh_interval) {
    // reprint previously recorded times
    Serial.println("");
    Serial.println(N_ptr->route + " " + N_ptr->route_direction + " ");
    for (int i=0; i<3;i++) {
      for (int j=0;j<2;j++) {
         Serial.print(N_ptr->prediction_time[i][j]);
      }
      if (i<2) Serial.print(", ");
    }
    last_refreshed = millis();
  }
}

void connect_to_update(char _route) {
  Serial.println("");
  Serial.println("Connecting...");
    // if you get a connection, report back via serial:
  if (client.connect(nextmuni, 80)) {
    Serial.println("Connected");

//    client.println("GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=4448 HTTP/1.0");

// DNS-based request:
    client.println("GET /N-munixml.txt HTTP/1.0");
//    client.println("GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=4448 HTTP/1.0");
//    client.println("Host: webservices.nextbus.com");
//    client.println("User-Agent: arduino");
//    client.println("Accept */*");
//    client.println("Connection: close");
    client.println();
  }  else {
    Serial.println("Connection failed.");
  }
  
  // record the last time the connection was attempted
  N_ptr->last_attempt = millis();
  Serial.println("last attempt: " + N_ptr->last_attempt);
}


////////////////////////////
// Process each char from web
void serialEvent(prediction* _route) {
   char inChar = client.read();
   Serial.print(inChar);
   if ( (inChar == 10) /* || (inChar == CR) /* || (inChar == LT) */) {
     tempRow = tmpStr;
//     Serial.println(tempRow);
     if (tempRow.startsWith("<predictions", 0)) {
       if (_route->route.length() < 1) {
         int RouteTitleIndexStart = tempRow.indexOf("routeTitle"); //This gives the name of the MUNI line
         int RouteTitleIndexEnd = tempRow.indexOf("routeTag"); //routeTag is the next tag after RouteTitle   
         _route->route = tempRow.substring(RouteTitleIndexStart + 12, RouteTitleIndexEnd - 2);
       }
     }
     
    if (tempRow.startsWith("  <prediction", 0)) {
      if (num_predictions < 3) {
         int predictionValueIndexStart = tempRow.indexOf("minutes");
         int predictionValueIndexEnd = tempRow.indexOf("isDeparture");
         String PredictionValue = tempRow.substring(predictionValueIndexStart + 9, predictionValueIndexEnd - 2);      
         PredictionValue.toCharArray(_route->prediction_time[num_predictions], 3);
         num_predictions++;
      }
     }
     
     if (tempRow.startsWith("  <direction", 0)) {
       if (_route->route_direction.length() < 1) {
         int directionTitleIndexStart = tempRow.indexOf("title");
         int directionTitleIndexEnd = tempRow.indexOf("to");
         _route->route_direction = tempRow.substring(directionTitleIndexStart + 7, directionTitleIndexEnd - 1);
       }
     }
     
     // Clear all Strings
     clearStr(tmpStr);
   }
     
  else  {
   addChar(inChar, tmpStr);
  //     Serial.print(tmpStr);
  }

}

String URL_constructor(int _stop_ID, char _route) {
  String base_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=";
  String suffix_URL = " HTTP/1.0";
  String _URL = base_URL + _route + "&s=" + _stop_ID + suffix_URL;
  return _URL;
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

// sample URL: http://webservices.nextbus.com/service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=4448 //

// list of routes for a line: http://webservices.nextbus.com/service/publicXMLFeed?command=routeConfig&a=sf-muni&r=J //


// Church st station outbound: stop ID = 6998
// Church st station inbound: stop ID = 5726

// N outbound stop at Church and Duboce: stop ID = 4447
// N inbound stop at Church and Duboce: stop ID = 4448

// J outbound stop at Church and 14th: stop ID = 7316
// J inbound stop at Church and Duboce: stop ID = 4006

// F outbound stop at Church and 14th: stop ID = 5661
// F inbound stop at Church and Market: stop ID = 5662

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
<body copyright="All data copyright San Francisco Muni 2012.">
<predictions agencyTitle="San Francisco Muni" routeTitle="N-Judah" routeTag="N" stopTitle="Duboce Ave &amp; Church St" stopTag="4448">
  <direction title="Inbound to Caltrain via Downtown">
  <prediction epochTime="1343886965750" seconds="284" minutes="4" isDeparture="false" dirTag="N__IBJU4K" vehicle="1544" block="9725" tripTag="5148882" />
  <prediction epochTime="1343888237468" seconds="1556" minutes="25" isDeparture="false" affectedByLayover="true" dirTag="N__IB1" vehicle="1513" block="9715" tripTag="5148763" />
  <prediction epochTime="1343888638996" seconds="1957" minutes="32" isDeparture="false" affectedByLayover="true" dirTag="N__IB1" ve  hicle="1511" block="9704" tripTag="5148764" />
  <prediction epochTime="1343889287632" seconds="2606" minutes="43" isDeparture="false" affectedByLayover="true" dirTag="N__IB1" vehicle="1539" block="9723" tripTag="5148762" />
  <prediction epochTime="1343889838996" seconds="3157" minutes="52" isDeparture="false" affectedByLayover="true" dirTag="N__IB1" vehicle="1538" block="9706" tripTag="5148765" />
  </direction>
<message text="PROOF OF PAYMENT&#10;is required when&#10;on a Muni vehicle&#10;or in a station."/>
<message text="No Elevator at&#10;Embarcadero Sta."/>
<message text="sfmta.com or 3 1 1&#10;for Muni info"/>
</predictions>
</body>
*/
 

