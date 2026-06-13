#include <Arduino.h>
#include <WiFi.h>

// ========== WiFi Configuration ==========
const char* ssid = "SOPA_Wifi";           // Change to your WiFi SSID
const char* password = "21072521";        // Change to your WiFi Password
#define WIFI_CONNECT_TIMEOUT 20000        // 20 seconds timeout

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

// ========== Relay State Variables ==========
bool relay1_state = false;  // false = OFF, true = ON
bool relay2_state = false;
bool relay3_state = false;

// ========== Switch State Variables (for edge detection) ==========
int sw1_prev_state = HIGH;
int sw2_prev_state = HIGH;
int sw3_prev_state = HIGH;

// ========== WiFi Status Variable ==========
bool wifi_connected = false;

// ========== Function Prototypes ==========
void toggleRelay(int relayPin, bool& relayState);
bool isSwitchPressedOnce(int switchPin, int& prevState);
void connectWiFi();
void printWiFiStatus();
bool isSwitchPressedOnce(int switchPin, int& prevState);

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
  
  // Print initialization message
  Serial.println("\n========================================");
  Serial.println("   Relay & Switch Control System");
  Serial.println("========================================");
  Serial.println("Relay1 (GPIO17) - Toggle via SW1");
  Serial.println("Relay2 (GPIO16) - Toggle via SW2");
  Serial.println("Relay3 (GPIO4)  - Toggle via SW3");
  Serial.println("\nSwitch Configuration:");
  Serial.println("SW1 (GPIO34) - Active Low with Pull-up");
  Serial.println("SW2 (GPIO35) - Active Low with Pull-up");
  Serial.println("SW3 (GPIO32) - Active Low with Pull-up");
  Serial.println("========================================\n");
  
  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  connectWiFi();
}

// ========== Main Loop ==========
void loop() {
  // Poll switches at regular intervals (non-blocking)
  unsigned long currentTime = millis();
  
  // Check WiFi Status Periodically
  if (currentTime - lastWiFiCheckTime >= WIFI_STATUS_CHECK_INTERVAL) {
    lastWiFiCheckTime = currentTime;
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[WiFi] Connection is active");
      printWiFiStatus();
    } else {
      Serial.println("[WiFi] Connection LOST - Attempting to reconnect...");
      connectWiFi();
    }
  }
  
  if (currentTime - lastSwitchCheckTime >= SWITCH_POLL_INTERVAL) {
    lastSwitchCheckTime = currentTime;
    
    // Check SW1 - Toggle Relay1
    if (isSwitchPressedOnce(SWITCH1_PIN, sw1_prev_state)) {
      toggleRelay(RELAY1_PIN, relay1_state);
      Serial.println("[EVENT] SW1 (GPIO34) Pressed -> Relay1 (GPIO17) Toggled");
      Serial.printf("Relay1 is now: %s\n", relay1_state ? "ON" : "OFF");
    }
    
    // Check SW2 - Toggle Relay2
    if (isSwitchPressedOnce(SWITCH2_PIN, sw2_prev_state)) {
      toggleRelay(RELAY2_PIN, relay2_state);
      Serial.println("[EVENT] SW2 (GPIO35) Pressed -> Relay2 (GPIO16) Toggled");
      Serial.printf("Relay2 is now: %s\n", relay2_state ? "ON" : "OFF");
    }
    
    // Check SW3 - Toggle Relay3
    if (isSwitchPressedOnce(SWITCH3_PIN, sw3_prev_state)) {
      toggleRelay(RELAY3_PIN, relay3_state);
      Serial.println("[EVENT] SW3 (GPIO32) Pressed -> Relay3 (GPIO4) Toggled");
      Serial.printf("Relay3 is now: %s\n", relay3_state ? "ON" : "OFF");
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
 * Connect to WiFi Network
 */
void connectWiFi() {
  unsigned long startTime = millis();
  
  // Disconnect any existing WiFi connection
  WiFi.disconnect(true);  // true = turn off WiFi radio
  delay(100);
  
  // Set WiFi mode to Station
  WiFi.mode(WIFI_STA);
  
  // Begin WiFi connection
  WiFi.begin(ssid, password);
  
  Serial.print("\nConnecting to WiFi: ");
  Serial.println(ssid);
  
  // Wait for connection or timeout
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
    // Check for timeout
    if (millis() - startTime > WIFI_CONNECT_TIMEOUT) {
      Serial.println("\n[WiFi] Connection FAILED - Timeout!");
      wifi_connected = false;
      return;
    }
  }
  
  Serial.println("\n[WiFi] Connected Successfully!");
  wifi_connected = true;
  printWiFiStatus();
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
