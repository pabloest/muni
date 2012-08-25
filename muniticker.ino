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
//#include <String.h>
#include "datamodel.h"

// Max String length may have to be adjusted depending on data to be extracted
#define MAX_STRING_LEN  220
#define MAX_STRING_ROWS 50

// Enter a MAC address and IP address for your controller below.
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//byte ip[] = { 192,168,1,177 };
IPAddress ip(192,168,1,177);
IPAddress myDns(8,8,8,8);
byte gateway[] = { 192,168,1,176 }; // my macbook, sharing its internet connection
byte subnet[] = { 255,255,255,0 };
//byte nextmuni[] = { 64,124,123,57 }; // nextmuni API, IP address resolved by webservices.nextbus.com
//byte nextmuni[] = { 192,168,1,176 }; // nextmuni API simulated by local computer MAMP
char nextmuni[] = "webservices.nextbus.com";

char tagStr[MAX_STRING_LEN] = "";
char* dataStr = { "" };
char tmpStr[MAX_STRING_LEN] = "";
char* tmpStr_ptr = &tmpStr[0];
char strarr[MAX_STRING_LEN][MAX_STRING_ROWS] = { "" };
char* strarr_ptr = &strarr[0][0];

//String tempRow;
//String* tempRow_ptr = &tempRow;

char endTag[3] = {'/', '>', '\0'};
char LF=10;
char CR=13;
char LT=62; // > char
int len;
int rowCounter = 0;

const int request_interval = 10000;  // delay between requests, 10 seconds
const int refresh_interval = 1000;  // delay between screen refreshes, 1 second
//long last_refreshed = 0;            // last time text was written to display

prediction N, F, J, K, L, M, six, twentytwo, seventyone;
prediction* F_ptr = &F; prediction* J_ptr = &J; prediction* K_ptr = &K;
prediction* L_ptr = &L; prediction* M_ptr = &M; prediction* N_ptr = &N;
prediction* six_ptr = &six; prediction* twentytwo_ptr = &twentytwo; prediction* seventyone_ptr = &seventyone;

int num_predictions = 0;
//int attempt_connect = 1;

EthernetClient client;

void setup() {
  Ethernet.begin(mac, ip, myDns, gateway, subnet); // start the Ethernet connection:
  Serial.begin(9600);
  delay(800); // give the Ethernet shield a second to initialize
  N_ptr->attempt_connect = 1;
  N_ptr->last_refreshed = 0;
  J_ptr->attempt_connect = 1;
  J_ptr->last_refreshed = 0;
  
}

void loop()
{
// find time of last update for the route, if needed, re-connect to the URL for the route, extraction some number of predictions
// store predictions and timestamp of update into memory, search for any special messages, move to the next route

// check N route for last update
// if N needs update, do it
// check if J for last update
// if J needs update, do it

  if (millis() - N_ptr->last_attempt > request_interval) {
    String N_URL = URL_constructor(4448,'N'); // N-Judah, inbound from Church and Duboce
//    Serial.print("N updating "); Serial.println(N_URL);
    N_ptr->attempt_connect = 1;
    connect_to_update(N_ptr, N_URL);
    delay(400);
    N_ptr->attempt_connect = 0;
  }
  else if (millis() - N_ptr->last_refreshed > refresh_interval) {
    // reprint previously recorded times
    if (strlen(N_ptr->route) > 1) {
      Serial.println("");
      Serial.print(N_ptr->route);Serial.print(" ");Serial.print(N_ptr->route_direction);Serial.println(": ");
      for (int i=0; i<3;i++) {
        for (int j=0;j<2;j++) {
           Serial.print(N_ptr->prediction_time[i][j]);
        }
        if (i<2) Serial.print(", ");
      }
      N_ptr->last_refreshed = millis();
    }
  }
  
  if (!client.connected()) {
    client.flush();
    client.stop();
//    Serial.println("Client stopped");
  }
//  
//  delay(1000);
  
  if (millis() - J_ptr->last_attempt > request_interval) {
    String J_URL = URL_constructor(4006,'J'); // J-Church, inbound from Church and Duboce
    Serial.print("Updating J-Church");
    J_ptr->attempt_connect = 1;
    connect_to_update(J_ptr, J_URL);
    delay(400);
    J_ptr->attempt_connect = 0;
    
  }
  else if (millis() - J_ptr->last_refreshed > refresh_interval) {
    // reprint previously recorded times
   if (strlen(J_ptr->route) > 1) {
      Serial.println("");
    
      Serial.print(J_ptr->route);Serial.print(" ");Serial.print(J_ptr->route_direction);Serial.println(": ");
      for (int i=0; i<3;i++) {
        for (int j=0;j<2;j++) {
           Serial.print(J_ptr->prediction_time[i][j]);
        }
        if (i<2) Serial.print(", ");
      }
      J_ptr->last_refreshed = millis();
    }
  }
  
  if (!client.connected()) {
    client.flush();
    client.stop();
//    Serial.println("Client stopped");
  }
}

