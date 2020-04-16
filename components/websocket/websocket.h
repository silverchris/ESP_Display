#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#ifdef __cplusplus

#include <ArduinoJson.h>
#include <freertos/ringbuf.h>


void ws_queue_add(JsonDocument &doc, void (*f)(const JsonDocument &json));

void ws_queue_add(JsonDocument &doc);

class CustomReader {
private:
    RingbufHandle_t buf;
    uint8_t next = 0;

public:
    int read();

    int peek();

    bool empty();

    explicit CustomReader(RingbufHandle_t buffer) {
        buf = buffer;
    }
};


extern "C" {
#endif


void websocket_init(void);

#ifdef __cplusplus
}
#endif


#endif