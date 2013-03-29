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
//#include <avr/pgmspace.h>


// Max String length may have to be adjusted depending on data to be extracted
#define MAX_STRING_LEN  225
#define MAX_STRING_ROWS 5

//char p_buffer[100];
//#define P(str) (strcpy_P(p_buffer, PSTR(str)), p_buffer)

// Enter a MAC address and IP address for your controller below.
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//byte ip[] = { 192,168,1,177 };
//// IP address for bench testing with Mac mini
IPAddress ip(192,168,1,177);
IPAddress myDns(8,8,8,8);
byte gateway[] = { 192,168,1,176 }; // my macbook, sharing its internet connection
byte subnet[] = { 255,255,255,0 };
////

//byte nextmuni[] = { 64,124,123,57 }; // nextmuni API, IP address resolved by webservices.nextbus.com
//byte nextmuni[] = { 192,168,1,176 }; // nextmuni API simulated by local computer MAMP

/// IP address for kitchen MX60
//IPAddress ip(192,168,0,75);
//IPAddress myDns(8,8,8,8);
//byte gateway[] = { 192,168,0,254 }; // my macbook, sharing its internet connection
//byte subnet[] = { 255,255,255,0 };
////

char nextmuni[] = "webservices.nextbus.com";
char tmpStr[MAX_STRING_LEN] = "";
char* tmpStr_ptr = &tmpStr[0];
char mins[3] = "";
char* mins_ptr = &mins[0];


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

prediction N_in, N_out, J_in, J_out, KT_in, L_in, M_in, twentytwo_in, twentytwo_out, seventyone_out;
prediction* J_in_ptr = &J_in; prediction* J_out_ptr = &J_out;
prediction* KT_in_ptr = &KT_in; 
prediction* L_in_ptr = &L_in; 
prediction* M_in_ptr = &M_in; 
prediction* N_in_ptr = &N_in; prediction* N_out_ptr = &N_out;
prediction* twentytwo_in_ptr = &twentytwo_in;  prediction* twentytwo_out_ptr = &twentytwo_out;
prediction* seventyone_out_ptr = &seventyone_out;
int num_avail_routes = 10;
prediction* avail_routes[10] = {N_in_ptr, N_out_ptr, J_in_ptr, J_out_ptr, KT_in_ptr, L_in_ptr, M_in_ptr, twentytwo_in_ptr, twentytwo_out_ptr, seventyone_out_ptr };

//static const char N_in_URL[] = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=4448 HTTP/1.0";
//static const char N_out_URL[] = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=N&s=4447 HTTP/1.0";
//static const char J_in_URL[] = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=J&s=4006 HTTP/1.0";
//static const char J_out_URL[] = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=J&s=7316 HTTP/1.0";
//char KT_in_URL[] = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=KT&s=5726 HTTP/1.0";
////  String KT_out_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=KT&s=6998 HTTP/1.0";
////  String L_in_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=L&s=4006 HTTP/1.0";
////  String L_out_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=L&s=6998 HTTP/1.0";
////  String M_in_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=M&s=5726 HTTP/1.0";
////  String M_out_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=M&s=6998 HTTP/1.0";
//String six_in_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=6&s=4953 HTTP/1.0";
//String six_out_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=6&s=4952 HTTP/1.0";
//static const char twentytwo_in_URL[] = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=22&s=4620 HTTP/1.0";
//static const char twentytwo_out_URL[] = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=22&s=4618 HTTP/1.0";
//String seventyone_in_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=71&s=4953 HTTP/1.0";
//static const char seventyone_out_URL[] = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=71&s=4952 HTTP/1.0";
static String base_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=";

//char base_URL[] = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=";

volatile byte num_predictions = 0;
//int attempt_connect = 1;
volatile byte next_displayed = 0;
long last_display_refresh;
volatile boolean display_direction = true;

//setup display with pin definitions
LiquidCrystal lcd(3, 9, 5, 6, 7, 8);

EthernetClient client;