void connect_to_update(prediction* _route, String _URL) {
  num_predictions = 0;
  Serial.println("");
    // if you get a connection, report back via serial:
  if (client.connect(nextmuni, 80)) {
    Serial.println("Connected");
    delay(100);
//    client.println("GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=4448 HTTP/1.0");

// DNS-based request:
//    client.println("GET /N-munixml.txt HTTP/1.0");
    client.println(_URL);
//    client.println("GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=4448 HTTP/1.1");
    client.println("Host: webservices.nextbus.com");
//    client.println("User-Agent: arduino");
//    client.println("Accept: */*");
//    client.println("Connection: close");
    client.println();
    delay(250);
    while (client.connected()) {
      if (client.available()) {
        serialEvent(_route); 
        // test it, take action
        // clear the string, get ready for the next one
      }
    }
//    Serial.print("Route: "); Serial.print(_route->route); Serial.print(" "); Serial.print(_route->route_direction); Serial.println("|");
    // record the last time the connection was attempted
    _route->last_attempt = millis();
//    Serial.println("last attempt: " + N_ptr->last_attempt);
  }  else {
    Serial.println("Update failed");
  }
}


////////////////////////////
// Process each char from web
void serialEvent(prediction* _route) {   
  char inChar = client.read();   
  if ( (inChar == 10) /* || (inChar == CR) /* || (inChar == LT) */) {
//    addChar('\0', tmpStr_ptr);
//    Serial.print("temp line: "); Serial.print(tmpStr); Serial.println("|");
    // take action
    if (strspn(tmpStr_ptr, "<predictions") == 12) {
//      Serial.println("this is a prediction category");
      extractRoute(tmpStr, _route);
    }
    else if (strspn(tmpStr_ptr, "  <prediction epochTime") == 23) {
//      Serial.println("this is a prediction time");
      extractTime(tmpStr, _route);
    }
    else if (strspn(tmpStr_ptr, "  <direction title=") == 19) {
//      Serial.println("this is a route title");
      extractDir(tmpStr, _route);
    }
    // clear the string for the next line
    clearStr(tmpStr_ptr);
  }
  else if (strlen(tmpStr) < MAX_STRING_LEN)  {
    addChar(inChar, tmpStr_ptr);
  }
}

void extractRoute(char * _tmpStr, prediction* _route) {
  const char * RouteTitleIndexStart;
  char * RouteTitleIndexEnd;
  char route[16];
  int string_length_route;
  RouteTitleIndexStart = strstr(_tmpStr, "routeTitle="); //This gives the name of the MUNI line
  RouteTitleIndexEnd = strstr(_tmpStr, "routeTag="); //routeTag is the next tag after RouteTitle
  string_length_route = (RouteTitleIndexEnd - 2) - (RouteTitleIndexStart+12);
//  Serial.print("route length: "); Serial.println(string_length_route);
  memcpy(route, RouteTitleIndexStart+12, string_length_route);
  route[string_length_route] = '\0';
//  Serial.print("route: "); Serial.print(route);Serial.println("|");
  memcpy(_route->route, route, 16);
}

void extractTime(char * _tmpStr, prediction* _route) {
  if (num_predictions < 3) {
    const char * predictionValueIndexStart;
    char * predictionValueIndexEnd;
    int string_length_pred;
    char mins[3];
    predictionValueIndexStart = strstr(tmpStr_ptr, "minutes=");
    predictionValueIndexEnd = strstr(tmpStr_ptr, "isDeparture=");
    string_length_pred = (predictionValueIndexEnd - 2) - (predictionValueIndexStart + 9);
  //  Serial.print("time length: ");Serial.println(string_length_pred);
    memcpy(mins, predictionValueIndexStart+9, string_length_pred);
    mins[string_length_pred] = '\0';
//    Serial.print("mins: "); Serial.print(mins);Serial.println("|");
    memcpy(_route->prediction_time[num_predictions], mins, string_length_pred);
    num_predictions++;
  }
}

void extractDir(char * _tmpStr, prediction* _route) {
  const char * directionTitleIndexStart;
  char * directionTitleIndexEnd;
  int string_length_dir; 
  char dir[8];
  directionTitleIndexStart = strstr(tmpStr_ptr, "title=");
  directionTitleIndexEnd = strstr(tmpStr_ptr, "to ");
  string_length_dir = (directionTitleIndexEnd - 2) - (directionTitleIndexStart + 6);
//  Serial.print("direction length: ");Serial.println(string_length_dir);
  memcpy(dir, directionTitleIndexStart+7, string_length_dir);
  dir[string_length_dir] = '\0';
  memcpy(_route->route_direction, dir, 8);
//  Serial.print("direction: "); Serial.print(dir);Serial.println("|");
}

