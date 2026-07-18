# Telegram configuration notes

ไฟล์นี้เก็บข้อมูลสำคัญเกี่ยวกับการตั้งค่า Telegram Bot สำหรับโปรเจกต์

## สถานที่ที่ต้องแก้ในโค้ด
- `src/main.cpp`
  - `TELEGRAM_BOT_TOKEN` : ใส่ Bot Token ที่ได้จาก BotFather
  - `TELEGRAM_CHAT_ID` : ใส่หมายเลข Chat ID ของผู้รับ/กลุ่ม

## วิธีสร้าง Bot Token
1. เปิด Telegram และค้นหา `@BotFather`
2. ส่งคำสั่ง `/newbot` แล้วทำตามขั้นตอน
3. BotFather จะส่งข้อความที่มี `Bot Token` ให้เก็บไว้เป็นความลับ

## วิธีหา Chat ID (ง่าย)
1. เริ่มคุยกับบอทของคุณ (ส่งข้อความ `/start`) หรือเพิ่มบอทเข้าไปในกลุ่ม
2. เปิดเบราว์เซอร์แล้วเรียก (แทน `<BOT_TOKEN>` ด้วย token จริง):

```
https://api.telegram.org/bot<BOT_TOKEN>/getUpdates
```

3. หาในผลลัพธ์ JSON ค่า `"chat":{"id": <number>}` นั่นคือ `chat_id` ที่ต้องใช้
- สำหรับ supergroup/ช่อง ค่า `chat_id` อาจอยู่ในรูปแบบ `-1001234567890`

## เก็บค่าคอนฟิกอย่างปลอดภัย
หากต้องการเก็บ `TELEGRAM_BOT_TOKEN` และ `TELEGRAM_CHAT_ID` ให้แยกออกเป็นไฟล์ config ที่ไม่ commit ลง repository เช่น `config.h`

ตัวอย่าง `config.h`:

```cpp
#define OPENWEATHER_API_KEY "YOUR_OPENWEATHER_API_KEY"
#define TELEGRAM_BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define TELEGRAM_CHAT_ID "YOUR_TELEGRAM_CHAT_ID"
```

จากนั้นใน `src/main.cpp` ให้เพิ่ม:

```cpp
#include "config.h"
```

และเพิ่ม `config.h` ลงใน `.gitignore` เพื่อป้องกันการเผยแพร่ค่า secret

## วิธีทดสอบ token
- เรียก:

```
https://api.telegram.org/bot<YOUR_TOKEN>/getMe
```

- ผลที่ได้จะแสดงข้อมูล bot หาก token ถูกต้อง

## ข้อผิดพลาดที่พบบ่อยและการแก้ไข
- `404 Not Found` : มักเกิดเมื่อ URL ผิดรูปแบบ (ตรวจสอบให้แน่ใจว่า URL เป็น `https://api.telegram.org/bot<TOKEN>/...`)
- `400 Bad Request: chat not found` : หมายความว่า `chat_id` ไม่ถูกต้อง หรือบอทยังไม่ได้รับสิทธิ์ส่งข้อความไปยัง chat นั้น
  - แก้ไข: ให้ผู้ใช้เริ่มคุยกับบอท (หรือเพิ่มบอทในกลุ่ม) แล้วเรียก `getUpdates` เพื่อหาค่า `chat.id`
- หากได้ HTTP code อื่น ๆ ให้ดู `response` ใน Serial Log (โค้ดโปรเจกต์แสดง response เมื่อส่งไม่สำเร็จ)

## คำแนะนำด้านความปลอดภัย
- ห้าม commit `TELEGRAM_BOT_TOKEN` ลงใน repository สาธารณะ
- ถ้า token หลุด ให้ไปที่ `@BotFather` และ revoke หรือสร้าง token ใหม่
- พิจารณาเก็บ token/chat_id ในไฟล์ config ภายนอกที่ไม่ commit (เช่น `config.h` ที่ถูกเพิ่มใน `.gitignore`)

## ตัวอย่างค่าใน `src/main.cpp`
```cpp
#define TELEGRAM_BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define TELEGRAM_CHAT_ID "YOUR_TELEGRAM_CHAT_ID"
```

## ขั้นตอนถัดไปที่ผมช่วยได้
- ช่วยลบ token จริงออกจาก `src/main.cpp` และใส่ placeholder
- ช่วยเรียก `getUpdates`/`getMe` หากคุณให้ token ชั่วคราว (แนะนำอย่าส่ง token ผ่านช่องทางสาธารณะ)
- ช่วยแก้ `README.md` หรือไฟล์ config ให้ใช้ตัวแปรแยกสำหรับ token


---
ไฟล์นี้ถูกสร้างเมื่อ 2026-07-18 โดยผู้ช่วย (update project notes).