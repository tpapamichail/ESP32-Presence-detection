#include <WiFi.h>
#include <PubSubClient.h>
#include <ld2410.h>
#include <Preferences.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define SEND_EVERY_MS 1000

Preferences preferences;
WebServer server(80);

char ssid[32];
char password[32];
char mqtt_server[16];
char mqtt_user[32];
char mqtt_pass[32];

WiFiClient espClient;
PubSubClient client(espClient);
ld2410 radar;

uint32_t lastReading = 0;
bool radarConnected = false;
bool ap_mode = false;

// Pin configurations
int radar_rx_pin = 16;
int radar_tx_pin = 17;
int bme680_sda_pin = 21;
int bme680_scl_pin = 22;

Adafruit_BME680 bme;
bool bme680Connected = false;

// Variables to store previous states
bool lastPresenceDetected = false;
bool lastStationaryTargetDetected = false;
float lastStationaryDistance = 0;
float lastStationaryEnergy = 0;
bool lastMovingTargetDetected = false;
float lastMovingDistance = 0;
float lastMovingEnergy = 0;
int send_interval = 1000; // Default value


void setup_wifi() {
  delay(10);
  MONITOR_SERIAL.println();
  MONITOR_SERIAL.print("Connecting to ");
  MONITOR_SERIAL.println(ssid);
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    MONITOR_SERIAL.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    MONITOR_SERIAL.println("\nWiFi connection failed. Starting Access Point.");
    ap_mode = true;
    WiFi.softAP("ESP32-Presence-AP", "123456789");
    MONITOR_SERIAL.print("Access Point IP address: ");
    MONITOR_SERIAL.println(WiFi.softAPIP());
  } else {
    MONITOR_SERIAL.println("\nWiFi connected");
    ap_mode = false;
  }
}