//     delay(5);
////     Serial.print("end of row, tempstring: "); Serial.println(tmpStr_ptr);
////     Serial.println(strspn(tmpStr, "<predictions"));
//     if (strspn(tmpStr_ptr, "<predictions") == 12) {
//       Serial.println("predictions found");
//       if (_route->route != "") {
//         const char * RouteTitleIndexStart;
//         char * RouteTitleIndexEnd;
//         RouteTitleIndexStart = strstr(tmpStr_ptr, "routeTitle="); //This gives the name of the MUNI line
//         RouteTitleIndexEnd = strstr(tmpStr_ptr, "routeTag="); //routeTag is the next tag after RouteTitle
////         Serial.print("route title starts: ");Serial.println(RouteTitleIndexStart+11); Serial.print("ends: ");Serial.println(RouteTitleIndexEnd-2);
//////         _route->route = tmpStr[RouteTitleIndexStart + 12, RouteTitleIndexEnd - 2);
//         string_length_route = (RouteTitleIndexEnd - 2) - (RouteTitleIndexStart+12);
//         Serial.print("length: ");Serial.println(string_length_route);
//         char* tmpRoute[string_length_route+1];
//         Serial.print("Free: "); Serial.println(freeRam());
//         strncpy(*tmpRoute, (RouteTitleIndexStart+12), string_length_route);
//         *tmpRoute[string_length_route+1] = '\0';
//         Serial.print("tmpRoute: "); Serial.println(*tmpRoute);
//         Serial.print("Free: "); Serial.println(freeRam());
////         _route->route = String(*tmpRoute);
//       }
//     }
     
//    if (strspn(tmpStr_ptr, "  <prediction epochTime") == 23) {
//      Serial.println("true!");
//      if (num_predictions < 3) {
//         const char * predictionValueIndexStart;
//         char * predictionValueIndexEnd;
////         char tmpPred[4]; // max prediction time will be 3 digits long, plus one more for null terminating char
////         char* tmpPred_ptr = &tmpPred[0];
//         
//         predictionValueIndexStart = strstr(tmpStr_ptr, "minutes=");
//         predictionValueIndexEnd = strstr(tmpStr_ptr, "isDeparture=");
//         string_length_pred = (predictionValueIndexEnd - 2) - (predictionValueIndexStart + 9);
//         char* tmpPred[string_length_pred+1];
////         Serial.print("prediction starts: ");Serial.println(predictionValueIndexStart + 9);
//         Serial.print("length: "); Serial.println(string_length_pred);
//         strncpy(*tmpPred, (predictionValueIndexStart+9), string_length_pred);
//         *tmpPred[string_length_pred-1] = '\0';
//         Serial.print("Minutes: "); Serial.print(*tmpPred); Serial.println("|");
////         strncpy(_route->prediction_time[num_predictions], &tmpStr[predictionValueIndexStart + 10], string_length - 10); 
//         
////         _route->prediction_time[num_predictions] = tmpPred;   
////         PredictionValue.toCharArray(_route->prediction_time[num_predictions], 3);
//         num_predictions++;
//      }
//     }
     
//     if (strspn(tmpStr_ptr, "  <direction title=") == 1) {
//       Serial.println("directions found");
//         const char * directionTitleIndexStart;
//         char * directionTitleIndexEnd;
//         
//         directionTitleIndexStart = strstr(tmpStr_ptr, "title=");
//         directionTitleIndexEnd = strstr(tmpStr_ptr, "to ");
//         Serial.print("direction title starts: ");Serial.println(directionTitleIndexStart); Serial.print("ends: ");Serial.println(directionTitleIndexEnd);
//         string_length_dir = (directionTitleIndexEnd - 1) - (directionTitleIndexStart + 6);
//         char* tmpDir[string_length_dir];
//         Serial.print("length: "); Serial.println(string_length_dir);
//         strncpy(*tmpDir, (directionTitleIndexStart+6), string_length_dir);
//         *tmpDir[string_length_dir+1] = '\0';
//         Serial.print("Direction: "); Serial.print(*tmpDir); Serial.println("|");
//
////         strncpy(tmpDirection, &tmpStr[directionTitleIndexStart + 10], string_length - 10);
////         _route->route_direction = String(tmpDirection);
//     }
//     
//     // Clear all Strings
//     clearStr(tmpStr_ptr);
//   }
//}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
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
 