void setup() {
  Serial.begin(115200);
//  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  lcd.begin(16, 2);
  delay(50);
  lcd.clear();
  lcd.print("Initializing... ");
//  lcd.clear();
//  lcd.print("Getting IP...   "); 
  Ethernet.begin(mac, ip, myDns, gateway, subnet); // start the Ethernet connection:
//  if (Ethernet.begin(mac) == 0 ) {
//    Serial.println("No DHCP");
//  }
   // start the Ethernet connection:
  delay(10);
  Serial.println(" ");Serial.println("Initializing...");Serial.println("");
//  Serial.print("IP address is: ");
//  Serial.println(Ethernet.localIP());
  // configure the prescaler to CPU clock divided by 1024
  // (CPU frequency) / (prescaler value)  => 64us. // CPU = 16 MHz, prescaler = 1024
  // (desired period = 3 sec) / 64us = 46,875  
  OCR1A =  46875;
  TCCR1B |= ((1<<WGM12) | (1<<CS12) | (1<<CS10)); 
  TIMSK1 |= (1<<OCIE1A);  // enable Compare Match A interrupt enable
//  TIMSK1 &= ~(1<<TOIE1); // disable the timer overflow interrupt while configuring
//  TCCR1A &= ~((1<<WGM11) | (1<<WGM10));
//  TCCR1B &= ~((1<<WGM13) | (1<CS11)); // configure timer1 in output compare mode  (CTC)
//  
//  DDRD &= ~(1<<DDD2); // set PD2 to input
//  PORTD |= (1<<PORTD2); // set PD2 to high 
//  pinMode(2, INPUT);
//  digitalWrite(2, HIGH);
//  interrupts();
//  EIMSK |= (1<<INT0);
//  EICRA |= ((1<<ISC01)); // enable external interrupt 0 with falling edge interrupt

  attachInterrupt(0, pin2ISR, FALLING);
    
//  interrupts();
  delay(100); // give the Ethernet shield a moment to initialize
  N_in_ptr->last_refreshed = 0;
  N_in_ptr->route_direction = 1;
  
  N_out_ptr->last_refreshed = 0;
  N_out_ptr->route_direction = 0;  
  
  J_in_ptr->last_refreshed = 0;
  J_in_ptr->route_direction = 1;

  J_out_ptr->last_refreshed = 0;
  J_out_ptr->route_direction = 0;  
  
  KT_in_ptr->last_refreshed = 0;
  KT_in_ptr->route_direction = 1;

  L_in_ptr->last_refreshed = 0;
  L_in_ptr->route_direction = 1;

  M_in_ptr->last_refreshed = 0;
  M_in_ptr->route_direction = 1;

  twentytwo_in_ptr->last_refreshed = 0;
  twentytwo_in_ptr->route_direction = 1;
  
  twentytwo_out_ptr->last_refreshed = 0;
  twentytwo_out_ptr->route_direction = 0; 
  
  seventyone_out_ptr->last_refreshed = 0;
  seventyone_out_ptr->route_direction = 0;
  
  memmove(N_in_ptr->route, "N-Judah Inbound ", 16);
  memmove(N_out_ptr->route, "N-Judah Outbound", 16);  
  memmove(J_in_ptr->route, "J-Church Inbound", 16);
  memmove(J_out_ptr->route, "J-Church Out    ", 16); 
  memmove(KT_in_ptr->route, "KT-Ingl/3rd In  ", 16);
  memmove(L_in_ptr->route, "L-Taraval In    ", 16);
  memmove(M_in_ptr->route, "M-Ocean View In", 16);
  memmove(twentytwo_in_ptr->route, "22-Fillmore In  ", 16);
  memmove(twentytwo_out_ptr->route, "22-Fillmore Out ", 16);
  memmove(seventyone_out_ptr->route, "71-Haight-No Out", 16);
  last_display_refresh = millis();
  delay(200);
}

