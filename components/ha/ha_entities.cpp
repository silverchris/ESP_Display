#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "esp_http_client.h"

#include <ArduinoJson.h>

#include "ha.h"
#include "ha_event.h"
#include "websocket.h"

#include "ha_entities.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define TAG "HA_ENTITY"

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    static int output_len;       // Stores number of bytes read
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    memcpy((char *) evt->user_data + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}


int get_state(const char *entity, char *out) {
    char url[100] = "http://" CONFIG_HA_ADDRESS ":8123/api/states/";
    strncat(url, entity, sizeof(url) - strlen(url) - 1);
    printf("url %s\n", url);
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    esp_http_client_config_t config = {
            .url = url,
            .port = 8123,
            .event_handler = _http_event_handler,
            .user_data = local_response_buffer,        // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Authorization", "Bearer " CONFIG_HA_KEY);
    esp_http_client_set_header(client, "Content-Type", "application/json");


    // GET
    int err = esp_http_client_perform(client);
    int status;
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "HTTP GET Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
        status = esp_http_client_get_status_code(client);
        memcpy(out, local_response_buffer, MAX_HTTP_OUTPUT_BUFFER);
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    if (err == ESP_OK) {
        return status;
    }
    return err;
}

ha_entity::ha_entity(const char *entity_id, const char *dname) {
    strncpy(id, entity_id, sizeof(id) - 1);
    if (dname != nullptr) {
        strncpy(name, dname, sizeof(name) - 1);
    }

    if (strncmp(entity_id, "light.", 5) == 0) {
        type = ha_entity_type::ha_light;
    } else if (strncmp(entity_id, "switch.", 7) == 0) {
        type = ha_entity_type::ha_switch;
    } else if (strncmp(entity_id, "binary_sensor.", 14) == 0) {
        type = ha_entity_type::ha_binary_sensor;
    } else if (strncmp(entity_id, "weather.", 8) == 0) {
        type = ha_entity_type::ha_weather;
    } else if (strncmp(entity_id, "sensor.", 7) == 0) {
        type = ha_entity_type::ha_sensor;
    }

}

void ha_entity::update(JsonObjectConst &doc) {
    supported_features = doc["attributes"]["supported_features"];
    for (void *call : callbacks) {
        ha_event_data out = {.ptr = call};
        ha_event_post(HA_EVENT_UPDATE, &out, sizeof(ha_event_data), 100);
    }
}

void ha_entity_switch::toggle() {
}

int ha_entity_switch::getState() {
    return state;
}

void ha_entity_switch::update(JsonObjectConst &doc) {
    state = static_cast<uint8_t>(strncmp(doc["state"], "on", 2) == 0);
    ha_entity::update(doc);
}

int ha_entity_light::getState() {
    return state;
}

void ha_entity_light::dim(uint8_t dim_value) {
    StaticJsonDocument<300> doc_out;
    doc_out["type"] = "call_service";
    doc_out["domain"] = "light";
    doc_out["service"] = "turn_on";
    doc_out["service_data"]["entity_id"] = id;
    doc_out["service_data"]["transition"] = 1;
    doc_out["service_data"]["brightness"] = (uint8_t) dim_value;
    ws_queue_add(doc_out);
}

void ha_entity_light::toggle() {
    StaticJsonDocument<300> doc_out;
    doc_out["type"] = "call_service";
    doc_out["domain"] = "light";
    doc_out["service"] = "toggle";
    doc_out["service_data"]["entity_id"] = id;
    ws_queue_add(doc_out);
}

void ha_entity_light::features(void *feature_struct_ptr) {
    auto fstruct = (ha_light_features *) feature_struct_ptr;
    printf("supports brightness %i\n", supported_features & HA_LIGHT_SUPPORT_BRIGHTNESS);
    fstruct->SUPPORT_BRIGHTNESS = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_BRIGHTNESS);
    fstruct->SUPPORT_COLOR_TEMP = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_COLOR_TEMP);
    fstruct->SUPPORT_EFFECT = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_EFFECT);
    fstruct->SUPPORT_FLASH = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_FLASH);
    fstruct->SUPPORT_COLOR = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_COLOR);
    fstruct->SUPPORT_TRANSITION = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_TRANSITION);
    fstruct->SUPPORT_WHITE_VALUE = static_cast<bool>(supported_features & HA_LIGHT_SUPPORT_WHITE_VALUE);
}


