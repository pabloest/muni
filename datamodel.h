#ifndef datamodel_h
#define datamodel_h

typedef struct {
  char route[20];
  char route_direction_in[8];
  char route_direction_out[8];
//  String stop_description;
  
  int in_stop_ID;
//  time_t in_last_update;
  char prediction_time_in[3][2];
  char prediction_time_out[3][2];
//  String in_prediction_URL;
  int attempt_connect;
  int out_stop_ID;
//  time_t out_last_update;
//  String out_prediction_URL;
  long last_attempt_in;
  long last_attempt_out;
  long last_refreshed_in;
  long last_refreshed_out;
} prediction;

#endif 
