# ESP32 LD2410 Radar and BME680 Environmental Sensor with MQTT and Web Configuration

This project uses an **ESP32** to interface with the **LD2410 radar sensor** for detecting motion and presence, and with the **BME680 environmental sensor** to measure temperature, humidity, pressure, and gas resistance. It publishes this data to **Home Assistant** via **MQTT**. Additionally, the ESP32 provides a web interface for configuring WiFi, MQTT settings, and sensor pin assignments, with persistent storage to retain the configuration across reboots.

## Features

- **LD2410 Radar Sensor Integration**: Reads presence, stationary target, and moving target information.
- **BME680 Environmental Sensor Integration**: Measures temperature, humidity, pressure, and gas resistance.
- **Configurable Sensor Pins via Web Interface**: Define the pins connected to the LD2410 and BME680 sensors, with default values if not specified.
- **WiFi Configuration via Web Interface**: If unable to connect to WiFi, the ESP32 starts in Access Point mode for configuration.
- **MQTT Integration**: Publishes sensor data to a specified MQTT broker for integration with home automation platforms like Home Assistant.
- **Persistent Storage**: Uses the ESP32's `Preferences` library to store settings across reboots.
- **HTTP Server**: Hosts a web page for easy configuration of WiFi, MQTT settings, and sensor pins.

## Hardware Setup

### Required Components

