# ESP_Display

ESP_Display is a platform for interacting with Home Assistant from a ESP32+Touchscreen.

It is an attempt at a low power interface that can be cheaply deployed to control devices in a Home Assistant instance.

ESP_Display connects to Home Assistant using a websocket, the same way the Home Assistants lovelace ui does.
No additional configuration or modification of your home assistant configuration is needed.

### Currently supported features:
* Reading entities from Home Assistant
* Displaying current weather from a Home Assistant Weather Service
* Lights can be turned on and off and dimmed!
* Sensors that report a value between 0 and 100


### Currently unsupported features:
* Areas
* Groups
* Switches
* Every Other kind of sensor


ESP_Display is configured with a json file. An example is located in [/spiffs/config.json](/spiffs/config.json)

Currently the WiFi settings, Home Assistant Address, and Home Assistant API key is set at compile time