# ESP32 DevKit V2 Board Details

บอร์ดพัฒนา **ESP32 DevKit V2** เป็นโมดูลไมโครคอนโทรลเลอร์ที่มีประสิทธิภาพสูง รองรับการเชื่อมต่อไร้สายทั้ง Wi-Fi และ Bluetooth ในตัว เหมาะสำหรับงานด้าน IoT (Internet of Things)

## 1. ข้อมูลทางเทคนิค (Technical Specifications)

| คุณสมบัติ | รายละเอียด |
|-----------|-----------|
| **Microcontroller** | Xtensa® Dual-Core 32-bit LX6 |
| **Clock Speed** | สูงสุด 240 MHz |
| **SRAM** | 520 KB |
| **Flash Memory** | 4 MB (ส่วนใหญ่) |
| **Wi-Fi** | 802.11 b/g/n (สูงสุด 150 Mbps) |
| **Bluetooth** | v4.2 BR/EDR และ BLE (Bluetooth Low Energy) |
| **Operating Voltage** | 3.3V (Internal), 5V (USB/Vin) |
| **Digital I/O Pins** | 34 ขา (รวมขาที่เป็น Input Only) |
| **ADC Channels** | 12-bit, 18 ช่อง |
| **DAC Channels** | 8-bit, 2 ช่อง |

## 2. การเชื่อมต่อและพอร์ตต่างๆ (Peripherals)
- **UART:** 3 พอร์ต
- **SPI:** 3 พอร์ต
- **I2C:** 2 พอร์ต
- **PWM:** 16 ช่อง (สำหรับควบคุม Servo หรือ LED)
- **Capacitive Touch:** 10 เซนเซอร์
- **Hardware Encryption:** AES, SHA-2, RSA, ECC, RNG

## 3. ขาใช้งานที่สำคัญ (Pinout Summary)
เนื่องจากบอร์ด V2 มักจะมี 38 ขา ต่อไปนี้คือกลุ่มขาที่ควรทราบ:

### แหล่งจ่ายไฟ (Power)
- **5V / Vin:** รับไฟเข้า 5V จาก USB หรือแหล่งจ่ายภายนอก
- **3V3:** ขาจ่ายไฟออก 3.3V (จาก Regulator บนบอร์ด)
- **GND:** ขากราวด์

### ขา Input Only (ไม่สามารถ Output ได้)
- GPIO 34, 35, 36 (VP), 39 (VN)

### ขาสำหรับการสื่อสาร (Default Pins)
- **I2C:** SDA (GPIO 21), SCL (GPIO 22)
- **SPI:** MOSI (GPIO 23), MISO (GPIO 19), SCK (GPIO 18), CS (GPIO 5)
- **UART0 (Debug):** TX (GPIO 1), RX (GPIO 3)

## 4. ข้อควรระวัง
1. **Logic Level:** ESP32 ทำงานที่ระดับแรงดัน **3.3V** เท่านั้น การต่อเซนเซอร์ที่เป็น 5V โดยตรงอาจทำให้ขา GPIO เสียหายได้
2. **Current Limit:** กระแสสูงสุดที่แต่ละขา GPIO จ่ายได้คือประมาณ 12-20mA
3. **Boot Pins:** GPIO 0, 2, 5, 12, 15 เป็นขา Strapping Pins ที่มีผลต่อการ Boot เครื่อง ควรหลีกเลี่ยงการต่ออุปกรณ์ที่ดึงสัญญาณ (Pull-up/Pull-down) ในขณะเปิดเครื่องหากไม่จำเป็น

## 5. ส่วนประกอบบนบอร์ด (On-board Components)

| ส่วนประกอบ | รายละเอียด |
|-----------|-----------|
| **USB Connector** | Micro USB - สำหรับอัปโหลดโปรแกรมและจ่ายไฟ |
| **Reset Button** | ปุ่มรีเซ็ต - สำหรับรีสตาร์ทบอร์ด |
| **Boot Button** | ปุ่ม IO0 - ใช้สำหรับเข้าสู่ Bootloader Mode |
| **Power LED** | ไฟบ่งชี้ไฟเลี้ยง (ปกติสีแดง) |
| **Voltage Regulator** | ไอซีที่ลด 5V เป็น 3.3V |
| **USB-to-Serial Chip** | เพื่อการสื่อสาร UART ผ่าน USB |

## 6. การใช้งานเบื้องต้น (Getting Started)

### การตั้งค่า PlatformIO
```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
```

### การอัปโหลดโปรแกรม
1. เสียบสาย Micro USB เข้าคอมพิวเตอร์
2. เปิด PlatformIO และเลือก "Upload"
3. หากการอัปโหลดล้มเหลว ให้กดและค้างปุ่ม Boot แล้วกดปุ่ม Reset

