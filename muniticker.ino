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
#include <LiquidCrystal.h>

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
byte len;
byte rowCounter = 0;

const long request_interval = 60000;  // delay between requests, milliseconds
const int refresh_interval = 3000;  // delay between screen refreshes, 3 seconds
//long last_refreshed = 0;            // last time text was written to display

/* Timer2 reload value, globally available */  
unsigned int tcnt2; 

prediction N, F, J, K, L, M, six, twentytwo, seventyone;
prediction* F_ptr = &F; prediction* J_ptr = &J; prediction* K_ptr = &K;
prediction* L_ptr = &L; prediction* M_ptr = &M; prediction* N_ptr = &N;
prediction* six_ptr = &six; prediction* twentytwo_ptr = &twentytwo; prediction* seventyone_ptr = &seventyone;
//prediction* avail_routes[] = {N_ptr, J_ptr, twentytwo_ptr, seventyone_ptr};
prediction* avail_routes[] = {N_ptr, J_ptr, six_ptr, twentytwo_ptr, seventyone_ptr};
int num_avail_routes = 5;

byte num_predictions = 0;
//int attempt_connect = 1;
byte next_displayed = 0;
long last_display_refresh;
boolean display_direction = true;

//setup display with pin definitions
LiquidCrystal lcd(2, 3, 5, 6, 7, 8);

EthernetClient client;

void setup() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  lcd.begin(16, 2);
  delay(50);
  lcd.clear();
  lcd.print("Initializing... ");
  Ethernet.begin(mac, ip, myDns, gateway, subnet); // start the Ethernet connection:
  Serial.begin(115200);
  delay(10);
  OCR1A =  46875;
  TCCR1B |= ((1<<WGM12) | (1<<CS12) | (1<<CS10)); 
  TIMSK1 |= (1<<OCIE1A);  // enable Compare Match A interrupt enable
//  TIMSK1 &= ~(1<<TOIE1); // disable the timer overflow interrupt while configuring
//  TCCR1A &= ~((1<<WGM11) | (1<<WGM10));
  
//  TCCR1B &= ~((1<<WGM13) | (1<CS11)); // configure timer1 in output compare mode  (CTC)
  
  // configure the prescaler to CPU clock divided by 1024
  // (CPU frequency) / (prescaler value)  => 64us. // CPU = 16 MHz, prescaler = 1024
  // (desired period = 3 sec) / 64us = 46,875  
   
  interrupts();
  
  delay(100); // give the Ethernet shield a second to initialize
  N_ptr->attempt_connect = 1;
  N_ptr->last_refreshed_in = 0;
  N_ptr->last_refreshed_out = 0;
  J_ptr->attempt_connect = 1;
  J_ptr->last_refreshed_in = 0;
  J_ptr->last_refreshed_out = 0;
  six_ptr->attempt_connect = 1;
  six_ptr->last_refreshed_in = 0;
  six_ptr->last_refreshed_out = 0;
  twentytwo_ptr->attempt_connect = 1;
  twentytwo_ptr->last_refreshed_in = 0;
  twentytwo_ptr->last_refreshed_out = 0;
  seventyone_ptr->attempt_connect = 1;
  seventyone_ptr->last_refreshed_in = 0;
  seventyone_ptr->last_refreshed_out = 0; 
  Serial.println(" ");Serial.println("Initializing...");Serial.println("");
  last_display_refresh = millis();
}

