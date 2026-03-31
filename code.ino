#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <MFRC522.h>

// ==========================================
// 1. CONFIGURATION
// ==========================================
const char* ssid = "SSID";
const char* password = "PASSWD";

// Pushover Credentials
const char* PUSHOVER_USER  = "USER_KEY";  // PASTE USER KEY
const char* PUSHOVER_TOKEN = "TOKEN"; // PASTE API TOKEN

// RFID Setup
String validCard1 = "XX XX XX XX"; 
String validName1 = "Admin One";

String validCard2 = "YY YY YY YY"; 
String validName2 = "Admin Two";

// System State
bool systemArmed = false; 

// ==========================================
// 2. PIN DEFINITIONS
// ==========================================
const int trigPin = 5;    // D1
const int echoPin = 4;    // D2
const int buzzerPin = 16; // D0 (GPIO 16)
#define SS_PIN  15        // D8
#define RST_PIN 0         // D3

// ==========================================
// 3. NOKIA TUNE SETTINGS
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
// 4. GLOBAL OBJECTS
// ==========================================
MFRC522 mfrc522(SS_PIN, RST_PIN);
WiFiClientSecure client;

long duration;
int distance;
const int thresholdDistance = 20; 
bool messageSent = false; 

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // Startup Sound (Buzz First)
  tone(buzzerPin, 1000, 200); 

  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected.");

  client.setInsecure(); // Faster connection

  sendPushover("System Online: Guard Ready");
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

    // Stop card reading momentarily
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    // Process the card
    checkAccess(content);
    
    delay(1000); 
  }

  // ----------------------------------------
  // B. SECURITY LOGIC
  // ----------------------------------------
  if (systemArmed) {
    digitalWrite(trigPin, LOW); delayMicroseconds(2);
    digitalWrite(trigPin, HIGH); delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;

    if (distance > 0 && distance < thresholdDistance) {
      // === INTRUDER SEQUENCE ===
      Serial.println("CHILD DETECTED!");

      // 1. BUZZ FIRST (Continuous Scream)
      // This sound plays WHILE the message is sending
      tone(buzzerPin, 3000); 

      // 2. SEND MESSAGE
      if (!messageSent) {
        sendPushover("🚨 ALERT! CHILD BREACH Distance: " + String(distance) + " cm");
        messageSent = true;
      }

      // 3. PLAY NOKIA TUNE (Blocking Alarm)
      // The 'scream' stops when this melody begins
      playNokiaTune(); 
      
    } else {
      // Clear flag when intruder leaves
      messageSent = false;
      noTone(buzzerPin);
    }
  }
  
  delay(100);
}

// ==========================================
// FUNCTIONS
// ==========================================

void checkAccess(String uid) {
  String personName = "";
  
  if (uid == validCard1) personName = validName1;
  else if (uid == validCard2) personName = validName2;
  
  if (personName != "") {
    // === VALID CARD ===
    systemArmed = !systemArmed;

    if (systemArmed) {
      // 1. BUZZ FIRST (1 Beep)
      beep(1); 
      // 2. SEND MESSAGE
      Serial.println("ARMED by " + personName);
      sendPushover("🛡️ ARMED by " + personName);
    } else {
      // 1. BUZZ FIRST (2 Beeps)
      beep(2);
      // 2. SEND MESSAGE
      Serial.println("DISARMED by " + personName);
      sendPushover("✅ DISARMED by " + personName);
    }
  } else {
    // === UNKNOWN CARD ===
    // 1. BUZZ FIRST (Error Sound)
    tone(buzzerPin, 100); 
    
    // 2. SEND MESSAGE (While buzzing)
    Serial.println("Unknown Card: " + uid);
    sendPushover("🚫 Unauthorized Card: " + uid);

    // Stop buzzing after message sent + small delay
    delay(500); 
    noTone(buzzerPin);
  }
}

void sendPushover(String message) {
  if (client.connect("api.pushover.net", 443)) {
    String postData = "token=" + String(PUSHOVER_TOKEN) + 
                      "&user=" + String(PUSHOVER_USER) + 
                      "&message=" + message;
                      
    client.println("POST /1/messages.json HTTP/1.1");
    client.println("Host: api.pushover.net");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.println(postData);
    
    // Quickly flush response to unblock
    while(client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break;
    }
    client.stop();
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