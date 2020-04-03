#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#ifdef __cplusplus

#include <ArduinoJson.h>

void ws_queue_add(JsonDocument &doc, void (*f)(const JsonDocument &json));

void ws_queue_add(JsonDocument &doc);


extern "C" {
#endif

extern bool ha_ready;

void websocket_init(void);
//void websocket_task(void);


//int websocket(void);
//
//void websocket_thread_func(void);

#ifdef __cplusplus
}
#endif


#endif