void loop()
{
// find time of last update for the route, if needed, re-connect to the URL for the route, extraction some number of predictions
// store predictions and timestamp of update into memory, search for any special messages, move to the next route

//Serial.print("mem free 1: ");Serial.println(freeRam());

// update the display to start
//if (millis() - last_display_refresh > refresh_interval) {
//  Serial.println("");Serial.println("refreshing...");Serial.println("");
//  if (display_direction) update_display(next_displayed, 1);
//  else update_display(next_displayed, 0);
//  display_direction = !display_direction;
//  if (next_displayed < (num_avail_routes - 1)) next_displayed++;
//  else next_displayed = 0;
//  last_display_refresh = millis();
//}

/* ////// N-Judah \\\\\\\ */
  if (millis() - N_ptr->last_attempt_in > request_interval) {
    String N_in_URL = URL_constructor(4448,"N"); // N-Judah, inbound from Church and Duboce
    N_ptr->attempt_connect = 1;
    connect_to_update(N_ptr, N_in_URL, 1);
    delay(50);
    N_ptr->attempt_connect = 0;
  }

//  else if (millis() - N_ptr->last_refreshed_in > refresh_interval) {
//    // reprint previously recorded times
//    if (strlen(N_ptr->route) > 1) {
//      N_ptr->last_refreshed_in = millis();
//      update_display(next_displayed, 1);
//      if (next_displayed < (num_avail_routes - 1)) next_displayed++;
//      else next_displayed = 0;
//    }
//  }

  if (!client.connected()) {
    client.flush();
    client.stop();
  }
  
/* ////// N-Judah \\\\\\\ */
  if (millis() - N_ptr->last_attempt_out > request_interval) {
    String N_out_URL = URL_constructor(4447,"N"); // N-Judah, outbound from Church and Duboce
    N_ptr->attempt_connect = 1;
    connect_to_update(N_ptr, N_out_URL, 0);
    delay(400);
    N_ptr->attempt_connect = 0;
  }

  if (!client.connected()) {
    client.flush();
    client.stop();
  }

/* ////// J-Church \\\\\\\ */
  if (millis() - J_ptr->last_attempt_in > request_interval) {
    String J_URL = URL_constructor(4006, "J"); // J-Church, inbound from Church and Duboce
    Serial.println("Updating J-Church");
    J_ptr->attempt_connect = 1;
    connect_to_update(J_ptr, J_URL, 1);
    delay(400);
    J_ptr->attempt_connect = 0;
  }

  if (!client.connected()) {
    client.flush();
    client.stop();
  }

/* ////// J-Church \\\\\\\ */
  if (millis() - J_ptr->last_attempt_out > request_interval) {
    String J_URL = URL_constructor(7316, "J"); // J-Church, inbound from Church and Duboce
//    Serial.println("Updating J-Church");
    J_ptr->attempt_connect = 1;
    connect_to_update(J_ptr, J_URL, 0);
    delay(50);
    J_ptr->attempt_connect = 0;
  }

  if (!client.connected()) {
    client.flush();
    client.stop();
  }

/* ////// 6-Parnassus \\\\\\\ */
  if (millis() - six_ptr->last_attempt_in > request_interval) {
    String six_URL = URL_constructor(4953, "6"); // 6-Parnassus, inbound from Haight and Fillmore
//    Serial.println("Updating 6-Parnassus");
    six_ptr->attempt_connect = 1;
    connect_to_update(six_ptr, six_URL, 1);
    delay(50);
    six_ptr->attempt_connect = 0;
  }

  if (!client.connected()) {
    client.flush();
    client.stop();
  }
  
/* ////// 6-Parnassus \\\\\\\ */
  if (millis() - six_ptr->last_attempt_out > request_interval) {
    String six_URL = URL_constructor(4952, "6"); // 6-Parnassus, outbound from Haight and Fillmore
//    Serial.println("Updating 6-Parnassus");
    six_ptr->attempt_connect = 1;
    connect_to_update(six_ptr, six_URL, 0);
    delay(50);
    six_ptr->attempt_connect = 0;
  }

  if (!client.connected()) {
    client.flush();
    client.stop();
  }  


///* ////// 22-Fillmore \\\\\\\ */
  if (millis() - twentytwo_ptr->last_attempt_in > request_interval) {
    String twentytwo_URL = URL_constructor(4620, "22"); // 22-Fillmore, inbound to marina from Haight and Fillmore
//    Serial.println("Updating 22-Fill");
    twentytwo_ptr->attempt_connect = 1;
    connect_to_update(twentytwo_ptr, twentytwo_URL, 1);
    delay(50);
    twentytwo_ptr->attempt_connect = 0;
  }

  if (!client.connected()) {
    client.flush();
    client.stop();
  }
  
///* ////// 22-Fillmore \\\\\\\ */
  if (millis() - twentytwo_ptr->last_attempt_out > request_interval) {
    String twentytwo_URL = URL_constructor(4618, "22"); // 22-Fillmore, outbound to dogpatch from Haight and Fillmore
//    Serial.println("Updating 22-Fill");
    twentytwo_ptr->attempt_connect = 1;
    connect_to_update(twentytwo_ptr, twentytwo_URL, 0); // 0 indicated outbound, 1 indicates inbound
    delay(50);
    twentytwo_ptr->attempt_connect = 0;
  }

  if (!client.connected()) {
    client.flush();
    client.stop();
  }  
  
  
///* ////// 71-Haight \\\\\\\ */
  if (millis() - seventyone_ptr->last_attempt_in > request_interval) {
    String seventyone_in_URL = URL_constructor(4953, "71"); // 71-Haight, inbound from Haight and Fillmore
//    Serial.println("Updating 71-Haig in");
    seventyone_ptr->attempt_connect = 1;
//    Serial.println(seventyone_URL);
    connect_to_update(seventyone_ptr, seventyone_in_URL, 1);
    delay(50);
    seventyone_ptr->attempt_connect = 0;
  }

  if (!client.connected()) {
    client.flush();
    client.stop();
  }

/* ////// 71-Haight \\\\\\\ */
  if (millis() - seventyone_ptr->last_attempt_out > request_interval) {
    String seventyone_out_URL = URL_constructor(4952, "71"); // 71-Haight, outbound from Haight and Fillmore
//    Serial.println("Updating 71-Haig out");
    seventyone_ptr->attempt_connect = 1;
//    Serial.println(seventyone_URL);
    connect_to_update(seventyone_ptr, seventyone_out_URL, 0);
    delay(50);
    seventyone_ptr->attempt_connect = 0;
  }

  if (!client.connected()) {
    client.flush();
    client.stop();
  }

// END OF LOOP
}