void reconnect() {
  while (!client.connected()) {
    MONITOR_SERIAL.print("Attempting MQTT connection...");

    bool connected = (strlen(mqtt_user) > 0 && strlen(mqtt_pass) > 0)
                      ? client.connect("ESP32Client", mqtt_user, mqtt_pass)
                      : client.connect("ESP32Client");

    if (connected) {
      MONITOR_SERIAL.println("connected");
    } else {
      MONITOR_SERIAL.print("failed, rc=");
      MONITOR_SERIAL.print(client.state());
      MONITOR_SERIAL.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishToMQTT(const char* topic, const char* message) {
  client.publish(topic, message);
}

void handleRoot() {
  String html = R"rawliteral(
    <html>
    <head><style>
      body { background-color: #f7fafc; display: flex; justify-content: center; align-items: center; height: 100vh; font-family: Arial, sans-serif; }
      div { background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1); max-width: 400px; width: 100%%; }
      h1 { font-size: 24px; color: #4a5568; margin-bottom: 20px; text-align: center; }
      label { display: block; color: #4a5568; font-weight: bold; margin-bottom: 5px; }
      input[type='text'], input[type='password'], input[type='number'] { width: 100%%; padding: 10px; border: 1px solid #cbd5e0; border-radius: 5px; margin-bottom: 15px; box-sizing: border-box; }
      input[type='submit'] { background-color: #4a90e2; color: white; padding: 10px 15px; border: none; border-radius: 5px; cursor: pointer; width: 100%%; }
      input[type='submit']:hover { background-color: #357ab8; }
    </style></head>
    <body>
      <div>
        <h1>ESP32 Presence Configuration</h1>
        <form action='/save' method='POST'>
          <label>SSID</label><input type='text' name='ssid' value='%s'>
          <label>Password</label><input type='password' name='password' value='%s'>
          <label>MQTT Server</label><input type='text' name='mqtt_server' value='%s'>
          <label>MQTT Username</label><input type='text' name='mqtt_user' value='%s'>
          <label>MQTT Password</label><input type='text' name='mqtt_pass' value='%s'>
          <label>Radar RX Pin</label><input type='number' name='radar_rx_pin' value='%d'>
          <label>Radar TX Pin</label><input type='number' name='radar_tx_pin' value='%d'>
          <label>BME680 SDA Pin</label><input type='number' name='bme680_sda_pin' value='%d'>
          <label>BME680 SCL Pin</label><input type='number' name='bme680_scl_pin' value='%d'>
          <label>Send Interval (ms)</label><input type='number' name='send_interval' value='%d'>
          <input type='submit' value='Save'>
        </form>
      </div>
    </body>
    </html>)rawliteral";


  char page[2048];
  snprintf(page, sizeof(page), html.c_str(), ssid, password, mqtt_server, mqtt_user, mqtt_pass,
         radar_rx_pin, radar_tx_pin, bme680_sda_pin, bme680_scl_pin, send_interval);

  server.send(200, "text/html", page);
}

void handleSave() {
  preferences.putString("ssid", server.arg("ssid"));
  preferences.putString("password", server.arg("password"));
  preferences.putString("mqtt_server", server.arg("mqtt_server"));
  preferences.putString("mqtt_user", server.arg("mqtt_user"));
  preferences.putString("mqtt_pass", server.arg("mqtt_pass"));
  preferences.putInt("radar_rx_pin", server.arg("radar_rx_pin").toInt());
  preferences.putInt("radar_tx_pin", server.arg("radar_tx_pin").toInt());
  preferences.putInt("bme680_sda_pin", server.arg("bme680_sda_pin").toInt());
  preferences.putInt("bme680_scl_pin", server.arg("bme680_scl_pin").toInt());
  preferences.putInt("send_interval", server.arg("send_interval").toInt());

  strcpy(ssid, preferences.getString("ssid").c_str());
  strcpy(password, preferences.getString("password").c_str());
  strcpy(mqtt_server, preferences.getString("mqtt_server").c_str());
  strcpy(mqtt_user, preferences.getString("mqtt_user").c_str());
  strcpy(mqtt_pass, preferences.getString("mqtt_pass").c_str());
  radar_rx_pin = preferences.getInt("radar_rx_pin", 16);
  radar_tx_pin = preferences.getInt("radar_tx_pin", 17);
  bme680_sda_pin = preferences.getInt("bme680_sda_pin", 21);
  bme680_scl_pin = preferences.getInt("bme680_scl_pin", 22);
  send_interval = preferences.getInt("send_interval", 1000);

  server.send(200, "text/html", "<html><body><h1>Configuration Saved!</h1><a href='/'>Go back</a></body></html>");
  delay(2000);
  ESP.restart();
}

void setup(void) {
  MONITOR_SERIAL.begin(115200);
  radar.debug(MONITOR_SERIAL);

  preferences.begin("config", false);
  strcpy(ssid, preferences.getString("ssid", "my_ssid").c_str());
  strcpy(password, preferences.getString("password", "123456789").c_str());
  strcpy(mqtt_server, preferences.getString("mqtt_server", "").c_str());
  strcpy(mqtt_user, preferences.getString("mqtt_user", "").c_str());
  strcpy(mqtt_pass, preferences.getString("mqtt_pass", "").c_str());
  radar_rx_pin = preferences.getInt("radar_rx_pin", 16);
  radar_tx_pin = preferences.getInt("radar_tx_pin", 17);
  bme680_sda_pin = preferences.getInt("bme680_sda_pin", 21);
  bme680_scl_pin = preferences.getInt("bme680_scl_pin", 22);
  send_interval = preferences.getInt("send_interval", 1000);

  setup_wifi();

  if (!ap_mode) {
    client.setServer(mqtt_server, 1883);
  }

  #if defined(ESP32)
    RADAR_SERIAL.begin(256000, SERIAL_8N1, radar_rx_pin, radar_tx_pin);
  #endif
  delay(500);

  MONITOR_SERIAL.println("\nLD2410 radar sensor initialising...");
  radarConnected = radar.begin(RADAR_SERIAL);
  MONITOR_SERIAL.println(radarConnected ? "Radar connected" : "Radar not connected");

  radar.requestCurrentConfiguration();

  Wire.begin(bme680_sda_pin, bme680_scl_pin);
  if (!bme.begin()) {
    MONITOR_SERIAL.println("Could not find a valid BME680 sensor, check wiring!");
    bme680Connected = false;
  } else {
    MONITOR_SERIAL.println("BME680 sensor initialized.");
    bme680Connected = true;
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms
  }

  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
  MONITOR_SERIAL.println("HTTP server started");

  if (ap_mode) {
    MONITOR_SERIAL.println("Waiting for WiFi configuration...");
    while (true) {
      server.handleClient();
    }
  }
}

void loop() {
  if (!ap_mode && !client.connected()) {
    reconnect();
  }

  client.loop();
  server.handleClient();
  radar.read();

  if (radar.isConnected() && millis() - lastReading > send_interval) {
    lastReading = millis();

    if (!ap_mode) {
      // Presence detection
      bool presenceDetected = radar.presenceDetected();
      if (presenceDetected != lastPresenceDetected) {
        publishToMQTT("homeassistant/sensor/ld2410/presence", presenceDetected ? "true" : "false");
        lastPresenceDetected = presenceDetected;
      }

      // Stationary target detection
      bool stationaryTargetDetected = radar.stationaryTargetDetected();
      float stationaryDistance = radar.stationaryTargetDistance();
      float stationaryEnergy = radar.stationaryTargetEnergy();

      if (stationaryTargetDetected != lastStationaryTargetDetected) {
        publishToMQTT("homeassistant/sensor/ld2410/stationary_target", stationaryTargetDetected ? "true" : "false");
        lastStationaryTargetDetected = stationaryTargetDetected;
      }

      if (stationaryTargetDetected && (stationaryDistance != lastStationaryDistance || stationaryEnergy != lastStationaryEnergy)) {
        publishToMQTT("homeassistant/sensor/ld2410/stationary_distance", String(stationaryDistance).c_str());
        publishToMQTT("homeassistant/sensor/ld2410/stationary_energy", String(stationaryEnergy).c_str());
        lastStationaryDistance = stationaryDistance;
        lastStationaryEnergy = stationaryEnergy;
      }

      // Moving target detection
      bool movingTargetDetected = radar.movingTargetDetected();
      float movingDistance = radar.movingTargetDistance();
      float movingEnergy = radar.movingTargetEnergy();

      if (movingTargetDetected != lastMovingTargetDetected) {
        publishToMQTT("homeassistant/sensor/ld2410/moving_target", movingTargetDetected ? "true" : "false");
        lastMovingTargetDetected = movingTargetDetected;
      }

      if (movingTargetDetected && (movingDistance != lastMovingDistance || movingEnergy != lastMovingEnergy)) {
        publishToMQTT("homeassistant/sensor/ld2410/moving_distance", String(movingDistance).c_str());
        publishToMQTT("homeassistant/sensor/ld2410/moving_energy", String(movingEnergy).c_str());
        lastMovingDistance = movingDistance;
        lastMovingEnergy = movingEnergy;
      }

      // Firmware version
      static String lastFirmwareVersion = "";
      String currentFirmwareVersion = String(radar.firmware_major_version) + "." +
                                      String(radar.firmware_minor_version) + "." +
                                      String(radar.firmware_bugfix_version, HEX);
      if (lastFirmwareVersion != currentFirmwareVersion) {
        publishToMQTT("homeassistant/sensor/ld2410/firmware_version", currentFirmwareVersion.c_str());
        lastFirmwareVersion = currentFirmwareVersion;
      }

      // BME680 Sensor Data
      if (bme680Connected) {
        if (bme.performReading()) {
          float temperature = bme.temperature;
          float pressure = bme.pressure / 100.0; // hPa
          float humidity = bme.humidity;
          float gas = bme.gas_resistance / 1000.0; // KiloOhms

          publishToMQTT("homeassistant/sensor/bme680/temperature", String(temperature).c_str());
          publishToMQTT("homeassistant/sensor/bme680/humidity", String(humidity).c_str());
          publishToMQTT("homeassistant/sensor/bme680/pressure", String(pressure).c_str());
          publishToMQTT("homeassistant/sensor/bme680/gas", String(gas).c_str());
        } else {
          MONITOR_SERIAL.println("Failed to perform reading from BME680 sensor");
        }
      }
    }
  }
}
