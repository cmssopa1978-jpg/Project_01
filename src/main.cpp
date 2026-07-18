#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ========== OpenWeather API Configuration ==========
#define OPENWEATHER_API_KEY "03169a21d57223736ed597768ebad921"  // Set your OpenWeather API key (see Telegram.md for secure storage suggestions)
#define CITY_NAME "Chiang Mai"
#define CITY_LAT 18.7883
#define CITY_LON 98.9853
#define WEATHER_UPDATE_INTERVAL 60000   // 1 minute in milliseconds
#define OPENWEATHER_WEATHER_URL "http://api.openweathermap.org/data/2.5/weather"
#define OPENWEATHER_AQI_URL "http://api.openweathermap.org/data/2.5/air_pollution"

// ========== Telegram Bot Configuration ==========
#define TELEGRAM_BOT_TOKEN "8828476817:AAFAweVGNF6Th3EIQjbuILQAgTTpW8Ug8Lo"    // Replace with Bot Token from @BotFather or set via external config
#define TELEGRAM_CHAT_ID "7861019137"    // Replace with numeric chat_id (use Telegram.md to obtain)
#define TELEGRAM_API_URL "https://api.telegram.org/bot"  // base URL without a trailing slash

// ========== OLED Display Configuration ==========
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C
#define OLED_REFRESH_INTERVAL 1000

// ========== WiFi Configuration ==========
#define WIFI_RESET_HOLD_TIME 5000  // 5 seconds to reset WiFi
#define AP_SSID "ESP32_CONFIG"     // Access Point SSID
#define AP_PASSWORD "12345678"     // Access Point Password

// ========== GPIO Pin Definitions ==========
// Relay Pins (Active Low: LOW=ON, HIGH=OFF)
#define RELAY1_PIN 17
#define RELAY2_PIN 16
#define RELAY3_PIN 4

// Switch Pins (Active Low: LOW=Pressed, HIGH=Released)
#define SWITCH1_PIN 34
#define SWITCH2_PIN 35
#define SWITCH3_PIN 32

// ========== Debouncing & Timing Constants ==========
#define DEBOUNCE_DELAY 20        // 20 ms debounce delay
#define SWITCH_POLL_INTERVAL 50  // 50 ms switch polling interval
#define WIFI_STATUS_CHECK_INTERVAL 60000  // 60 seconds WiFi status check
unsigned long lastSwitchCheckTime = 0;
unsigned long lastWiFiCheckTime = 0;
unsigned long lastWeatherUpdateTime = 0;
unsigned long lastOledRefreshTime = 0;

// ========== Weather Data Variables ==========
struct WeatherData {
  float temperature = 0.0;
  float feels_like = 0.0;
  float humidity = 0.0;
  float pressure = 0.0;
  String weather_description = "N/A";
  float wind_speed = 0.0;
  int aqi = 0;  // 1=Good, 2=Fair, 3=Moderate, 4=Poor, 5=Very Poor
  float pm25 = 0.0;
  float pm10 = 0.0;
  unsigned long last_update = 0;
} weatherData;

// ========== Relay State Variables ==========
bool relay1_state = false;  // false = OFF, true = ON
bool relay2_state = false;
bool relay3_state = false;

// ========== Switch State Variables (for edge detection) ==========
int sw1_prev_state = HIGH;
int sw2_prev_state = HIGH;
int sw3_prev_state = HIGH;

// ========== SW1 Long Press Variables ==========
unsigned long sw1_press_start_time = 0;
bool sw1_long_press_triggered = false;
int sw1_long_press_prev_state = HIGH;  // Separate for long press detection

// ========== WiFi Status Variable ==========
bool wifi_connected = false;
WiFiManager wifiManager;

// ========== OLED Display Variable ==========
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
bool oled_available = false;
char oled_status_line1[22] = "";
char oled_status_line2[22] = "";
bool oled_show_status = false;

