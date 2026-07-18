# Project_01 - ESP32 Weather, Relay, Switch และ OLED Dashboard

โปรเจกต์นี้เป็นโปรแกรมสำหรับบอร์ด **ESP32 DevKit** ที่พัฒนาด้วย **PlatformIO + Arduino Framework** ใช้สำหรับควบคุม Relay 3 ช่องด้วย Switch 3 ปุ่ม พร้อมเชื่อมต่อ WiFi ผ่าน WiFiManager, ดึงข้อมูลสภาพอากาศจาก OpenWeather และแสดงผลบนจอ **OLED 0.96 นิ้ว I2C**

## สารบัญ

- [ภาพรวมของโปรเจกต์](#ภาพรวมของโปรเจกต์)
- [Hardware ที่ใช้](#hardware-ที่ใช้)
- [Pin Mapping](#pin-mapping)
- [Library ที่เกี่ยวข้อง](#library-ที่เกี่ยวข้อง)
- [โครงสร้างไฟล์สำคัญ](#โครงสร้างไฟล์สำคัญ)
- [การตั้งค่า PlatformIO](#การตั้งค่า-platformio)
- [วิธีเปิดโปรแกรมด้วย VS Code](#วิธีเปิดโปรแกรมด้วย-vs-code)
- [วิธี Build](#วิธี-build)
- [วิธี Upload ลง ESP32](#วิธี-upload-ลง-esp32)
- [วิธีเปิด Serial Monitor](#วิธีเปิด-serial-monitor)
- [วิธีใช้งาน WiFiManager](#วิธีใช้งาน-wifimanager)
- [การ Reset WiFi ด้วย SW1](#การ-reset-wifi-ด้วย-sw1)
- [การแสดงผลบน OLED](#การแสดงผลบน-oled)
- [การแจ้งเตือนผ่าน Telegram](#การแจ้งเตือนผ่าน-telegram)
- [OpenWeather API](#openweather-api)
- [Logic การทำงานของ Relay และ Switch](#logic-การทำงานของ-relay-และ-switch)
- [ข้อควรระวังด้าน Hardware](#ข้อควรระวังด้าน-hardware)
- [Troubleshooting](#troubleshooting)
- [สถานะล่าสุดของโปรเจกต์](#สถานะล่าสุดของโปรเจกต์)
- [คำสั่งที่ใช้บ่อย](#คำสั่งที่ใช้บ่อย)
- [หมายเหตุ](#หมายเหตุ)

## ภาพรวมของโปรเจกต์

ระบบนี้ทำงานหลัก ๆ ดังนี้:

1. ควบคุม Relay 3 ตัวแบบ Active Low
2. อ่านปุ่ม Switch 3 ตัวแบบ Active Low พร้อม debounce
3. ใช้ SW1 สำหรับ reset การตั้งค่า WiFi เมื่อกดค้าง 5 วินาที
4. ตั้งค่า WiFi ผ่าน WiFiManager โดยไม่ต้อง hard-code SSID/password ในโปรแกรม
5. ดึงข้อมูลจาก OpenWeather API สำหรับจังหวัดเชียงใหม่
6. แสดงข้อมูลบน OLED:
   - Temp
   - Hum
   - AQI
   - PM2.5
   - สถานะ Relay 1-3
   - สถานะระหว่าง setup/connect WiFi
7. แจ้งเตือนสถานะสำคัญไปยัง Telegram
8. แสดง log การทำงานผ่าน Serial Monitor ที่ `115200`

## Hardware ที่ใช้

- ESP32 DevKit / DOIT ESP32 DEVKIT V1 compatible board
- Relay module 3 ช่อง
- Switch / Push button 3 ตัว
- OLED 0.96 นิ้ว I2C ความละเอียด 128x64, controller SSD1306
- สาย jumper และแหล่งจ่ายไฟที่เหมาะสม

## Pin Mapping

### Relay

Relay เป็นแบบ **Active Low**

| Relay | ESP32 GPIO | Logic |
|---|---:|---|
| Relay 1 | GPIO17 | LOW = ON, HIGH = OFF |
| Relay 2 | GPIO16 | LOW = ON, HIGH = OFF |
| Relay 3 | GPIO4 | LOW = ON, HIGH = OFF |

เมื่อเริ่มต้นระบบ โปรแกรมจะสั่ง Relay ทุกช่องเป็น `HIGH` เพื่อปิด Relay ทั้งหมดก่อน

### Switch

Switch เป็นแบบ **Active Low**

| Switch | ESP32 GPIO | หน้าที่ |
|---|---:|---|
| SW1 | GPIO34 | Toggle Relay 1 และกดค้าง 5 วินาทีเพื่อ reset WiFi |
| SW2 | GPIO35 | Toggle Relay 2 |
| SW3 | GPIO32 | Toggle Relay 3 |

หมายเหตุ:
- GPIO34 และ GPIO35 เป็น input-only ใช้เป็น input ได้อย่างเดียว
- โค้ดตั้ง `pinMode(..., INPUT)` เพราะออกแบบให้มี external pull-up
- ปุ่มกดทำงานแบบ Active Low คือ `HIGH = ไม่กด`, `LOW = กด`

### OLED 0.96 I2C

| OLED Pin | ESP32 Pin |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO21 |
| SCL | GPIO22 |

ค่า I2C address ในโปรแกรม:

```cpp
#define OLED_ADDRESS 0x3C
```

ถ้าจอไม่แสดงผล ให้ลองเปลี่ยนเป็น `0x3D` ใน `src/main.cpp`

## Library ที่เกี่ยวข้อง

กำหนดไว้ใน `platformio.ini`

```ini
lib_deps =
	tzapu/WiFiManager@^2.0.13-alpha
	bblanchon/ArduinoJson@^7.0.4
	adafruit/Adafruit SSD1306@^2.5.13
	adafruit/Adafruit GFX Library@^1.12.1
```

รายละเอียด:

| Library | หน้าที่ |
|---|---|
| WiFiManager | เปิดหน้า Config Portal สำหรับตั้งค่า WiFi |
| ArduinoJson | parse JSON จาก OpenWeather API |
| Adafruit SSD1306 | ควบคุมจอ OLED SSD1306 |
| Adafruit GFX Library | วาด text, line, shape บน OLED |
| WiFi / HTTPClient / WiFiClientSecure / Wire | library ของ Arduino ESP32 framework |

## โครงสร้างไฟล์สำคัญ

```text
Project_01/
├── platformio.ini
├── README.md
├── ESP32DevkitBoard.md
├── src/
│   └── main.cpp
├── include/
├── lib/
└── test/
```

ไฟล์หลัก:

- `src/main.cpp` - โปรแกรมหลักทั้งหมด
- `platformio.ini` - การตั้งค่า board, framework, upload, monitor และ library
- `ESP32DevkitBoard.md` - เอกสารข้อมูลบอร์ด ESP32 และการต่ออุปกรณ์
- `README.md` - เอกสารอธิบายโปรเจกต์นี้

## การตั้งค่า PlatformIO

ค่า config ปัจจุบัน:

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
upload_port = COM6
upload_speed = 115200
```

หมายเหตุ:
- ถ้า ESP32 ของคุณไม่ได้อยู่ที่ `COM4` ให้แก้ `upload_port` ให้ตรงกับเครื่อง เช่น `COM3`, `COM5`
- ใช้ `upload_speed = 115200` เพื่อเพิ่มความเสถียรในการ upload

ปัจจุบันบนเครื่องนี้ตั้งค่าเป็น `COM6` (อัปเดต: 2026-07-18)

## วิธีเปิดโปรแกรมด้วย VS Code

1. เปิด **Visual Studio Code**
2. ติดตั้ง extension **PlatformIO IDE**
3. เลือก `File > Open Folder...`
4. เปิดโฟลเดอร์:

```text
C:\Users\Dell\Documents\PlatformIO\Projects\Project_01
```

5. รอ PlatformIO โหลด project และติดตั้ง dependencies
6. เปิดไฟล์ `src/main.cpp` เพื่อดูหรือแก้ไขโปรแกรม
7. เปิดไฟล์ `platformio.ini` เพื่อตรวจ board, port และ library

## วิธี Build

ใน VS Code:

1. กดไอคอน PlatformIO ด้านซ้าย
2. เลือก `PROJECT TASKS`
3. เลือก environment `esp32doit-devkit-v1`
4. กด `Build`

หรือใช้ terminal:

```powershell
C:\Users\Dell\.platformio\penv\Scripts\platformio.exe run
```

## วิธี Upload ลง ESP32

1. ต่อ ESP32 เข้าคอมพิวเตอร์ด้วยสาย USB
2. ตรวจสอบว่า `upload_port` ใน `platformio.ini` ตรงกับพอร์ตจริง
3. ปิด Serial Monitor ก่อน upload
4. กด `Upload` ใน PlatformIO

หรือใช้ terminal:

```powershell
C:\Users\Dell\.platformio\penv\Scripts\platformio.exe run --target upload
```

ถ้า upload ไม่ผ่านและขึ้นว่า `COM4 busy` หรือ `Access is denied`:

- ปิด Serial Monitor ใน VS Code
- ปิด Arduino IDE, PuTTY หรือโปรแกรมอื่นที่ใช้ COM port
- ถอดสาย USB แล้วเสียบใหม่
- ลองกดปุ่ม BOOT ค้างระหว่าง upload แล้วปล่อยเมื่อเริ่มเชื่อมต่อ

## วิธีเปิด Serial Monitor

ใน PlatformIO เลือก `Monitor`

หรือใช้ terminal:

```powershell
C:\Users\Dell\.platformio\penv\Scripts\platformio.exe device monitor -b 115200
```

Serial Monitor ใช้ความเร็ว:

```text
115200 baud
```

## วิธีใช้งาน WiFiManager

เมื่อเปิดเครื่อง โปรแกรมจะพยายามเชื่อมต่อ WiFi ที่เคยบันทึกไว้

ถ้ายังไม่เคยตั้งค่า WiFi หรือเชื่อมต่อไม่ได้:

1. ESP32 จะเปิด Access Point ชื่อ:

```text
ESP32_CONFIG
```

2. รหัสผ่าน:

```text
12345678
```

3. ใช้มือถือหรือคอมพิวเตอร์เชื่อมต่อ WiFi ชื่อนี้
4. เปิดหน้า config portal ที่ WiFiManager แสดงขึ้น
5. เลือก WiFi บ้าน/สำนักงาน แล้วกรอกรหัสผ่าน
6. ESP32 จะบันทึกค่าและเชื่อมต่อ WiFi อัตโนมัติ

## การ Reset WiFi ด้วย SW1

มี 2 กรณีที่สามารถ reset WiFi ได้

### 1. กด SW1 ค้างตอนเริ่มเปิดเครื่อง

1. กด SW1 ค้างไว้
2. เปิดเครื่องหรือ reset ESP32
3. OLED จะแสดง countdown
4. กดค้างให้ครบ 5 วินาที
5. โปรแกรมจะลบค่า WiFi ที่บันทึกไว้
6. ESP32 restart ใหม่
7. เข้าสู่โหมด WiFiManager เพื่อ config ใหม่

ถ้าปล่อย SW1 ก่อนครบ 5 วินาที โปรแกรมจะยกเลิกการ reset และ boot ต่อ

### 2. กด SW1 ค้างระหว่างโปรแกรมทำงาน

กด SW1 ค้าง 5 วินาที โปรแกรมจะ reset WiFi settings และ restart เช่นกัน

หมายเหตุ: SW1 ยังใช้กดสั้นเพื่อ toggle Relay 1 ด้วย

## การแสดงผลบน OLED

ระหว่าง setup/connect WiFi OLED จะแสดงสถานะ เช่น:

- `System Boot`
- `Hold SW1 5 sec`
- `WiFi Setup`
- `Connecting...`
- `Config Portal`
- `WiFi Connected`
- `WiFi Failed`
- `WiFi Reset`

เมื่อเข้าสู่หน้าจอหลัก OLED จะแสดง:

```text
Chiang Mai        WiFi/AP
Temp xx.xC   Hum xx%
AQI x label  PM2.5 xx
R1 ON/OFF  R2 ON/OFF  R3 ON/OFF
```

Relay ที่เป็น ON จะแสดงเป็นกล่องพื้นขาว ตัวอักษรสีดำ ส่วน OFF จะแสดงเป็นกรอบ

## การแจ้งเตือนผ่าน Telegram

โปรแกรมรองรับการส่งข้อความแจ้งเตือนไปยัง Telegram ผ่าน Telegram Bot API โดยไม่ต้องติดตั้ง library เพิ่ม ใช้ `HTTPClient` และ `WiFiClientSecure` ของ ESP32

### สิ่งที่ Telegram จะแจ้งเตือน

- ESP32 เริ่มทำงานและเชื่อมต่อ WiFi สำเร็จ
- มีการ reset การตั้งค่า WiFi ด้วย SW1
- Relay 1, Relay 2, Relay 3 เปลี่ยนสถานะ
- รายงานข้อมูล OpenWeather ทุกครั้งที่มีการอัปเดตข้อมูล:
  - Temp
  - Hum
  - AQI
  - PM2.5
  - สถานะ Relay ทั้ง 3 ช่อง

### วิธีสร้าง Telegram Bot

1. เปิด Telegram แล้วค้นหา `@BotFather`
2. ส่งคำสั่ง `/newbot`
3. ตั้งชื่อ bot และ username ตามขั้นตอน
4. BotFather จะส่ง **Bot Token** มาให้
5. เก็บ token นี้ไว้ใช้ใน `src/main.cpp`

### วิธีหา Chat ID

1. ส่งข้อความหา bot ของคุณอย่างน้อย 1 ข้อความ
2. เปิด URL นี้ใน browser โดยเปลี่ยน `<BOT_TOKEN>` เป็น token จริง:

```text
https://api.telegram.org/bot<BOT_TOKEN>/getUpdates
```

3. มองหา `"chat":{"id":...}` ค่าเลข `id` คือ Chat ID

### วิธีเปิดใช้งาน Telegram ในโปรแกรม

แก้ค่าใน `src/main.cpp`:

```cpp
#define TELEGRAM_BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define TELEGRAM_CHAT_ID "YOUR_TELEGRAM_CHAT_ID"
```

ตัวอย่าง:

```cpp
#define TELEGRAM_BOT_TOKEN "123456789:ABCDEFxxxxxxxxxxxxxxxx"
#define TELEGRAM_CHAT_ID "123456789"
```

ถ้ายังไม่ได้แก้ 2 ค่านี้ โปรแกรมจะไม่ส่ง Telegram และจะแสดง log:

```text
[Telegram] Skipped: Bot token or chat ID is not configured
```

ข้อควรระวัง:

- อย่าเผยแพร่ Bot Token ลง GitHub หรือส่งต่อให้ผู้อื่น
- ถ้า token หลุด ให้ไปที่ BotFather แล้ว revoke token ใหม่
- Telegram จะส่งได้เมื่อ ESP32 เชื่อมต่อ WiFi และออกอินเทอร์เน็ตได้แล้วเท่านั้น

### ปัญหาที่พบบ่อย: "Bad Request: chat not found"

- สาเหตุที่พบบ่อย: `TELEGRAM_CHAT_ID` ผิดหรือบอทยังไม่ได้เริ่มการสนทนากับผู้รับ
- วิธีแก้ไข:
  1. ถ้าเป็นแชทส่วนตัว ให้ผู้ใช้เริ่มคุยกับบอทก่อน (ส่งข้อความ `/start`) แล้วเรียก
    `https://api.telegram.org/bot<BOT_TOKEN>/getUpdates` เพื่อดู `chat.id`
  2. ถ้าเป็นกลุ่ม ให้เพิ่มบอทเข้าไปในกลุ่มก่อน และใช้ `getUpdates` เพื่อหาค่า `chat.id`
    - สำหรับ supergroup/ช่อง ค่า `chat_id` อาจมีรูปแบบ `-100xxxxxxxxxx`
  3. หากต้องการทดสอบเบื้องต้น ให้เรียก `getMe` เพื่อยืนยันว่า token ถูกต้อง:

```
https://api.telegram.org/bot<YOUR_TOKEN>/getMe
```

หลังแก้ค่า `TELEGRAM_BOT_TOKEN` และ `TELEGRAM_CHAT_ID` ให้ทดลองรันและดู Serial Log
เพื่อยืนยันว่าการส่งสำเร็จ (จะเห็นสถานะ `[Telegram] Message sent`)

## OpenWeather API

โปรแกรมดึงข้อมูลจาก:

- Current Weather API
- Air Pollution API

ตำแหน่งที่ตั้งค่าไว้:

```cpp
#define CITY_NAME "Chiang Mai"
#define CITY_LAT 18.7883
#define CITY_LON 98.9853
```

ช่วงเวลาอัปเดต:

```cpp
#define WEATHER_UPDATE_INTERVAL 60000
```

คือทุก 60 วินาที

ข้อมูลที่ใช้งาน:

- Temperature
- Humidity
- AQI
- PM2.5
- PM10 ใช้ใน Serial Monitor แต่ไม่ได้แสดงบน OLED

ข้อสำคัญ:
- ใน `src/main.cpp` มี API key ของ OpenWeather อยู่ในโค้ด
- หากนำโปรเจกต์ไปเผยแพร่ ควรเปลี่ยน API key หรือย้ายไปเก็บในไฟล์ config ที่ไม่ commit ขึ้น repository

## Logic การทำงานของ Relay และ Switch

Switch ใช้ edge detection พร้อม debounce:

- กด SW1 สั้น: toggle Relay 1
- กด SW2 สั้น: toggle Relay 2
- กด SW3 สั้น: toggle Relay 3
- กด SW1 ค้าง 5 วินาที: reset WiFi

Relay เป็น Active Low:

```text
LOW  = Relay ON
HIGH = Relay OFF
```

## ข้อควรระวังด้าน Hardware

1. ESP32 เป็น logic 3.3V ห้ามป้อนสัญญาณ 5V เข้า GPIO โดยตรง
2. ถ้าใช้ relay module 5V ให้ตรวจสอบว่า input รองรับ 3.3V หรือไม่
3. GPIO34 และ GPIO35 ไม่มี internal pull-up/pull-down แบบ GPIO ทั่วไป ควรมี external pull-up
4. OLED แนะนำต่อ VCC กับ 3V3
5. ถ้าจอ OLED ไม่ขึ้น ให้ตรวจ wiring และลองเปลี่ยน address เป็น `0x3D`
6. อย่าใช้ Serial Monitor พร้อมกับ Upload เพราะจะทำให้ COM port busy

## Troubleshooting

### Upload ไม่ผ่าน: COM port busy

ข้อความตัวอย่าง:

```text
Could not open COM4, the port is busy or doesn't exist.
PermissionError(13, 'Access is denied.')
```

วิธีแก้:

- ปิด Serial Monitor
- ปิดโปรแกรมอื่นที่ใช้ COM port
- ถอดเสียบ USB ใหม่
- ตรวจ `upload_port`
- กด BOOT ค้างระหว่าง upload ถ้าบอร์ดไม่เข้า bootloader อัตโนมัติ

### OLED ไม่แสดงผล

ตรวจสอบ:

- VCC ต่อ 3V3
- GND ต่อ GND
- SDA ต่อ GPIO21
- SCL ต่อ GPIO22
- Address เป็น `0x3C` หรือ `0x3D`
- Library ติดตั้งครบ

### WiFi ไม่เชื่อมต่อ

วิธีแก้:

- กด SW1 ค้าง 5 วินาทีเพื่อ reset WiFi
- เชื่อมต่อ AP `ESP32_CONFIG`
- ตั้งค่า WiFi ใหม่
- ตรวจว่า router อยู่ในระยะสัญญาณ

### Relay ทำงานกลับด้าน

โปรเจกต์นี้ออกแบบสำหรับ Relay แบบ Active Low ถ้า relay module ของคุณเป็น Active High ต้องแก้ logic ใน `toggleRelay()`

## สถานะล่าสุดของโปรเจกต์

ฟีเจอร์ที่มีแล้ว:

- Relay control 3 ช่อง
- Switch input 3 ปุ่ม
- SW1 reset WiFi แบบกดค้าง 5 วินาที
- WiFiManager config portal
- OLED dashboard
- OLED setup/connect status
- Telegram notification
- OpenWeather weather + AQI
- PlatformIO build ผ่าน

สถานะปัจจุบัน (อัปเดต: 2026-07-18):

- `upload_port` ถูกตั้งเป็น `COM6` ใน `platformio.ini` (เครื่องนี้)
- พยายามอัปโหลด: Build สำเร็จ แต่การอัปโหลดล้มเหลวเพราะพอร์ต `COM6` ถูกล็อค/ถูกปฏิเสธสิทธิ์ (ลองปิด Serial Monitor หรือเรียก VS Code เป็นผู้ดูแลหรือถอดสาย USB แล้วเสียบใหม่)
- การส่ง Telegram: โค้ดแก้ไขให้สร้าง URL ถูกต้องแล้ว แต่ได้รับข้อผิดพลาด `400 Bad Request: chat not found` — แก้ที่ `TELEGRAM_CHAT_ID` หรือให้ผู้ใช้เริ่มคุยกับบอท/เพิ่มบอทเข้าไปในกลุ่ม

## คำสั่งที่ใช้บ่อย

Build:

```powershell
C:\Users\Dell\.platformio\penv\Scripts\platformio.exe run
```

Upload:

```powershell
C:\Users\Dell\.platformio\penv\Scripts\platformio.exe run --target upload
```

Serial Monitor:

```powershell
C:\Users\Dell\.platformio\penv\Scripts\platformio.exe device monitor -b 115200
```

## หมายเหตุ

เอกสารนี้อ้างอิงจากโค้ดปัจจุบันใน `src/main.cpp` และการตั้งค่าปัจจุบันใน `platformio.ini`