void loop()
{
// find time of last update for the route, if needed, re-connect to the URL for the route, extraction some number of predictions
// store predictions and timestamp of update into memory, search for any special messages, move to the next route

/* ////// N-Judah inbound \\\\\\\ */
  if (millis() - N_in_ptr->last_attempt > request_interval) {
//    char N_in_URL = URL_constructor(4448,"N"); // N-Judah, inbound from Church and Duboce
//    N_in_ptr->attempt_connect = 1;
    connect_to_update_prog(N_in_ptr, 1, 4448, "N");
    delay(10);
//    N_in_ptr->attempt_connect = 0;
    clear_client();
  }
  
/* ////// N-Judah outbound \\\\\\\ */
  if (millis() - N_out_ptr->last_attempt > request_interval) {
//    String N_out_URL = URL_constructor(4447,"N"); // N-Judah, outbound from Church and Duboce
//    N_out_ptr->attempt_connect = 1;
    connect_to_update_prog(N_out_ptr, 0, 4447, "N");
    delay(50);
//    N_out_ptr->attempt_connect = 0;
    clear_client();
  }

/* ////// J-Church inbound \\\\\\\ */
  if (millis() - J_in_ptr->last_attempt > request_interval) {
//    String J_in_URL = URL_constructor(4006, "J"); // J-Church, inbound from Church and Duboce
//    J_in_ptr->attempt_connect = 1;
    connect_to_update_prog(J_in_ptr,1, 4006, "J");
    delay(50);
//    J_in_ptr->attempt_connect = 0;
    clear_client();
  }

/* ////// J-Church outbound \\\\\\\ */
  if (millis() - J_out_ptr->last_attempt > request_interval) {
//    String J_out_URL = URL_constructor(7316, "J"); // J-Church, inbound from Church and Duboce
//    J_out_ptr->attempt_connect = 1;
    connect_to_update_prog(J_out_ptr, 0, 7316, "J");
    delay(50);
//    J_out_ptr->attempt_connect = 0;
    clear_client();
  }
  
/* ////// KT-Ingleside/Third Street \\\\\\\ */
  if (millis() - KT_in_ptr->last_attempt > request_interval) {
//    String K_in_URL = URL_constructor(5726, "KT"); // J-Church, inbound from Church and Duboce
    connect_to_update_prog(KT_in_ptr, 1, 5726, "KT");
    delay(50);
    clear_client();
  }
  
/* ////// L-Taraval \\\\\\\ */
  if (millis() - L_in_ptr->last_attempt > request_interval) {
//    String L_in_URL = URL_constructor(5726, "L"); // J-Church, inbound from Church and Duboce
    connect_to_update_prog(L_in_ptr, 1, 5726, "L");
    delay(50);
    clear_client();
  }
  
/* ////// M-Ocean View \\\\\\\ */
  if (millis() - M_in_ptr->last_attempt > request_interval) {
    connect_to_update_prog(M_in_ptr, 1, 5726, "M");
    delay(50);
    clear_client();
  }

///* ////// 22-Fillmore inbound \\\\\\\ */
  if (millis() - twentytwo_in_ptr->last_attempt > request_interval) {
//    String twentytwo_in_URL = URL_constructor(4620, "22"); // 22-Fillmore, inbound to marina from Haight and Fillmore
//    twentytwo_in_ptr->attempt_connect = 1;
    connect_to_update_prog(twentytwo_in_ptr, 1, 4620, "22");
    delay(50);
//    twentytwo_in_ptr->attempt_connect = 0;
    clear_client();
  }

/* ////// 22-Fillmore outbound \\\\\\\ */
  if (millis() - twentytwo_out_ptr->last_attempt > request_interval) {
//    String twentytwo_out_URL = URL_constructor(4618, "22"); // 22-Fillmore, outbound to dogpatch from Haight and Fillmore
//    twentytwo_out_ptr->attempt_connect = 1;
    connect_to_update_prog(twentytwo_out_ptr, 0, 4618, "22"); // 0 indicated outbound, 1 indicates inbound
    delay(50);
//    twentytwo_out_ptr->attempt_connect = 0;
    clear_client();
  }

/* ////// 71-Haight outbound \\\\\\\ */
  if (millis() - seventyone_out_ptr->last_attempt > request_interval) {
//    String seventyone_out_URL = URL_constructor(4952, "71"); // 71-Haight, outbound from Haight and Fillmore
//    seventyone_out_ptr->attempt_connect = 1;
    connect_to_update_prog(seventyone_out_ptr, 0, 4952, "71");
    delay(50);
//    seventyone_out_ptr->attempt_connect = 0;
    clear_client();
  }
  
//  Serial.print("mem: ");Serial.println(freeRam());

// END OF LOOP
}

