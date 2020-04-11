#ifndef ESP_DISPLAY_HA_DEVICES_H
#define ESP_DISPLAY_HA_DEVICES_H

class ha_device {
public:
    explicit ha_device(const char *dname, const char* darea);

    void add_entity(ha_entity *entity);


    char name[50] = "";
    char area[50] = "";
    std::vector<ha_entity *> entities;
};

ha_device *new_device(const char *name, const char *area);


#endif //ESP_DISPLAY_HA_DEVICES_H
