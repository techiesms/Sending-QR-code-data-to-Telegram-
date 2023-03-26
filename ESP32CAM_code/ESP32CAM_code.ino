#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#include <Arduino.h>
#include <ESP32QRCodeReader.h>

#define BUZ 12 // Buzzer will buzz when it scans a vaild QR Code 
#define LED 4 // Builtin LED will flash as soon as the data is sent to Telegram Bot
#define Internet_LED 15 // This LED Indicated whether our ESP32 CAM Board is connected with Internet or Not

const char* ssid = "SmS_jiofi";
const char* password = "sms123458956";

#define BOTtoken "Your_BOT_Token"
#define CHAT_ID "Your_Chat_ID"

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

String current, previous;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void onQrCodeTask(void* pvParameters) {
  struct QRCodeData qrCodeData;

  while (true)
  {
    if (reader.receiveQrCode(&qrCodeData, 100))
    {
      Serial.println("Found QRCode");
      if (qrCodeData.valid)
      {
        Serial.print("Payload: ");
        Serial.println((const char*)qrCodeData.payload);
        char* Message = (char*)qrCodeData.payload;
        current = (String)Message;


        if (current == previous)
        {
          Serial.println("Already Scanned");
          digitalWrite(BUZ, 1);
          delay(2000);
          digitalWrite(BUZ, 0);
        }

        else
        {
          digitalWrite(BUZ, 1);
          delay(250);
          digitalWrite(BUZ, 0);
          Serial.println("Sending to telegram");
          const char* myDomain = "api.telegram.org";
          String message = (String)Message;
          bot.sendMessage(CHAT_ID, message, "");
          digitalWrite(LED, 1);
          delay(500);
          digitalWrite(LED, 0);
          previous = current;
        }
      }
      else
      {
        Serial.print("Invalid: ");
        Serial.println((const char*)qrCodeData.payload);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(LED, OUTPUT);
  pinMode(BUZ, OUTPUT);
  pinMode(Internet_LED, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

  }
  reader.setup();
  Serial.println("Setup QRCode Reader");
  reader.beginOnCore(1);
  Serial.println("Begin on Core 1");
  xTaskCreate(onQrCodeTask, "onQrCode", 8 * 1024, NULL, 4, NULL);
  digitalWrite(Internet_LED, 0);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(Internet_LED, HIGH);
  }
  else
  {
    digitalWrite(Internet_LED, LOW);
  }

  delay(100);
}