void update_display(int _next_displayed, boolean _dir) {
 byte line_limit = 16;
 byte row_limit = 2;
 prediction* this_route = avail_routes[_next_displayed];
 if ((String(this_route->route_direction_in) == "Inbound") && _dir) {
   if (strlen(this_route->route) > 12) this_route->route[12] = '\0';
   lcd.clear();
   Serial.println("");
   Serial.print(this_route->route);Serial.println(" In  ");
   lcd.print(this_route->route); lcd.print(" In  ");
   lcd.setCursor(0,1);
   for (byte i=0; i<3;i++) {
      for (byte j=0;j<2;j++) {
        Serial.print(this_route->prediction_time_in[i][j]);
        if (this_route->prediction_time_in[i][j] != '\0') lcd.print(this_route->prediction_time_in[i][j]);
      } // end j for loop
      if ((i<2) && (this_route->prediction_time_in[i+1][0] != '\0')) { Serial.print(", "); lcd.print(", "); }
      else { Serial.println(" "); lcd.print(" "); }
    } // end i for loop
 } 
 if ((String(this_route->route_direction_out) == "Outbound") && !_dir) {
   if (strlen(this_route->route) > 12) this_route->route[12] = '\0';
   lcd.clear();
   Serial.println("");
   Serial.print(this_route->route);Serial.println(" Out ");
   lcd.print(this_route->route); lcd.print(" Out ");
   lcd.setCursor(0,1);
   for (byte i=0; i<3;i++) {
    for (byte j=0;j<2;j++) {
      Serial.print(this_route->prediction_time_out[i][j]);
      if (this_route->prediction_time_out[i][j] != '\0') lcd.print(this_route->prediction_time_out[i][j]);
    } // end j for loop
    if ((i<2) && (this_route->prediction_time_out[i+1][0] != '\0')){ Serial.print(", "); lcd.print(", "); }
    else { Serial.println(" "); lcd.print(" "); }
  } // end i for loop
  }
  Serial.println("");
}

void connect_to_update(prediction* _route, String _URL, boolean _dir) {
  num_predictions = 0;
  byte no_char_count = 0;
  Serial.println("");
    // if you get a connection, report back via serial:
  if (client.connect(nextmuni, 80)) {
    // first clear stale data for either inbound or outbound
    if (_dir) {
      for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
          _route->prediction_time_in[i][j] = '\0';
        }
      }
    }
    else {
      for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
          _route->prediction_time_out[i][j] = '\0';
        }
      }
    }
    if (strlen(_route->route) > 12) _route->route[12] = '\0';
    Serial.print("Updating ");Serial.print(_route->route);Serial.println("");
    Serial.println("Connected");
    delay(100);
//    client.println("GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=4448 HTTP/1.0");

// DNS-based request:
//    client.println("GET /71-munixml.txt HTTP/1.0");
    client.println(_URL); // needed
//    client.println("GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=4448 HTTP/1.1");
    client.println("Host: webservices.nextbus.com");  // needed
//    client.println("User-Agent: arduino");
//    client.println("Accept: */*");
//    client.println("Connection: close");
    client.println();
    delay(100);
    while (client.connected()) {
      if (client.available()) {
        serialEvent(_route, _dir);
        no_char_count = 0;
        // test it, take action
        // clear the string, get ready for the next one
      }
      delay(1);
      no_char_count++;
      if (no_char_count > 1000) client.stop();
    }