void update_display(int _next_displayed) {
 prediction* this_route = avail_routes[_next_displayed];
 lcd.clear();
 lcd.print(this_route->route);
 lcd.setCursor(0,1);
 for (byte i=0; i<3;i++) {
    for (byte j=0;j<2;j++) {
//        Serial.print(this_route->prediction_time_in[i][j]);
      if (this_route->prediction_time[i][j] != '\0') lcd.print(this_route->prediction_time[i][j]);
    } // end j for loop
    if ((i<2) && (this_route->prediction_time[i+1][0] != '\0')) { /* Serial.print(", "); */ lcd.print(", "); }
    else { /* Serial.println(" "); */ lcd.print(" "); }
    if (this_route->prediction_time[0][0] == '\0') { lcd.setCursor(0,1); lcd.print("--- No data --- "); }
  } // end i for loop
//  Serial.println(freeRam());
  if (next_displayed < (num_avail_routes - 1)) next_displayed++;
  else next_displayed = 0;
}

void connect_to_update_prog(prediction* _route, boolean _dir, int _stop_ID, char _route_line[2]) {
  String _URL = URL_constructor(_stop_ID, _route_line);
  num_predictions = 0;
  byte no_char_count = 0;
  Serial.println("");

  // first clear stale data for either inbound or outbound
  for (int i=0; i<3; i++) {
      for (int j=0; j<3; j++) {
        _route->prediction_time[i][j] = '\0';
      }
  }
    // if you get a connection, report back via serial:
  if (client.connect(nextmuni, 80)) {
    
    Serial.print("Updating ");Serial.print(_route->route);Serial.println("");
    Serial.println("Connected");
    delay(50);
    client.println(_URL); // needed
    client.println("Host: webservices.nextbus.com");  // needed
    client.println();
    delay(50);
    while (client.connected()) {
      while (client.available()) {
        serialEvent(_route);
        no_char_count = 0;
      }
      delay(1);
      no_char_count++;
      if (no_char_count > 1000) client.stop();
    }
    client.stop();
    delay(250);    
    _route->last_attempt = millis();
  } else {
    Serial.print("Update failed for "); Serial.print(_URL); Serial.print(" "); Serial.println(_dir);
  }
}


////////////////////////////
// Process each char from web server
void serialEvent(prediction* _route) {   
  char inChar = client.read();
  Serial.print(inChar);
  if (num_predictions < 3) {
    if ( (inChar == 10)  || (inChar == CR)  || (inChar == LT) ) {
      Serial.println("line feed found");
      if (strspn(tmpStr_ptr, "  <prediction epochTime") == 23) {
        Serial.println("matched prediction time");
        extractTime(mins_ptr);
        memmove(_route->prediction_time[num_predictions], mins_ptr, 3);
        Serial.print("prediction: "); Serial.println(_route->prediction_time[num_predictions]);
        num_predictions++;
      }
      clearStr(tmpStr_ptr);
    }
    else if (strlen(tmpStr) < MAX_STRING_LEN)  {
      addChar(inChar, tmpStr_ptr);
    }
    else clearStr(tmpStr_ptr);
  }
}

