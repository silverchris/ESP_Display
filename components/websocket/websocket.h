#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#ifdef __cplusplus

#include <deque>

#include <ArduinoJson.h>

void ws_queue_add(JsonDocument &doc, void (*f)(const JsonDocument &json));

void ws_queue_add(JsonDocument &doc);

class CustomReader {
private:
    std::deque<int> buf;
    SemaphoreHandle_t lock = xSemaphoreCreateMutex();

public:
    int read();

    int peek();

    void pop();

    void fill(char *buffer, size_t length);

    void putc(int character);

    bool empty();
};


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