//    Serial.print("Route: "); Serial.print(_route->route); Serial.print(" "); Serial.print(_route->route_direction); Serial.println("|");
    // record the last time the connection was attempted
    if (_dir) _route->last_attempt_in = millis();
    else _route->last_attempt_out = millis();
//    Serial.println("last attempt: " + N_ptr->last_attempt);
  }  else {
    Serial.println("Update failed");
  }
}


////////////////////////////
// Process each char from web
void serialEvent(prediction* _route, boolean _dir) {   
  char inChar = client.read();
//  Serial.print(inChar);
  if ( (inChar == 10) /* || (inChar == CR) /* || (inChar == LT) */) {
//    addChar('\0', tmpStr_ptr);
//    Serial.print("temp line: "); Serial.print(tmpStr); Serial.println("|");
    // take action
    if (strspn(tmpStr_ptr, "<predictions") == 12) {
      extractRoute(tmpStr, _route);
    }
    else if (strspn(tmpStr_ptr, "  <prediction epochTime") == 23) {
      extractTime(tmpStr, _route, _dir);
    }
    else if (strspn(tmpStr_ptr, "  <direction title=") == 19) {
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
  char route[20];
  byte string_length_route;
  RouteTitleIndexStart = strstr(_tmpStr, "routeTitle="); //This gives the name of the MUNI line
  RouteTitleIndexEnd = strstr(_tmpStr, "routeTag="); //routeTag is the next tag after RouteTitle
  string_length_route = (RouteTitleIndexEnd - 2) - (RouteTitleIndexStart+12);
  memcpy(route, RouteTitleIndexStart+12, string_length_route);
  route[string_length_route] = '\0';
  memcpy(_route->route, route, 20);
}

void extractTime(char * _tmpStr, prediction* _route, boolean _dir) {
  if (num_predictions < 3) {
    const char * predictionValueIndexStart;
    char * predictionValueIndexEnd;
    byte string_length_pred;
    char mins[3];
    predictionValueIndexStart = strstr(tmpStr_ptr, "minutes=");
    predictionValueIndexEnd = strstr(tmpStr_ptr, "isDeparture=");
    string_length_pred = (predictionValueIndexEnd - 2) - (predictionValueIndexStart + 9);
    memcpy(mins, predictionValueIndexStart+9, string_length_pred);
    mins[string_length_pred] = '\0';
    if (_dir) memcpy(_route->prediction_time_in[num_predictions], mins, string_length_pred);
    else memcpy(_route->prediction_time_out[num_predictions], mins, string_length_pred);
    num_predictions++;
  }
}

void extractDir(char * _tmpStr, prediction* _route) {
  const char * directionTitleIndexStart;
  char * directionTitleIndexEnd;
  byte string_length_dir; 
  char dir[8];
  directionTitleIndexStart = strstr(tmpStr_ptr, "title=");
  directionTitleIndexEnd = strstr(tmpStr_ptr, "to ");
  string_length_dir = (directionTitleIndexEnd - 2) - (directionTitleIndexStart + 6);
  memcpy(dir, directionTitleIndexStart+7, string_length_dir);
  dir[string_length_dir] = '\0';
  if (String(dir) == "Inbound") memcpy(_route->route_direction_in, dir, 8);
  else memcpy(_route->route_direction_out, dir, 8);
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

String URL_constructor(int _stop_ID, char _route[2]) {
  String base_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=";
  String _URL = base_URL + _route + "&s=" + _stop_ID + " HTTP/1.0";
  return _URL;
}

// Function to clear a String
void clearStr (char* str) {
   byte len = strlen(str);
   for (int c = 0; c < len; c++) {
      str[c] = 0; // str[c] = NULL;
   }
}

//Function to add a char to a String and check its length
void addChar (char ch, char* str) {
      str[strlen(str)] = ch;
}

ISR(TIMER1_COMPA_vect) {
  if (display_direction) update_display(next_displayed, 1);
  else {
    update_display(next_displayed, 0);
    if (next_displayed < (num_avail_routes - 1)) next_displayed++;
    else next_displayed = 0;
  }
  display_direction = !display_direction;
//  if (next_displayed < (num_avail_routes - 1)) next_displayed++;
//  else next_displayed = 0;
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
 

