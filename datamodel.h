#ifndef datamodel_h
#define datamodel_h

#include <WString.h>

typedef struct {
  char route[16];
  char route_direction[8];
//  String stop_description;
  
  int in_stop_ID;
//  time_t in_last_update;
  char prediction_time[3][2];
//  String in_prediction_URL;
  int attempt_connect;
  int out_stop_ID;
//  time_t out_last_update;
//  String out_prediction_URL;
  long last_attempt;
} prediction;

#endif 
