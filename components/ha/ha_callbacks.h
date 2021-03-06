#ifndef CALLBACKS_HPP
#define CALLBACKS_HPP

void callback_state_events(const JsonDocument &json);

void callback_state_events_register(const JsonDocument &json);

void callback_state(const JsonDocument &json);

void callback_entities(const JsonDocument &json);
//
//void callback_devices(const JsonDocument &json);
//
//void callback_area(const JsonDocument &json);

void callback_auth();

#endif