void ha_entity_light::update(JsonObjectConst &doc) {
    state = strncmp(doc["state"], "on", 2) == 0;
    brightness = doc["attributes"]["brightness"];
    color_temp = doc["attributes"]["color_temp"];
    min_mireds = doc["attributes"]["min_mireds"];
    min_mireds = doc["attributes"]["min_mireds"];
    white_value = doc["attributes"]["white_value"];
    ha_entity::update(doc);
}

void ha_entity_weather::update(JsonObjectConst &doc) {
    temperature = doc["attributes"]["temperature"];
    pressure = doc["attributes"]["pressure"];
    humidity = doc["attributes"]["humidity"];
    visibility = doc["attributes"]["visibility"];
    wind_speed = doc["attributes"]["wind_speed"];
    wind_bearing = doc["attributes"]["wind_bearing"];
    strncpy(friendly_name, (const char *) doc["attributes"]["friendly_name"], sizeof(friendly_name) - 1);

    ha_entity::update(doc);
}

int ha_entity_weather::getState() {
    return (int) temperature;
}

void ha_entity_sensor::update(JsonObjectConst &doc) {
    if (doc.containsKey("state")) {
        strncpy(state, (const char *) doc["state"], sizeof(state) - 1);
    }

    if (doc["attributes"].containsKey("unit_of_measurement")) {
        strncpy(unit_of_measurement, (const char *) doc["attributes"]["unit_of_measurement"],
                sizeof(unit_of_measurement) - 1);
    }
    ha_entity::update(doc);
}

int ha_entity_sensor::getState() {
    return strtol(state, nullptr, 10);
}


float ha_entity_sensor::getStateAsFloat() {
    return 0.00;
}

char *ha_entity_sensor::getStateAsString() {
    return state;
}

class SwitchEntityFactory : public EntityFactory {
    ha_entity *create(const char *entity_id, const char *dname) override {
        return new ha_entity_switch(entity_id, dname);
    }
};

class LightEntityFactory : public EntityFactory {
    ha_entity *create(const char *entity_id, const char *dname) override {
        return new ha_entity_light(entity_id, dname);
    }
};

class WeatherEntityFactory : public EntityFactory {
    ha_entity *create(const char *entity_id, const char *dname) override {
        return new ha_entity_weather(entity_id, dname);
    }
};

class SensorEntityFactory : public EntityFactory {
    ha_entity *create(const char *entity_id, const char *dname) override {
        return new ha_entity_sensor(entity_id, dname);
    }
};

typedef std::map<const char *, EntityFactory *, StrCompare> EntitybyName;

ha_entity *new_entity(const char *entity_id, const char *dname) {
    EntitybyName entity_by_name;
    entity_by_name["switch"] = new SwitchEntityFactory();
    entity_by_name["light"] = new LightEntityFactory();
    entity_by_name["weather"] = new WeatherEntityFactory();
    entity_by_name["sensor"] = new SensorEntityFactory();

    char entity[20];
    strncpy(entity, entity_id, sizeof(entity) - 1);
    strtok(entity, ".");

    if (entity_by_name.count(entity) > 0) {
        EntityFactory *factory = entity_by_name[entity];
        ha_entity *e = factory->create(entity_id, dname);

        char *buf = (char *) calloc(sizeof(char), MAX_HTTP_OUTPUT_BUFFER);
        get_state(entity_id, buf);
        DynamicJsonDocument doc(3000);
        DeserializationError error = deserializeJson(doc, buf);
        if (error) {
            ESP_LOGE(TAG, "deserializeJson() failed: %s\n", error.c_str());
        } else {
            JsonObjectConst state = doc.as<JsonObject>();
            e->update(state);
        }
        free(buf);
        return e;
    }
    return nullptr;
}