// ========== Function Prototypes ==========
void toggleRelay(int relayPin, bool& relayState);
bool isSwitchPressedOnce(int switchPin, int& prevState);
void connectWiFi();
void printWiFiStatus();
void checkSW1LongPress();
void resetWiFiConfiguration();
void fetchWeatherData();
void fetchAirQualityData();
void printWeatherData();
bool isTelegramConfigured();
bool sendTelegramMessage(const String& message);
String urlEncode(const String& value);
String formatRelayTelegramMessage(const char* relayName, bool relayState);
String formatWeatherTelegramMessage();
void initOLED();
void updateOLED();
void showOLEDStatus(const char* title, const char* line1 = "", const char* line2 = "");
void clearOLEDStatus();
void checkStartupWiFiReset();
void drawRelayStatus(int x, int y, const char* label, bool state);
const char* getAQILabel(int aqi);

// ========== Setup ==========
void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(1000);
  
  // Set Relay Pins as OUTPUT
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  
  // Initialize all Relays to OFF (Active Low = HIGH = OFF)
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  
  // Set Switch Pins as INPUT (no internal pull-up needed, external pull-up present)
  pinMode(SWITCH1_PIN, INPUT);
  pinMode(SWITCH2_PIN, INPUT);
  pinMode(SWITCH3_PIN, INPUT);

  // Initialize OLED after GPIO setup so SW1 boot reset status can be shown.
  initOLED();
  showOLEDStatus("System Boot", "Hold SW1 5 sec", "Reset WiFi");
  delay(500);
  checkStartupWiFiReset();
  
  // Print initialization message
  Serial.println("\n========================================");
  Serial.println("   Relay & Switch Control System");
  Serial.println("========================================");
  Serial.println("Relay1 (GPIO17) - Toggle via SW1");
  Serial.println("Relay2 (GPIO16) - Toggle via SW2");
  Serial.println("Relay3 (GPIO4)  - Toggle via SW3");
  Serial.println("\nSwitch Configuration:");
  Serial.println("SW1 (GPIO34) - Active Low with Pull-up (Hold 5s to Reset WiFi)");
  Serial.println("SW2 (GPIO35) - Active Low with Pull-up");
  Serial.println("SW3 (GPIO32) - Active Low with Pull-up");
  Serial.println("========================================\n");
  
  // Initialize WiFiManager
  Serial.println("Initializing WiFi Manager...");
  showOLEDStatus("WiFi Setup", "Connecting...", "Please wait");
  wifiManager.setAPCallback([](WiFiManager *wiFiManager) {
    Serial.println("\n[WiFi] Entered Config Portal");
    Serial.print("[WiFi] Access Point SSID: ");
    Serial.println(AP_SSID);
    Serial.print("[WiFi] Access Point Password: ");
    Serial.println(AP_PASSWORD);
    showOLEDStatus("Config Portal", AP_SSID, "Open WiFi setup");
  });
  
  wifiManager.setConfigPortalTimeout(180);  // 3 minutes timeout
  
  // Auto-connect or start config portal
  if (!wifiManager.autoConnect(AP_SSID, AP_PASSWORD)) {
    Serial.println("[WiFi] Failed to connect, reset will be attempted");
    showOLEDStatus("WiFi Failed", "Config timeout", "Restarting...");
    delay(1500);
  } else {
    showOLEDStatus("WiFi Connected", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    sendTelegramMessage(String("ESP32 started\nWiFi connected: ") + WiFi.SSID() +
                        "\nIP: " + WiFi.localIP().toString());
    delay(1500);
  }
  clearOLEDStatus();
  updateOLED();
  
  Serial.println("[Setup] Initialization Complete!");
  
  // Check OpenWeather API Key
  Serial.println("\n========== Weather Data Configuration ==========");
  if (strcmp(OPENWEATHER_API_KEY, "YOUR_API_KEY") == 0) {
    Serial.println("[Weather] WARNING: API Key not set!");
    Serial.println("[Weather] Get API key from: https://openweathermap.org/api");
    Serial.println("[Weather] Update OPENWEATHER_API_KEY in main.cpp");
  } else {
    Serial.println("[Weather] API Key configured");
    Serial.println("[Weather] Location: Chiang Mai (18.7883°N, 98.9853°E)");
    Serial.println("[Weather] Update Interval: 2 minutes");
    Serial.println("[Weather] Data: Temperature, Humidity, AQI, PM2.5, PM10");
    Serial.println("=============================================\n");
  }
}