## 7. การใช้ WiFi และ Bluetooth

### WiFi Connectivity
```cpp
#include <WiFi.h>

void setup() {
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}
```

### Bluetooth Low Energy (BLE)
ESP32 รองรับ BLE สำหรับการสื่อสารแบบประหยัดพลังงาน เหมาะสำหรับอุปกรณ์แบตเตอรี่

## 8. ปริมาณการใช้กำลังไฟ (Power Consumption)
- **Deep Sleep Mode:** ~10 µA
- **Light Sleep Mode:** ~0.1 mA
- **Idle Mode:** ~15 mA
- **WiFi TX:** ~160 - 240 mA
- **BLE TX:** ~80 - 100 mA

## 9. การเชื่อมต่อเซนเซอร์ที่พบบ่อย (Common Sensors)

### Temperature/Humidity (DHT22)
- VCC → 3V3
- GND → GND
- DATA → GPIO (เลือกได้ตามต้องการ)

### Ultrasonic Sensor (HC-SR04)
- VCC → 5V (ต้องใช้ Voltage Divider สำหรับ Echo pin)
- GND → GND
- Trig → GPIO
- Echo → GPIO (ผ่าน Voltage Divider เพื่อลดจาก 5V เป็น 3.3V)

### OLED Display (I2C)
- VCC → 3V3
- GND → GND
- SCL → GPIO 22
- SDA → GPIO 21

## 10. โหมด Sleep และการประหยัดพลังงาน

ESP32 มีหลายโหมดการนอนหลับเพื่อประหยัดพลังงาน:
- **Light Sleep:** ปิด CPU แต่ RTC ยังทำงาน (~0.1 mA)
- **Deep Sleep:** ปิดทุกอย่างยกเว้น RTC (~10 µA)
- **Hibernation:** โหมดประหยัดพลังงานสูงสุด

## 11. ตัวอย่างโปรแกรมง่ายๆ (Simple Example)

```cpp
#include <Arduino.h>

const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  Serial.println("LED Blink!");
}
```

