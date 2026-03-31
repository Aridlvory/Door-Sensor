#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>

// ==========================================
// 1. RFID & SECURITY CONFIGURATION
// ==========================================
// REPLACE these with your actual Card IDs
String validCard1 = "FD 13 14 05"; 
String validName1 = "Admin One";

String validCard2 = "YY YY YY YY"; 
String validName2 = "Admin Two";

// System State
bool systemArmed = false; 

// ==========================================
// 2. NETWORK & TELEGRAM DETAILS
// ==========================================
const char* ssid = "SSID";       
const char* password = "PASSWD"; 

#define BOT_TOKEN "123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11" 
#define CHAT_ID "-123456789" 

// ==========================================
// 3. PIN DEFINITIONS
// ==========================================
const int trigPin = 5;    // D1
const int echoPin = 4;    // D2
const int buzzerPin = 16; // D0 
#define SS_PIN  15        // D8 
#define RST_PIN 0         // D3 

// ==========================================
// 4. MUSIC NOTES (NOKIA TUNE)
// ==========================================
#define NOTE_E5  659
#define NOTE_D5  587
#define NOTE_FS4 370
#define NOTE_GS4 415
#define NOTE_CS5 554
#define NOTE_B4  494
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_A4  440
#define NOTE_CS4 277

int tempo = 180;
int melody[] = {
  NOTE_E5, 8, NOTE_D5, 8, NOTE_FS4, 4, NOTE_GS4, 4, 
  NOTE_CS5, 8, NOTE_B4, 8, NOTE_D4, 4, NOTE_E4, 4, 
  NOTE_B4, 8, NOTE_A4, 8, NOTE_CS4, 4, NOTE_E4, 4,
  NOTE_A4, 2, 
};
int notes = sizeof(melody) / sizeof(melody[0]) / 2;
int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;

// ==========================================
// 5. OBJECTS & VARIABLES
// ==========================================
long duration;
int distance;
const int thresholdDistance = 20; 

bool messageSent = false; 

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
MFRC522 mfrc522(SS_PIN, RST_PIN); 

void setup() {
  Serial.begin(115200);
  SPI.begin();         
  mfrc522.PCD_Init();  
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  client.setInsecure();
  
  // NOTE: Sending a startup message here will delay the loop slightly, that is normal.
  bot.sendMessage(CHAT_ID, "System Power On. Status: DISARMED.", "");
  Serial.println("System Ready.");
}

void loop() {
  // ----------------------------------------
  // A. CHECK RFID
  // ----------------------------------------
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String content = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    content = content.substring(1); 

    Serial.print("Scanned Card: ");
    Serial.println(content);

    checkAccess(content);
    
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    delay(1000); 
  }

  // ----------------------------------------
  // B. IF ARMED -> CHECK ULTRASONIC
  // ----------------------------------------
  if (systemArmed) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;

    if (distance > 0 && distance < thresholdDistance) {
      // === INTRUDER DETECTED ===
      Serial.print("Intruder! Distance: ");
      Serial.println(distance);
      
      // 1. BUZZ FIRST (Immediate Alarm)
      playNokiaTune(); 
      
      // 2. SEND MESSAGE SECOND (Only if not sent yet)
      if (!messageSent) {
        bot.sendMessage(CHAT_ID, "🚨 INTRUDER ALERT! Motion detected at " + String(distance) + " cm.", "");
        messageSent = true;
      }

    } else {
      // === ALL CLEAR ===
      messageSent = false; 
    }
  } 
  
  delay(100); 
}

// ==========================================
// HELPER FUNCTIONS
// ==========================================

void checkAccess(String uid) {
  String personName = "";
  
  if (uid == validCard1) personName = validName1;
  else if (uid == validCard2) personName = validName2;
  
  if (personName != "") {
    // === VALID CARD ===
    systemArmed = !systemArmed; 

    if (systemArmed) {
      Serial.println("System ARMED by " + personName);
      // Beep First
      beep(1); 
      // Send Message Second
      bot.sendMessage(CHAT_ID, "🛡️ System ARMED by " + personName, "");
    } else {
      Serial.println("System DISARMED by " + personName);
      // Beep First
      beep(2);
      // Send Message Second
      bot.sendMessage(CHAT_ID, "✅ System DISARMED by " + personName, "");
    }
  } else {
    // === UNKNOWN CARD ===
    Serial.println("Access Denied! Unknown Card: " + uid);
    
    // Beep First (Error Sound)
    tone(buzzerPin, 100); 
    delay(1000);
    noTone(buzzerPin);

    // Send Message Second
    String msg = "🚫 WARNING: Unauthorized Access Attempt!\nCard ID: " + uid;
    bot.sendMessage(CHAT_ID, msg, "");
  }
}

void beep(int times) {
  for(int i=0; i<times; i++) {
    tone(buzzerPin, 2000);
    delay(100);
    noTone(buzzerPin);
    delay(100);
  }
}

void playNokiaTune() {
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
    divider = melody[thisNote + 1];
    if (divider > 0) {
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; 
    }
    tone(buzzerPin, melody[thisNote], noteDuration * 0.9);
    delay(noteDuration);
    noTone(buzzerPin);
  }
}