void extractTime(char* _mins) {
    const char * predictionValueIndexStart;
    char * predictionValueIndexEnd;
    byte string_length_pred;
    predictionValueIndexStart = strstr(tmpStr_ptr, "minutes=");
    predictionValueIndexEnd = strstr(tmpStr_ptr, "isDeparture=");
    string_length_pred = (predictionValueIndexEnd - 2) - (predictionValueIndexStart + 9);
    memmove(_mins, predictionValueIndexStart+9, string_length_pred);
    if (string_length_pred == 1) _mins[1] = '\0';
    Serial.print("mins: "); Serial.println(_mins);
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

String URL_constructor(int _stop_ID, char _route[2]) {
//  String base_URL = "GET /service/publicXMLFeed?command=predictions&a=sf-muni&r=";
  String _URL = base_URL + _route + "&s=" + _stop_ID + " HTTP/1.0";
  return _URL;
}

// Function to clear a String
void clearStr (char* str) {
   byte len = strlen(str);
   for (int c = 0; c < len; c++) {
      str[c] = '\0'; // str[c] = NULL;
   }
}

//Function to add a char to a String and check its length
void addChar (char ch, char* str) {
  byte len = strlen(str);
  str[len] = ch;
}

ISR(TIMER1_COMPA_vect) { // this called on every overflow interrupt, currently timed at every 3 secs
//  if (display_direction) update_display(next_displayed);
  update_display(next_displayed);
}

void pin2ISR(void) {
//  noInterrupts();
  delayMicroseconds(5000);
  Serial.println(TCNT1);
  update_display(next_displayed);
  if (next_displayed < (num_avail_routes - 1)) next_displayed++;
  else next_displayed = 0;
  TCNT1=0;
//  interrupts();
}

void clear_client(void) {
//  if (!client.connected()) {
//    client.flush();
    client.stop();
//  }
}

//ISR(EXT_INT0_vect) {
////  noInterrupts();
//  delayMicroseconds(5000);
//  if (display_direction) update_display(next_displayed, 1);
//  else {
//    update_display(next_displayed, 0);
//    if (next_displayed < (num_avail_routes - 1)) next_displayed++;
//    else next_displayed = 0;
//  }
//  display_direction = !display_direction;
////  interrupts();
//}

void initial_data(void) {
//  String N_in_URL = URL_constructor(4448,"N"); // N-Judah, inbound from Church and Duboce
//  N_in_ptr->attempt_connect = 1;
//  connect_to_update(N_in_ptr, N_in_URL, 1);
//  delay(200);
//  N_in_ptr->attempt_connect = 0;
//  clear_client();
  
//  String N_out_URL = URL_constructor(4447,"N"); // N-Judah, outbound from Church and Duboce
//  N_ptr->attempt_connect = 1;
//  connect_to_update(N_ptr, N_out_URL, 0);
//  delay(200);
//  N_ptr->attempt_connect = 0;
//  clear_client();
//  
//  String J_in_URL = URL_constructor(4006, "J"); // J-Church, inbound from Church and Duboce
//  J_ptr->attempt_connect = 1;
//  connect_to_update(J_ptr, J_in_URL, 1);
//  delay(200);
//  J_ptr->attempt_connect = 0;
//  clear_client();
//
//  String J_out_URL = URL_constructor(7316, "J"); // J-Church, inbound from Church and Duboce
//  J_ptr->attempt_connect = 1;
//  connect_to_update(J_ptr, J_out_URL, 0);
//  delay(200);
//  J_ptr->attempt_connect = 0;
//  clear_client();
// 
//  String six_in_URL = URL_constructor(4953, "6"); // 6-Parnassus, inbound from Haight and Fillmore
//  six_ptr->attempt_connect = 1;
//  connect_to_update(six_ptr, six_in_URL, 1);
//  delay(200);
//  six_ptr->attempt_connect = 0;
//  clear_client();
//
//  String six_out_URL = URL_constructor(4952, "6"); // 6-Parnassus, outbound from Haight and Fillmore
//  six_ptr->attempt_connect = 1;
//  connect_to_update(six_ptr, six_out_URL, 0);
//  delay(200);
//  six_ptr->attempt_connect = 0;
//  clear_client();
//
//  String twentytwo_in_URL = URL_constructor(4620, "22"); // 22-Fillmore, inbound to marina from Haight and Fillmore
//  twentytwo_ptr->attempt_connect = 1;
//  connect_to_update(twentytwo_ptr, twentytwo_in_URL, 1);
//  delay(200);
//  twentytwo_ptr->attempt_connect = 0;
//  clear_client();
//
//  String twentytwo_out_URL = URL_constructor(4618, "22"); // 22-Fillmore, outbound to dogpatch from Haight and Fillmore
//  twentytwo_ptr->attempt_connect = 1;
//  connect_to_update(twentytwo_ptr, twentytwo_out_URL, 0); // 0 indicated outbound, 1 indicates inbound
//  delay(200);
//  twentytwo_ptr->attempt_connect = 0;
//  clear_client();
//
//  String seventyone_in_URL = URL_constructor(4953, "71"); // 71-Haight, inbound from Haight and Fillmore
//  seventyone_ptr->attempt_connect = 1;
//  connect_to_update(seventyone_ptr, seventyone_in_URL, 1);
//  delay(200);
//  seventyone_ptr->attempt_connect = 0;
//  clear_client();
// 
//  String seventyone_out_URL = URL_constructor(4952, "71"); // 71-Haight, outbound from Haight and Fillmore
//  seventyone_ptr->attempt_connect = 1;
//  connect_to_update(seventyone_ptr, seventyone_out_URL, 0);
//  delay(200);
//  seventyone_ptr->attempt_connect = 0;
//  clear_client();
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
 