// ========== Main Loop ==========
void loop() {
  // Poll switches at regular intervals (non-blocking)
  unsigned long currentTime = millis();
  
  // Check SW1 Long Press for WiFi Reset
  checkSW1LongPress();
  
  // Fetch Weather Data Periodically (every 2 minutes)
  if (currentTime - lastWeatherUpdateTime >= WEATHER_UPDATE_INTERVAL) {
    lastWeatherUpdateTime = currentTime;
    
    if (WiFi.status() == WL_CONNECTED && strcmp(OPENWEATHER_API_KEY, "YOUR_API_KEY") != 0) {
      Serial.println("\n[Weather] Fetching data...");
      fetchWeatherData();
      fetchAirQualityData();
      printWeatherData();
      sendTelegramMessage(formatWeatherTelegramMessage());
      updateOLED();
    }
  }
  
  // Check WiFi Status Periodically
  if (currentTime - lastWiFiCheckTime >= WIFI_STATUS_CHECK_INTERVAL) {
    lastWiFiCheckTime = currentTime;
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[WiFi] Connection is active");
      printWiFiStatus();
    } else {
      Serial.println("[WiFi] Connection LOST - Attempting to reconnect...");
      updateOLED();
      wifiManager.autoConnect(AP_SSID, AP_PASSWORD);
      updateOLED();
    }
  }

  if (currentTime - lastOledRefreshTime >= OLED_REFRESH_INTERVAL) {
    lastOledRefreshTime = currentTime;
    updateOLED();
  }
  
  if (currentTime - lastSwitchCheckTime >= SWITCH_POLL_INTERVAL) {
    lastSwitchCheckTime = currentTime;
    
    // Check SW1 - Toggle Relay1 (only if not in long press mode)
    if (!sw1_long_press_triggered && isSwitchPressedOnce(SWITCH1_PIN, sw1_prev_state)) {
      toggleRelay(RELAY1_PIN, relay1_state);
      Serial.println("[EVENT] SW1 (GPIO34) Pressed -> Relay1 (GPIO17) Toggled");
      Serial.printf("Relay1 is now: %s\n", relay1_state ? "ON" : "OFF");
      sendTelegramMessage(formatRelayTelegramMessage("Relay1", relay1_state));
      updateOLED();
    }
    
    // Check SW2 - Toggle Relay2
    if (isSwitchPressedOnce(SWITCH2_PIN, sw2_prev_state)) {
      toggleRelay(RELAY2_PIN, relay2_state);
      Serial.println("[EVENT] SW2 (GPIO35) Pressed -> Relay2 (GPIO16) Toggled");
      Serial.printf("Relay2 is now: %s\n", relay2_state ? "ON" : "OFF");
      sendTelegramMessage(formatRelayTelegramMessage("Relay2", relay2_state));
      updateOLED();
    }
    
    // Check SW3 - Toggle Relay3
    if (isSwitchPressedOnce(SWITCH3_PIN, sw3_prev_state)) {
      toggleRelay(RELAY3_PIN, relay3_state);
      Serial.println("[EVENT] SW3 (GPIO32) Pressed -> Relay3 (GPIO4) Toggled");
      Serial.printf("Relay3 is now: %s\n", relay3_state ? "ON" : "OFF");
      sendTelegramMessage(formatRelayTelegramMessage("Relay3", relay3_state));
      updateOLED();
    }
  }
}

// ========== Function Implementations ==========

/**
 * Toggle Relay On/Off (Active Low)
 * @param relayPin: GPIO pin number
 * @param relayState: Reference to relay state variable
 */
void toggleRelay(int relayPin, bool& relayState) {
  relayState = !relayState;  // Toggle state
  
  if (relayState) {
    // Turn ON (Active Low = LOW)
    digitalWrite(relayPin, LOW);
  } else {
    // Turn OFF (Active Low = HIGH)
    digitalWrite(relayPin, HIGH);
  }
}

/**
 * Detect Single Press of Switch (Active Low) with Debouncing
 * Returns true only on the transition from HIGH to LOW (button press)
 * @param switchPin: GPIO pin number
 * @param prevState: Reference to previous state variable
 * @return: true if switch was pressed (edge detection), false otherwise
 */
