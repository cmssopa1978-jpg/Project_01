#include <Arduino.h>

// กำหนด GPIO Pins สำหรับ Relay
#define RELAY1_PIN 17
#define RELAY2_PIN 16
#define RELAY3_PIN 4

// กำหนด Timing Constants (มิลลิวินาที)
#define RELAY_ON_TIME 5000    // 5 วินาที
#define RELAY_OFF_TIME 5000   // 5 วินาที

// ฟังก์ชันควบคุม Relay (Active Low)
void turnOnRelay(int relayPin) {
  digitalWrite(relayPin, LOW);  // LOW = ON (Active Low)
}

void turnOffRelay(int relayPin) {
  digitalWrite(relayPin, HIGH);  // HIGH = OFF (Active Low)
}

void setup() {
  // เริ่มต้น Serial สำหรับการแสดงผล
  Serial.begin(115200);
  delay(1000);
  
  // ตั้งค่า Relay Pins เป็น OUTPUT
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  
  // ตั้งค่าเริ่มต้น ปิด Relay ทั้งหมด (Active Low = HIGH = OFF)
  turnOffRelay(RELAY1_PIN);
  turnOffRelay(RELAY2_PIN);
  turnOffRelay(RELAY3_PIN);
  
  Serial.println("========================================");
  Serial.println("     Relay Control System Started");
  Serial.println("========================================");
  Serial.println("Relay1 (GPIO17) - Active Low");
  Serial.println("Relay2 (GPIO16) - Active Low");
  Serial.println("Relay3 (GPIO4)  - Active Low");
  Serial.println("ON Time:  5 seconds");
  Serial.println("OFF Time: 5 seconds");
  Serial.println("========================================");
}

void loop() {
  // ========== เปิด Relay ทั้ง 3 ตัว ==========
  Serial.println("\n[ACTION] Turning ON all Relays...");
  turnOnRelay(RELAY1_PIN);
  turnOnRelay(RELAY2_PIN);
  turnOnRelay(RELAY3_PIN);
  
  Serial.println("  ✓ Relay1 (GPIO17) - ON");
  Serial.println("  ✓ Relay2 (GPIO16) - ON");
  Serial.println("  ✓ Relay3 (GPIO4)  - ON");
  Serial.println("Waiting for 5 seconds...");
  
  // รอ 5 วินาที
  delay(RELAY_ON_TIME);
  
  // ========== ปิด Relay ทั้ง 3 ตัว ==========
  Serial.println("\n[ACTION] Turning OFF all Relays...");
  turnOffRelay(RELAY1_PIN);
  turnOffRelay(RELAY2_PIN);
  turnOffRelay(RELAY3_PIN);
  
  Serial.println("  ✓ Relay1 (GPIO17) - OFF");
  Serial.println("  ✓ Relay2 (GPIO16) - OFF");
  Serial.println("  ✓ Relay3 (GPIO4)  - OFF");
  Serial.println("Waiting for 5 seconds...");
  
  // รอ 5 วินาที
  delay(RELAY_OFF_TIME);
}
