#include <cstdio>
#include <map>
#include <esp_event.h>

#include <ArduinoJson.h>

#include "websocket.h"


#include "ha.h"
#include "ha_entities.h"
#include "ha_event.h"
#include "ha_devices.h"
#include "ha_areas.h"
#include "ha_callbacks.h"


std::map<const char *, ha_device *, StrCompare> ha_devices;

//std::map<const char *, ha_area *, StrCompare> ha_areas;

std::map<const char *, ha_entity *, StrCompare> ha_entities;


ha_entity *add_entity(const char *entity_id) {
    ha_entity *entity = nullptr;

    char name[] = "null";
    size_t id_len = 50;
    char *id = (char *) calloc(sizeof(char), id_len);
    strncpy(id, entity_id, id_len-1);

    entity = new_entity(id, name);
    if (entity != nullptr) {
        ha_entities.emplace(id, entity);
    } else {
        free(id);
    }
    return entity;
}

void update_entity(const char *entity_id, JsonObjectConst &doc) {
    if (ha_entities.count(entity_id)) {
        ha_entities[entity_id]->update(doc);
    }
}

ha_entity *get_entity(const char *entity_id) {
    if (ha_entities.count(entity_id)) {
        ha_entity *entity = nullptr;
        entity = ha_entities[entity_id];
        return entity;
    } else {
        char buf[2048];
        if(get_state(entity_id, buf) == 200){
            return add_entity(entity_id);
        }
    }
    return nullptr;
}

void add_device(const char *id, const char *name, const char *area) {
    ha_device *device = nullptr;

    device = new_device(name, area);
    if (device != nullptr) {
        ha_devices.emplace(id, device);
    }
}


ha_device *get_device(const char *device_id){
    if (ha_devices.count(device_id)) {
        ha_device *device = nullptr;
        device = ha_devices[device_id];
        return device;
    }
    return nullptr;
}

void device_entity_assoc(const char * deviceid, const char *entityid){
    if(ha_entities.count(entityid) > 0 && ha_devices.count(deviceid)){
        ha_devices[deviceid]->add_entity(ha_entities[entityid]);
    }
}

//void create_area(char *id, char *name) {
//    auto *area = new ha_area;
//    strncpy(area->name, name, 50);
//    ha_areas.emplace(id, area);
//}
//
//void destroy_area(char *id) {
//    if (ha_areas[id]->devices.empty()) {
//        free(ha_areas[id]);
//        ha_areas.erase(id);
//    }
//}



int ha_state = ha_state_auth;
int32_t ha_state_time;
esp_timer_handle_t ha_timer;

void ha_state_machine(void *args) {
    StaticJsonDocument<100> doc_out;

    if (ha_state_time == 0) {
        switch (ha_state) {
            case ha_state_auth:
                break;
            case ha_state_entities:
                doc_out["type"] = "config/entity_registry/list";
                ws_queue_add(doc_out, callback_entities);
                break;
            case ha_state_subscribe:
                doc_out["type"] = "subscribe_events";
                doc_out["event_type"] = "state_changed";
                ws_queue_add(doc_out, callback_state_events_register);
                break;
            case ha_state_devices:
                doc_out["type"] = "config/device_registry/list";
//                ws_queue_add(doc_out, callback_devices);
                break;
            case ha_state_areas:
                doc_out["type"] = "config/area_registry/list";
//                ws_queue_add(doc_out, callback_areas);
                break;
            case ha_state_finished:
                esp_event_post_to(ha_event_loop_hdl, ESP_HA_EVENT, HA_EVENT_READY, nullptr, 0, 100);
                esp_timer_stop(ha_timer);
                break;
            default:
                break;
        }
        ha_state_time = esp_timer_get_time() / 1000ULL;
    }
}

void ha_state_set(int new_state) {
    ha_state = new_state;
    ha_state_time = 0;
    printf("state advanced %i\n", ha_state);
}

void ha_init() {
    const esp_timer_create_args_t ha_timer_args = {
            .callback = &ha_state_machine,
            .arg = nullptr,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "ha_task"
    };
    ESP_ERROR_CHECK(esp_timer_create(&ha_timer_args, &ha_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(ha_timer, 500 * 1000));
}