- **ESP32 Development Board**
- **LD2410 Radar Sensor**
- **BME680 Environmental Sensor**
- **WiFi Access Point** (router)
- **MQTT Broker** (e.g., Home Assistant's Mosquitto add-on)

### Pin Connections

By default, the pins are connected as follows:

#### LD2410 Radar Sensor

| LD2410 Pin | ESP32 Default Pin |
|------------|-------------------|
| **TX**     | GPIO **17**       |
| **RX**     | GPIO **16**       |
| **GND**    | GND               |
| **VCC**    | 3.3V              |

#### BME680 Sensor

| BME680 Pin | ESP32 Default Pin |
|------------|-------------------|
| **SDA**    | GPIO **21**       |
| **SCL**    | GPIO **22**       |
| **GND**    | GND               |
| **VCC**    | 3.3V              |

> **Note:** You can change these pins via the web configuration interface.

### Wiring Diagram

Ensure all components are connected correctly according to the pin assignments. Use pull-up resistors for the I2C lines if necessary (most BME680 breakout boards include these).

## Software Setup

1. **Install Required Libraries**

   Ensure you have the following libraries installed in your Arduino IDE or PlatformIO environment:

   - **WiFi** (usually included with ESP32 support)
   - **PubSubClient** (for MQTT communication)
   - **Preferences** (for saving settings)
   - **WebServer** (for serving the web page)
   - **ld2410** (library for the LD2410 radar sensor)
   - **Wire** (for I2C communication)
   - **Adafruit_Sensor** (base class for sensor libraries)
   - **Adafruit_BME680** (for the BME680 sensor)

   You can install these libraries via the Arduino Library Manager or by adding them to your `platformio.ini` file if using PlatformIO.

2. **Configure the Code**

   - **Adjust Default Pin Settings (Optional):** If you wish to change the default pins before flashing, modify the following lines in the code:

     ```cpp
     int radar_rx_pin = 16;
     int radar_tx_pin = 17;
     int bme680_sda_pin = 21;
     int bme680_scl_pin = 22;
     ```

   - **Set WiFi and MQTT Credentials (Optional):** You can predefine your WiFi and MQTT settings in the code, but it's recommended to use the web interface.

3. **Flashing the ESP32**

   Upload the provided code to your ESP32 using the Arduino IDE or PlatformIO.

4. **Initial Configuration (Access Point Mode)**

   - On first boot, or if the WiFi credentials are incorrect, the ESP32 will enter **AP mode** and create a WiFi network named `ESP32-Presence-AP` (default password: `123456789`).
   - Connect your computer or smartphone to this WiFi network.
   - Open a web browser and navigate to `http://192.168.4.1`.
   - Fill in your WiFi and MQTT credentials, and specify the sensor pins if different from the defaults.
   - Save the configuration; the ESP32 will restart and attempt to connect to your WiFi network.

5. **Normal Operation**

   - After configuration, the ESP32 will connect to your WiFi network.
   - It will initialize the LD2410 radar sensor and the BME680 sensor using the specified pins.
   - Sensor data will be published to the MQTT broker at regular intervals.

## Web Configuration Interface

The ESP32 hosts a configuration page at the root (`/`) of the web server. Access it by entering the ESP32's IP address in a web browser.

### Configuration Options

- **WiFi Settings**
  - **SSID**: Your WiFi network name.
  - **Password**: Your WiFi network password.
- **MQTT Settings**
  - **MQTT Server**: IP address or hostname of your MQTT broker.
  - **MQTT Username**: MQTT username (if required).
  - **MQTT Password**: MQTT password (if required).
- **Sensor Pins**
  - **Radar RX Pin**: ESP32 pin connected to LD2410 TX.
  - **Radar TX Pin**: ESP32 pin connected to LD2410 RX.
  - **BME680 SDA Pin**: ESP32 SDA pin for I2C communication.
  - **BME680 SCL Pin**: ESP32 SCL pin for I2C communication.

> **Defaults** are used if no values are specified for the pins.

### Saving Configuration

- After entering your settings, click the **"Save"** button.
- The ESP32 will store the settings in non-volatile memory.
- The device will automatically restart to apply the new settings.

## MQTT Topics

The ESP32 publishes sensor data to the following MQTT topics:

### LD2410 Radar Sensor Topics

| Topic                                            | Payload                         | Description                                   |
|--------------------------------------------------|---------------------------------|-----------------------------------------------|
| `homeassistant/sensor/ld2410/presence`           | `"true"` or `"false"`           | Presence detection state                      |
| `homeassistant/sensor/ld2410/stationary_target`  | `"true"` or `"false"`           | Stationary target detection state             |
| `homeassistant/sensor/ld2410/stationary_distance`| Distance (float, meters)        | Distance to stationary target                 |
| `homeassistant/sensor/ld2410/stationary_energy`  | Energy level (float)            | Energy level of stationary target             |
| `homeassistant/sensor/ld2410/moving_target`      | `"true"` or `"false"`           | Moving target detection state                 |
| `homeassistant/sensor/ld2410/moving_distance`    | Distance (float, meters)        | Distance to moving target                     |
| `homeassistant/sensor/ld2410/moving_energy`      | Energy level (float)            | Energy level of moving target                 |
| `homeassistant/sensor/ld2410/firmware_version`   | Firmware version (string)       | Radar sensor firmware version                 |

### BME680 Environmental Sensor Topics

| Topic                                         | Payload                         | Description                                   |
|-----------------------------------------------|---------------------------------|-----------------------------------------------|
| `homeassistant/sensor/bme680/temperature`     | Temperature (float, °C)         | Ambient temperature                           |
| `homeassistant/sensor/bme680/humidity`        | Humidity (float, %)             | Ambient humidity                              |
| `homeassistant/sensor/bme680/pressure`        | Pressure (float, hPa)           | Atmospheric pressure                          |
| `homeassistant/sensor/bme680/gas`             | Gas resistance (float, kΩ)      | Gas resistance (indicative of air quality)    |

## Example: Home Assistant Configuration

Integrate the sensors into Home Assistant by adding the following configuration:

### 1. Install MQTT Integration

- Navigate to **Configuration** > **Devices & Services**.
- Click on **"+ Add Integration"** and select **MQTT**.
- Follow the prompts to set up the MQTT broker.

### 2. Add MQTT Sensors

Add the following to your `configuration.yaml`:

```yaml
sensor:
  # LD2410 Radar Sensor
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
    device_class: distance

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
    device_class: distance

  - platform: mqtt
    name: "LD2410 Moving Energy"
    state_topic: "homeassistant/sensor/ld2410/moving_energy"
    unit_of_measurement: "J"

  - platform: mqtt
    name: "LD2410 Firmware Version"
    state_topic: "homeassistant/sensor/ld2410/firmware_version"

  # BME680 Environmental Sensor
  - platform: mqtt
    name: "BME680 Temperature"
    state_topic: "homeassistant/sensor/bme680/temperature"
    unit_of_measurement: "°C"
    device_class: temperature

  - platform: mqtt
    name: "BME680 Humidity"
    state_topic: "homeassistant/sensor/bme680/humidity"
    unit_of_measurement: "%"
    device_class: humidity

  - platform: mqtt
    name: "BME680 Pressure"
    state_topic: "homeassistant/sensor/bme680/pressure"
    unit_of_measurement: "hPa"
    device_class: pressure

  - platform: mqtt
    name: "BME680 Gas Resistance"
    state_topic: "homeassistant/sensor/bme680/gas"
    unit_of_measurement: "kΩ"
```

### 3. Restart Home Assistant

- After saving the configuration, restart Home Assistant to apply the changes.

### 4. Verify Sensor Data

- Navigate to **Overview** in Home Assistant.
- Add the new sensors to your dashboard to monitor real-time data.

## Troubleshooting

- **WiFi Connection Issues**: If the ESP32 cannot connect to the WiFi network, it will enter AP mode for reconfiguration.
- **MQTT Connection Issues**: Ensure the MQTT broker address and credentials are correct.
- **Radar Not Detected**: Verify the LD2410 is connected to the correct pins and is powered properly.
- **BME680 Not Detected**: Verify the BME680 is connected correctly. Ensure the I2C pins are correct and that the sensor is powered.
- **Accessing Web Interface After Initial Setup**: To access the web interface after initial setup, connect to the ESP32's IP address on your network. You can find the IP address via your router's connected devices list or by using a network scanning tool.

## Reconfiguring Settings After Initial Setup

To change settings after the initial configuration:

1. **Find the ESP32's IP Address**

   - **Via Serial Monitor**: Connect the ESP32 to your computer and open the Serial Monitor at 115200 baud to view the IP address.
   - **Via Router**: Access your router's admin interface to find the device in the connected devices list.
   - **Network Scanner**: Use a network scanning app (e.g., Fing) to detect devices on your network.

2. **Access the Web Interface**

   - Open a web browser and navigate to `http://<ESP32_IP_ADDRESS>/`.
   - The configuration page will load, allowing you to adjust settings.

3. **Make Changes and Save**

   - Update any settings as needed.
   - Click **"Save"** to store the new configuration.
   - The ESP32 will restart to apply changes.

## Future Improvements

- [ ] **Over-the-Air (OTA) Updates**: Implement OTA updates for easier firmware upgrades.
- [ ] **Additional Sensors**: Extend support for more sensors and functionalities.
- [ ] **Advanced Error Handling**: Improve logging and error messages for troubleshooting.
- [ ] **Secure Web Interface**: Add authentication to the web interface for enhanced security.
- [ ] **Custom MQTT Topics**: Allow users to define custom MQTT topics via the web interface.

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for more details.

---

By integrating both the LD2410 radar sensor and the BME680 environmental sensor, this project provides comprehensive environmental monitoring capabilities suitable for home automation and IoT applications. The web interface simplifies configuration, making it accessible even for those new to ESP32 development.

If you encounter any issues or have suggestions for improvements, feel free to open an issue or submit a pull request on the project's repository.

Happy building!