bool isSwitchPressedOnce(int switchPin, int& prevState) {
  int currentState = digitalRead(switchPin);
  
  // Check for state change
  if (currentState != prevState) {
    // Wait for debouncing
    delay(DEBOUNCE_DELAY);
    
    // Read again after debouncing
    currentState = digitalRead(switchPin);
    
    // Detect falling edge: HIGH -> LOW (button press)
    if (currentState == LOW && prevState == HIGH) {
      prevState = currentState;
      return true;  // Button was pressed
    }
    
    // Update previous state
    prevState = currentState;
  }
  
  return false;  // No button press
}

/**
 * Check SW1 for Long Press (5 seconds) to Reset WiFi Configuration
 */
void checkSW1LongPress() {
  int sw1_current = digitalRead(SWITCH1_PIN);
  
  // Detect button press (HIGH -> LOW)
  if (sw1_current == LOW && sw1_long_press_prev_state == HIGH) {
    sw1_press_start_time = millis();
    sw1_long_press_triggered = false;
    Serial.println("[SW1] Button Press Detected - Starting timer...");
  }
  
  // Check if button is still pressed and 5 seconds have elapsed
  if (sw1_current == LOW && !sw1_long_press_triggered) {
    unsigned long pressTime = millis() - sw1_press_start_time;
    
    // Debounce check
    if (pressTime > DEBOUNCE_DELAY) {
      // Print countdown every second
      static unsigned long lastPrintTime = 0;
      if (pressTime - lastPrintTime >= 1000) {
        unsigned long secondsElapsed = (pressTime / 1000);
        Serial.printf("[SW1] Hold time: %lu seconds...\n", secondsElapsed + 1);
        lastPrintTime = pressTime;
      }
      
      // Check if long press threshold reached
      if (pressTime >= WIFI_RESET_HOLD_TIME) {
        sw1_long_press_triggered = true;
        Serial.println("\n[SW1] Long Press DETECTED (5 seconds) - Resetting WiFi Configuration!");
        showOLEDStatus("WiFi Reset", "SW1 hold 5 sec", "Restarting...");
        resetWiFiConfiguration();
      }
    }
  }
  
  // Detect button release (LOW -> HIGH)
  if (sw1_current == HIGH && sw1_long_press_prev_state == LOW) {
    sw1_long_press_triggered = false;
    if (millis() - sw1_press_start_time < WIFI_RESET_HOLD_TIME) {
      Serial.println("[SW1] Button Released (Short Press)");
    }
  }
  
  sw1_long_press_prev_state = sw1_current;
}

/**
 * Reset WiFi Configuration and Enter Config Portal
 */
void resetWiFiConfiguration() {
  Serial.println("\n========== WiFi Reset Starting ==========");
  Serial.println("[WiFi] Resetting saved WiFi configuration...");
  showOLEDStatus("WiFi Reset", "Clearing saved", "settings...");
  sendTelegramMessage("ESP32 WiFi settings reset requested. Restarting...");
  
  // Reset WiFi settings
  wifiManager.resetSettings();
  
  Serial.println("[WiFi] Configuration reset complete!");
  Serial.println("[WiFi] Restarting in 2 seconds...");
  showOLEDStatus("WiFi Reset Done", "Restarting ESP32", "");
  delay(2000);
  
  // Restart ESP32
  ESP.restart();
}

/**
 * Connect to WiFi Network using WiFiManager
 */
void connectWiFi() {
  if (!wifiManager.autoConnect(AP_SSID, AP_PASSWORD)) {
    Serial.println("[WiFi] Failed to connect and timeout occurred");
  }
}

/**
 * Print WiFi Status Information
 */
void printWiFiStatus() {
  Serial.println("\n========== WiFi Status ==========");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  
  Serial.print("Signal Strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  
  Serial.println("================================\n");
}

/**
 * Fetch Weather Data from OpenWeather API
 */
void fetchWeatherData() {
  WiFiClient client;
  HTTPClient http;
  
  // Build URL with parameters
  String url = String(OPENWEATHER_WEATHER_URL) + "?lat=" + String(CITY_LAT, 4) + "&lon=" + String(CITY_LON, 4) + 
               "&appid=" + OPENWEATHER_API_KEY + "&units=metric";
  
  Serial.printf("[Weather] GET %s\n", url.c_str());
  
  http.begin(client, url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      // Extract weather data
      weatherData.temperature = doc["main"]["temp"];
      weatherData.feels_like = doc["main"]["feels_like"];
      weatherData.humidity = doc["main"]["humidity"];
      weatherData.pressure = doc["main"]["pressure"];
      weatherData.weather_description = doc["weather"][0]["description"].as<String>();
      weatherData.wind_speed = doc["wind"]["speed"];
      weatherData.last_update = millis();
      
      Serial.println("[Weather] Data fetched successfully");
    } else {
      Serial.print("[Weather] JSON parsing error: ");
      Serial.println(error.f_str());
    }
  } else {
    Serial.printf("[Weather] HTTP error: %d\n", httpCode);
  }
  
  http.end();
}

