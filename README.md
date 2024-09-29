# ESP32 LD2410 Radar Sensor with MQTT and Web Configuration

This project uses an ESP32 to interface with the LD2410 radar sensor for detecting motion and presence, and it publishes this data to Home Assistant via MQTT. Additionally, the ESP32 provides a web interface for configuring WiFi and MQTT settings, with persistent storage to retain the configuration across reboots.

## Features

- **LD2410 Radar Sensor Integration**: The ESP32 reads presence, stationary target, and moving target information from the LD2410 radar sensor.
- **WiFi Configuration via Web Interface**: If the ESP32 cannot connect to a pre-configured WiFi network, it will start in Access Point (AP) mode, allowing the user to connect and configure WiFi and MQTT settings through a web interface.
- **MQTT Integration**: Publishes sensor data to a specified MQTT broker for easy integration with home automation platforms like Home Assistant.
- **Persistent Storage**: Uses the ESP32's `Preferences` library to store and retrieve WiFi and MQTT credentials between reboots.
- **HTTP Server**: Hosts a web page for easy configuration of WiFi and MQTT settings.

## Hardware Setup

You will need the following components:

- ESP32 development board
- LD2410 radar sensor
- WiFi access point (router)
- MQTT broker (e.g., Home Assistant's Mosquitto add-on)

### Pin Connections

| LD2410 Pin | ESP32 Pin |
|------------|-----------|
| TX         | GPIO 17   |
| RX         | GPIO 16   |
| GND        | GND       |
| VCC        | 3.3V      |

Make sure the LD2410 radar sensor is correctly connected to the ESP32 before flashing the code.

## Software Setup

1. **Install Required Libraries**: Ensure you have the following libraries installed in your Arduino IDE or PlatformIO environment:
   - WiFi
   - PubSubClient (for MQTT communication)
   - Preferences (for saving WiFi/MQTT settings)
   - WebServer (for serving the configuration web page)

2. **Flashing the ESP32**: Upload the provided code to your ESP32 using Arduino IDE or PlatformIO.

3. **Initial Configuration (Access Point Mode)**:
   - On first boot, or if the WiFi credentials are incorrect, the ESP32 will enter AP mode and start a local network with the SSID `ESP32-Presence-AP` (default password: `123456789`).
   - Connect your computer or smartphone to this WiFi network.
   - Open a web browser and navigate to `192.168.4.1`. This will bring up a web page where you can enter your WiFi and MQTT credentials.

4. **Normal Operation**:
   - Once configured, the ESP32 will connect to the specified WiFi network and start publishing sensor data to the MQTT broker.

## Web Configuration Interface

The ESP32 serves a configuration page at the root (`/`) of the web server. The page includes fields for entering:

- WiFi SSID
- WiFi Password
- MQTT Server Address
- MQTT Username
- MQTT Password

After entering the details and saving, the ESP32 will automatically restart and apply the new settings.

## MQTT Topics

The ESP32 publishes sensor data to the following MQTT topics:

| Topic                                         | Payload                         | Description                                   |
|-----------------------------------------------|---------------------------------|-----------------------------------------------|
| `homeassistant/sensor/ld2410/presence`        | `"true"` or `"false"`           | Presence detection state (true = detected)    |
| `homeassistant/sensor/ld2410/stationary_target` | `"true"` or `"false"`           | Stationary target detection state             |
| `homeassistant/sensor/ld2410/stationary_distance` | Distance (float, meters)        | Distance to the stationary target (in meters) |
| `homeassistant/sensor/ld2410/stationary_energy`   | Energy level (float)            | Energy level of the stationary target         |
| `homeassistant/sensor/ld2410/moving_target`      | `"true"` or `"false"`           | Moving target detection state                 |
| `homeassistant/sensor/ld2410/moving_distance`    | Distance (float, meters)        | Distance to the moving target (in meters)     |
| `homeassistant/sensor/ld2410/moving_energy`      | Energy level (float)            | Energy level of the moving target             |
| `homeassistant/sensor/ld2410/firmware_version`   | Firmware version (string)       | Radar sensor firmware version                 |

## Example: Home Assistant Configuration

To integrate this project into Home Assistant, follow these steps:

1. **Install MQTT Integration in Home Assistant**:
   - Navigate to `Configuration` > `Devices & Services`.
   - Click on `+ Add Integration` and select `MQTT`.
   - Configure the MQTT broker (if not already set up).

2. **Add MQTT Sensors**:
   In Home Assistant, you can create MQTT-based sensors using the following YAML configuration in `configuration.yaml` or via the UI:

```yaml
sensor:
  - platform: mqtt
    name: "LD2410 Presence"
    state_topic: "homeassistant/sensor/ld2410/presence"
    payload_on: "true"
    payload_off: "false"
    device_class: motion
  - platform: mqtt
    name: "LD2410 Stationary Target"
    state_topic: "homeassistant/sensor/ld2410/stationary_target"
    payload_on: "true"
    payload_off: "false"
    device_class: motion
  - platform: mqtt
    name: "LD2410 Stationary Distance"
    state_topic: "homeassistant/sensor/ld2410/stationary_distance"
    unit_of_measurement: "m"
  - platform: mqtt
    name: "LD2410 Stationary Energy"
    state_topic: "homeassistant/sensor/ld2410/stationary_energy"
    unit_of_measurement: "J"
  - platform: mqtt
    name: "LD2410 Moving Target"
     state_topic: "homeassistant/sensor/ld2410/moving_target"
    payload_on: "true"
    payload_off: "false"
    device_class: motion
  - platform: mqtt
    name: "LD2410 Moving Distance"
    state_topic: "homeassistant/sensor/ld2410/moving_distance"
    unit_of_measurement: "m"
  - platform: mqtt
    name: "LD2410 Moving Energy"
    state_topic: "homeassistant/sensor/ld2410/moving_energy"
    unit_of_measurement: "J"
  - platform: mqtt
    name: "LD2410 Firmware Version"
    state_topic: "homeassistant/sensor/ld2410/firmware_version"
```

3. Restart Home Assistant: After adding the sensors, restart Home Assistant for the changes to take effect.

4. Verify Sensor Data: You should now see the sensor data from the LD2410 radar sensor available in the Home Assistant dashboard.

## Troubleshooting

* WiFi Connection Issues: If the ESP32 fails to connect to the WiFi network, it will automatically revert to Access Point mode to allow you to reconfigure the WiFi settings.
* MQTT Connection Issues: If the MQTT broker cannot be reached, the ESP32 will continue trying to reconnect every 5 seconds.
* Radar Not Detected: Ensure the radar sensor is properly connected to the correct GPIO pins on the ESP32 and that it is powered correctly.

## Future Improvements

- [ ] Add over-the-air (OTA) firmware updates.
- [ ] Extend support for additional sensors or functionality.
- [ ] Implement more advanced error handling and logging.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.