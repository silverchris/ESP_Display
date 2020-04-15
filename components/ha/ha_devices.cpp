#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <map>

#include <ArduinoJson.h>


#include "ha.hpp"
#include "ha_entities.h"
#include "ha_devices.h"


ha_device::ha_device(const char *dname, const char* darea) {
    strncpy(name, dname, sizeof(name)-1);
    strncpy(area, darea, sizeof(area)-1);
}

void ha_device::add_entity(ha_entity *entity) {
    entities.push_back(entity);
}



ha_device *new_device(const char *name, const char *area){
    auto *new_device = new ha_device(name, area);
    return new_device;
}