/**
 * Fetch Air Quality Data (AQI, PM2.5, PM10) from OpenWeather API
 */
void fetchAirQualityData() {
  WiFiClient client;
  HTTPClient http;
  
  // Build URL for Air Pollution API
  String url = String(OPENWEATHER_AQI_URL) + "?lat=" + String(CITY_LAT, 4) + "&lon=" + String(CITY_LON, 4) + 
               "&appid=" + OPENWEATHER_API_KEY;
  
  Serial.printf("[AQI] GET %s\n", url.c_str());
  
  http.begin(client, url);
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    
    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      // Extract AQI and pollutant data
      weatherData.aqi = doc["list"][0]["main"]["aqi"];
      weatherData.pm25 = doc["list"][0]["components"]["pm2_5"];
      weatherData.pm10 = doc["list"][0]["components"]["pm10"];
      
      Serial.println("[AQI] Data fetched successfully");
    } else {
      Serial.print("[AQI] JSON parsing error: ");
      Serial.println(error.f_str());
    }
  } else {
    Serial.printf("[AQI] HTTP error: %d\n", httpCode);
  }
  
  http.end();
}

/**
 * Print Weather Data to Serial Monitor
 */
void printWeatherData() {
  Serial.println("\n========== Weather Data for Chiang Mai ==========");
  Serial.print("Update Time: ");
  Serial.println(weatherData.last_update);
  
  Serial.println("\n--- Temperature & Humidity ---");
  Serial.printf("Temperature: %.2f°C\n", weatherData.temperature);
  Serial.printf("Feels Like: %.2f°C\n", weatherData.feels_like);
  Serial.printf("Humidity: %.0f%%\n", weatherData.humidity);
  Serial.printf("Pressure: %.0f hPa\n", weatherData.pressure);
  Serial.print("Weather: ");
  Serial.println(weatherData.weather_description);
  Serial.printf("Wind Speed: %.2f m/s\n", weatherData.wind_speed);
  
  Serial.println("\n--- Air Quality Index ---");
  Serial.print("AQI Level: ");
  switch(weatherData.aqi) {
    case 1: Serial.println("Good"); break;
    case 2: Serial.println("Fair"); break;
    case 3: Serial.println("Moderate"); break;
    case 4: Serial.println("Poor"); break;
    case 5: Serial.println("Very Poor"); break;
    default: Serial.println("Unknown");
  }
  
  Serial.println("\n--- Particulate Matter ---");
  Serial.printf("PM2.5: %.2f µg/m³\n", weatherData.pm25);
  Serial.printf("PM10: %.2f µg/m³\n", weatherData.pm10);
  
  // Health recommendation based on AQI
  Serial.println("\n--- Health Recommendation ---");
  if (weatherData.aqi <= 2) {
    Serial.println("Air quality is satisfactory.");
  } else if (weatherData.aqi == 3) {
    Serial.println("Members of sensitive groups should consider limiting outdoor activity.");
  } else if (weatherData.aqi >= 4) {
    Serial.println("Everyone should consider limiting outdoor activity.");
  }
  
  Serial.println("==============================================\n");
}

/**
 * Check whether Telegram Bot Token and Chat ID are configured.
 */
bool isTelegramConfigured() {
  return strcmp(TELEGRAM_BOT_TOKEN, "YOUR_TELEGRAM_BOT_TOKEN") != 0 &&
         strcmp(TELEGRAM_CHAT_ID, "YOUR_TELEGRAM_CHAT_ID") != 0;
}

/**
 * Send a message to Telegram via Bot API.
 */