## 12. ลิงก์อ้างอิงที่มีประโยชน์ (Useful Resources)
- [Espressif ESP32 Official Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Arduino Core for ESP32](https://github.com/espressif/arduino-esp32)
- [PlatformIO ESP32 Support](https://docs.platformio.org/en/latest/platforms/espressif32.html)
- [ESP32 Pinout Diagram](https://github.com/AzureKn1ght/ESP32_DevKit_v1)

## 13. แนวทางการ Troubleshooting (Troubleshooting Guide)

| ปัญหา | สาเหตุ | วิธีแก้ไข |
|-------|--------|--------|
| ไม่สามารถอัปโหลดโปรแกรม | ไม่มี COM Port | ตรวจสอบไดรเวอร์ USB-to-Serial, ลองเข้า Boot Mode |
| บอร์ดไม่ตอบสนอง | ไดรเวอร์หรือ Firmware มีปัญหา | ลองรีเซ็ตหรืออัปโหลด Bootloader ใหม่ |
| WiFi ล้มเหลว | Credentials ผิดหรือสัญญาณ WiFi อ่อน | ตรวจสอบ SSID และ Password, อยู่ใกล้ Router |
| GPIO ไม่ทำงาน | ใช้ Strapping Pin หรือ Input-Only Pin | เลือก GPIO ที่ถูกต้องตามไฟแสง |
| หน่วยความจำไม่เพียงพอ | Code ใหญ่เกินไป | ปรับลดขนาด Code หรือเพิ่มหน่วยความจำ PSRAM |

## 14. การควบคุม Relay (Relay Control)

บอร์ด ESP32 DevKit V2 นี้รองรับการควบคุม Relay แบบ Active Low ดังนี้:

### ตารางการกำหนด Relay

| Relay | GPIO Pin | สถานะ | หมายเหตุ |
|-------|----------|--------|---------|
| **Relay1** | GPIO17 | Active Low | ปกติ ON (High=OFF, Low=ON) |
| **Relay2** | GPIO16 | Active Low | ปกติ ON (High=OFF, Low=ON) |
| **Relay3** | GPIO4 | Active Low | ปกติ ON (High=OFF, Low=ON) |

### ตัวอย่างการเขียนโปรแกรมควบคุม Relay

```cpp
#include <Arduino.h>

// กำหนด GPIO Pins สำหรับ Relay
#define RELAY1_PIN 17
#define RELAY2_PIN 16
#define RELAY3_PIN 4

void setup() {
  Serial.begin(115200);
  
  // ตั้งค่า Relay Pins เป็น OUTPUT
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  
  // ตั้งค่าเริ่มต้น ปิด Relay ทั้งหมด (Active Low = HIGH = OFF)
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  
  Serial.println("Relay pins initialized!");
}

void loop() {
  // เปิด Relay1 (Active Low)
  digitalWrite(RELAY1_PIN, LOW);
  Serial.println("Relay1 ON");
  delay(2000);
  
  // ปิด Relay1
  digitalWrite(RELAY1_PIN, HIGH);
  Serial.println("Relay1 OFF");
  delay(2000);
  
  // เปิด Relay2
  digitalWrite(RELAY2_PIN, LOW);
  Serial.println("Relay2 ON");
  delay(2000);
  
  // ปิด Relay2
  digitalWrite(RELAY2_PIN, HIGH);
  Serial.println("Relay2 OFF");
  delay(2000);
  
  // เปิด Relay3
  digitalWrite(RELAY3_PIN, LOW);
  Serial.println("Relay3 ON");
  delay(2000);
  
  // ปิด Relay3
  digitalWrite(RELAY3_PIN, HIGH);
  Serial.println("Relay3 OFF");
  delay(2000);
}
```

### ฟังก์ชันช่วยสำหรับการควบคุม Relay

```cpp
// ฟังก์ชันควบคุม Relay แบบ Active Low
void turnOnRelay(int relayPin) {
  digitalWrite(relayPin, LOW);  // LOW = ON (Active Low)
}

void turnOffRelay(int relayPin) {
  digitalWrite(relayPin, HIGH);  // HIGH = OFF (Active Low)
}

// ตัวอย่างการใช้งาน
void exampleUsage() {
  turnOnRelay(RELAY1_PIN);
  delay(1000);
  turnOffRelay(RELAY1_PIN);
}
```

### ข้อควรระวังเกี่ยวกับ Relay

1. **Active Low Logic:** 
   - `LOW` (0V) = Relay ON (เปิด)
   - `HIGH` (3.3V) = Relay OFF (ปิด)

2. **Current Limitation:** GPIO pin ของ ESP32 สามารถจ่ายกระแสได้ประมาณ 12-20 mA เท่านั้น ซึ่งอาจไม่เพียงพอสำหรับการควบคุม Relay โดยตรง อาจต้องใช้ Transistor หรือ Relay Driver Module เพิ่มเติม

3. **Flyback Diode:** หากใช้ Relay Module ต่อพ่วง ตรวจสอบว่ามี Flyback Diode เพื่อป้องกัน Voltage Spike

4. **Initialization:** เสมอตั้งค่า Relay เป็น OFF (HIGH) เมื่อเริ่มต้น เพื่อป้องกันการทำงานที่ไม่คาดหวัง

### แผนวงจร (Circuit Diagram)

```
ESP32 GPIO17 ----[220Ω Resistor]----[Transistor Base/Gate]
                                           |
                                      [Relay Coil]
                                           |
                                      [Flyback Diode]
                                           |
                                          GND
```

## 15. การควบคุม Switch / ปุ่มกด (Switch Control)

บอร์ด ESP32 DevKit V2 นี้รองรับการควบคุม Switch แบบ Active Low ดังนี้:

### ตารางการกำหนด Switch

| Switch | GPIO Pin | สถานะ | Pull-up | หมายเหตุ |
|--------|----------|--------|---------|---------|
| **SW1** | GPIO34 | Active Low | External | ปกติ High, กดลง = Low |
| **SW2** | GPIO35 | Active Low | External | ปกติ High, กดลง = Low |
| **SW3** | GPIO32 | Active Low | External | ปกติ High, กดลง = Low |

### ตัวอย่างการเขียนโปรแกรมอ่าน Switch

```cpp
#include <Arduino.h>

// กำหนด GPIO Pins สำหรับ Switch
#define SWITCH1_PIN 34
#define SWITCH2_PIN 35
#define SWITCH3_PIN 32

// ตัวแปรเก็บสถานะเดิม (สำหรับ Debouncing)
int sw1_prev_state = HIGH;
int sw2_prev_state = HIGH;
int sw3_prev_state = HIGH;

void setup() {
  Serial.begin(115200);
  
  // ตั้งค่า Switch Pins เป็น INPUT
  pinMode(SWITCH1_PIN, INPUT);
  pinMode(SWITCH2_PIN, INPUT);
  pinMode(SWITCH3_PIN, INPUT);
  
  Serial.println("========================================");
  Serial.println("     Switch Control System Started");
  Serial.println("========================================");
  Serial.println("SW1 (GPIO34) - Active Low with Pull-up");
  Serial.println("SW2 (GPIO35) - Active Low with Pull-up");
  Serial.println("SW3 (GPIO32) - Active Low with Pull-up");
  Serial.println("========================================");
}

void loop() {
  // อ่านสถานะ Switch
  int sw1_current = digitalRead(SWITCH1_PIN);
  int sw2_current = digitalRead(SWITCH2_PIN);
  int sw3_current = digitalRead(SWITCH3_PIN);
  
  // ตรวจสอบ SW1
  if (sw1_current != sw1_prev_state) {
    delay(20);  // Debouncing
    sw1_current = digitalRead(SWITCH1_PIN);
    
    if (sw1_current == LOW) {
      Serial.println("[SW1] Button Pressed!");
    } else {
      Serial.println("[SW1] Button Released!");
    }
    sw1_prev_state = sw1_current;
  }
  
  // ตรวจสอบ SW2
  if (sw2_current != sw2_prev_state) {
    delay(20);  // Debouncing
    sw2_current = digitalRead(SWITCH2_PIN);
    
    if (sw2_current == LOW) {
      Serial.println("[SW2] Button Pressed!");
    } else {
      Serial.println("[SW2] Button Released!");
    }
    sw2_prev_state = sw2_current;
  }
  
  // ตรวจสอบ SW3
  if (sw3_current != sw3_prev_state) {
    delay(20);  // Debouncing
    sw3_current = digitalRead(SWITCH3_PIN);
    
    if (sw3_current == LOW) {
      Serial.println("[SW3] Button Pressed!");
    } else {
      Serial.println("[SW3] Button Released!");
    }
    sw3_prev_state = sw3_current;
  }
  
  delay(10);
}
```

### ฟังก์ชันช่วยสำหรับการอ่าน Switch

```cpp
// ฟังก์ชันอ่าน Switch แบบ Active Low (มี Debouncing)
bool isSwitchPressed(int switchPin) {
  if (digitalRead(switchPin) == LOW) {
    delay(20);  // Debouncing
    if (digitalRead(switchPin) == LOW) {
      return true;
    }
  }
  return false;
}

// ฟังก์ชันตรวจสอบการกดปุ่มแบบ One-Shot
bool isSwitchPressedOnce(int switchPin, int& prevState) {
  int currentState = digitalRead(switchPin);
  
  if (currentState != prevState) {
    delay(20);  // Debouncing
    currentState = digitalRead(switchPin);
    
    if (currentState == LOW && prevState == HIGH) {
      prevState = currentState;
      return true;
    }
    prevState = currentState;
  }
  return false;
}

// ตัวอย่างการใช้งาน
void exampleUsage() {
  static int prev_state = HIGH;
  
  if (isSwitchPressedOnce(SWITCH1_PIN, prev_state)) {
    Serial.println("SW1 Detected!");
  }
}
```

### ข้อควรระวังเกี่ยวกับ Switch

1. **Active Low Logic:** 
   - `HIGH` (3.3V) = Switch OFF (ปุ่มไม่ถูกกด)
   - `LOW` (0V) = Switch ON (ปุ่มถูกกด)

2. **External Pull-up:** Switch มี External Pull-up ดังนั้นไม่ต้องเปิด Internal Pull-up
   ```cpp
   pinMode(SWITCH1_PIN, INPUT);  // ไม่ต้องใช้ INPUT_PULLUP
   ```

3. **Debouncing:** ปุ่มกดอาจมี Bouncing (กระดุมแสดงการเปลี่ยนแปลงเร็ว ๆ) ต้องใช้ Debouncing delay ประมาณ 20 ms

4. **GPIO34 & GPIO35 เป็น Input Only:** 
   - GPIO34 และ GPIO35 เป็นขา Input Only ไม่สามารถเป็น Output ได้
   - GPIO32 สามารถเป็น Input หรือ Output ได้

5. **Capacitive Touch Optional:** GPIO34, 35, 32 สามารถใช้เป็น Capacitive Touch Input ได้หากต้องการ

### แผนวงจร (Circuit Diagram)

```
           3.3V
             |
           10K Ω (Pull-up)
             |
    ─────────○─────── GPIO34/35/32
             |
           Switch
             |
            GND
```

### ตัวอย่างการประยุกต์ใช้

```cpp
// ตัวอย่าง: ใช้ Switch เพื่อควบคุม Relay
void setup() {
  pinMode(SWITCH1_PIN, INPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, HIGH);  // ปิด Relay
}

void loop() {
  if (digitalRead(SWITCH1_PIN) == LOW) {
    // ถ้ากดปุ่ม = เปิด Relay
    digitalWrite(RELAY1_PIN, LOW);
    Serial.println("Relay ON");
  } else {
    // ถ้าปล่อยปุ่ม = ปิด Relay
    digitalWrite(RELAY1_PIN, HIGH);
    Serial.println("Relay OFF");
  }
  delay(50);
}
```

---
*เอกสารนี้ถูกจัดทำขึ้นเพื่อใช้ร่วมกับโปรเจกต์ใน PlatformIO*
*ปรับปรุงล่าสุด: 2026-06-13*