bool sendTelegramMessage(const String& message) {
  if (!isTelegramConfigured()) {
    Serial.println("[Telegram] Skipped: Bot token or chat ID is not configured");
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Telegram] Skipped: WiFi is not connected");
    return false;
  }

  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure();

  String url = String(TELEGRAM_API_URL) + TELEGRAM_BOT_TOKEN + "/sendMessage";
  String body = "chat_id=" + urlEncode(TELEGRAM_CHAT_ID) +
                "&text=" + urlEncode(message) +
                "&disable_web_page_preview=true";

  Serial.println("[Telegram] Sending message...");
  Serial.print("[Telegram] URL: ");
  Serial.println(url);
  Serial.print("[Telegram] Body: ");
  Serial.println(body);

  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(body);

  if (httpCode == HTTP_CODE_OK) {
    Serial.println("[Telegram] Message sent");
    http.end();
    return true;
  }

  String payload = http.getString();
  Serial.printf("[Telegram] Send failed, HTTP code: %d\n", httpCode);
  if (payload.length() > 0) {
    Serial.print("[Telegram] Response: ");
    Serial.println(payload);
  }
  http.end();
  return false;
}

/**
 * URL-encode text for Telegram form POST body.
 */
String urlEncode(const String& value) {
  const char* hex = "0123456789ABCDEF";
  String encoded = "";

  for (size_t i = 0; i < value.length(); i++) {
    uint8_t c = value[i];

    if ((c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') ||
        c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += char(c);
    } else if (c == ' ') {
      encoded += '+';
    } else {
      encoded += '%';
      encoded += hex[(c >> 4) & 0x0F];
      encoded += hex[c & 0x0F];
    }
  }

  return encoded;
}

/**
 * Format relay status notification.
 */
String formatRelayTelegramMessage(const char* relayName, bool relayState) {
  String message = "ESP32 Relay Update\n";
  message += relayName;
  message += ": ";
  message += relayState ? "ON" : "OFF";
  message += "\nR1: ";
  message += relay1_state ? "ON" : "OFF";
  message += " | R2: ";
  message += relay2_state ? "ON" : "OFF";
  message += " | R3: ";
  message += relay3_state ? "ON" : "OFF";
  return message;
}

/**
 * Format OpenWeather data notification.
 */
String formatWeatherTelegramMessage() {
  String message = "ESP32 Weather Update - ";
  message += CITY_NAME;
  message += "\nTemp: ";
  message += String(weatherData.temperature, 1);
  message += " C";
  message += "\nHum: ";
  message += String(weatherData.humidity, 0);
  message += "%";
  message += "\nAQI: ";
  message += String(weatherData.aqi);
  message += " ";
  message += getAQILabel(weatherData.aqi);
  message += "\nPM2.5: ";
  message += String(weatherData.pm25, 1);
  message += " ug/m3";
  message += "\nRelay: R1 ";
  message += relay1_state ? "ON" : "OFF";
  message += ", R2 ";
  message += relay2_state ? "ON" : "OFF";
  message += ", R3 ";
  message += relay3_state ? "ON" : "OFF";
  return message;
}

/**
 * Initialize OLED 0.96" I2C display.
 */
void initOLED() {
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

  oled_available = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (!oled_available) {
    Serial.println("[OLED] Display not found at 0x3C");
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("ESP32 Weather");
  display.println("OLED Ready");
  display.display();
  Serial.println("[OLED] Display initialized");
}

/**
 * Show a setup/status screen on the OLED.
 */
void showOLEDStatus(const char* title, const char* line1, const char* line2) {
  strncpy(oled_status_line1, line1, sizeof(oled_status_line1) - 1);
  oled_status_line1[sizeof(oled_status_line1) - 1] = '\0';
  strncpy(oled_status_line2, line2, sizeof(oled_status_line2) - 1);
  oled_status_line2[sizeof(oled_status_line2) - 1] = '\0';
  oled_show_status = true;

  if (!oled_available) {
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.drawRoundRect(0, 0, OLED_WIDTH, OLED_HEIGHT, 4, SSD1306_WHITE);
  display.fillRect(0, 0, OLED_WIDTH, 13, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(4, 3);
  display.print(title);

  display.setTextColor(SSD1306_WHITE);
  display.setCursor(8, 22);
  display.print(oled_status_line1);
  display.setCursor(8, 36);
  display.print(oled_status_line2);
  display.display();
}

/**
 * Return the OLED to the normal dashboard screen.
 */
void clearOLEDStatus() {
  oled_show_status = false;
  oled_status_line1[0] = '\0';
  oled_status_line2[0] = '\0';
}

/**
 * During setup, holding SW1 for 5 seconds clears saved WiFi settings.
 */
void checkStartupWiFiReset() {
  if (digitalRead(SWITCH1_PIN) == HIGH) {
    return;
  }

  Serial.println("[Setup] SW1 held during boot - checking for WiFi reset...");
  unsigned long holdStart = millis();
  unsigned long lastSecondShown = 0;

  while (digitalRead(SWITCH1_PIN) == LOW) {
    unsigned long holdTime = millis() - holdStart;
    unsigned long secondsHeld = holdTime / 1000;

    if (secondsHeld != lastSecondShown) {
      lastSecondShown = secondsHeld;
      char line2[22];
      snprintf(line2, sizeof(line2), "%lu / 5 seconds", secondsHeld);
      showOLEDStatus("Hold SW1", "Reset WiFi?", line2);
      Serial.printf("[Setup] SW1 hold: %lu seconds\n", secondsHeld);
    }

    if (holdTime >= WIFI_RESET_HOLD_TIME) {
      Serial.println("[Setup] Startup WiFi reset triggered");
      resetWiFiConfiguration();
    }

    delay(50);
  }

  Serial.println("[Setup] SW1 released before reset threshold");
  showOLEDStatus("WiFi Reset", "Cancelled", "Continue boot");
  delay(800);
}

/**
 * Draw weather data and relay status on the OLED.
 */
void updateOLED() {
  if (!oled_available) {
    return;
  }

  if (oled_show_status) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Header
  display.fillRect(0, 0, OLED_WIDTH, 11, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(3, 2);
  display.print(CITY_NAME);
  display.setCursor(102, 2);
  display.print(WiFi.status() == WL_CONNECTED ? "WiFi" : "AP");

  display.setTextColor(SSD1306_WHITE);
  display.drawLine(0, 13, OLED_WIDTH, 13, SSD1306_WHITE);

  // Weather values
  display.setCursor(0, 17);
  display.print("Temp ");
  if (weatherData.last_update > 0) {
    display.print(weatherData.temperature, 1);
    display.print("C");
  } else {
    display.print("--.-C");
  }

  display.setCursor(70, 17);
  display.print("Hum ");
  if (weatherData.last_update > 0) {
    display.print(weatherData.humidity, 0);
    display.print("%");
  } else {
    display.print("--%");
  }

  display.setCursor(0, 29);
  display.print("AQI ");
  if (weatherData.aqi > 0) {
    display.print(weatherData.aqi);
    display.print(" ");
    display.print(getAQILabel(weatherData.aqi));
  } else {
    display.print("--");
  }

  display.setCursor(70, 29);
  display.print("PM2.5 ");
  if (weatherData.pm25 > 0.0) {
    display.print(weatherData.pm25, 0);
  } else {
    display.print("--");
  }

  // Relay status strip
  display.drawLine(0, 43, OLED_WIDTH, 43, SSD1306_WHITE);
  drawRelayStatus(0, 48, "R1", relay1_state);
  drawRelayStatus(43, 48, "R2", relay2_state);
  drawRelayStatus(86, 48, "R3", relay3_state);

  display.display();
}

/**
 * Draw one relay pill. Filled means ON, outline means OFF.
 */
void drawRelayStatus(int x, int y, const char* label, bool state) {
  display.drawRoundRect(x, y, 39, 14, 3, SSD1306_WHITE);

  if (state) {
    display.fillRoundRect(x, y, 39, 14, 3, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else {
    display.setTextColor(SSD1306_WHITE);
  }

  display.setCursor(x + 4, y + 3);
  display.print(label);
  display.setCursor(x + 18, y + 3);
  display.print(state ? "ON" : "OFF");
  display.setTextColor(SSD1306_WHITE);
}

/**
 * Convert OpenWeather AQI number to a compact display label.
 */
const char* getAQILabel(int aqi) {
  switch (aqi) {
    case 1: return "Good";
    case 2: return "Fair";
    case 3: return "Mod";
    case 4: return "Poor";
    case 5: return "Bad";
    default: return "Unk";
  }
}
