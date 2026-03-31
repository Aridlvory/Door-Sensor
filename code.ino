/*
 * ESP8266 Ultrasonic Sensor & Buzzer Project
 * 
 * Pin Connections:
 * HC-SR04 Trig -> D1 (GPIO 5)
 * HC-SR04 Echo -> D2 (GPIO 4)
 * Buzzer (+)   -> D5 (GPIO 14)
 * 
 * Logic:
 * If an object is detected closer than the 'thresholdDistance' (e.g., 20cm),
 * the buzzer will turn ON.
 */

// Define Pins
const int trigPin = 5;  // D1
const int echoPin = 4;  // D2
const int buzzerPin = 14; // D5

// Variables for measurement
long duration;
int distance;

// Threshold distance in cm to trigger buzzer
const int thresholdDistance = 20; 

void setup() {
  // Start Serial Monitor
  Serial.begin(115200); 

  // Set pin modes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  // Ensure buzzer is off initially
  digitalWrite(buzzerPin, LOW);
  
  Serial.println("System Initialized");
}

void loop() {
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10 microsecond pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the incoming pulse on Echo pin
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance: Distance = (Time * Speed of Sound) / 2
  // Speed of sound is approx 0.034 cm/microsecond
  distance = duration * 0.034 / 2;

  // Print distance to Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Logic to control the Buzzer
  if (distance > 0 && distance < thresholdDistance) {
    // Object detected within range!
    digitalWrite(buzzerPin, HIGH); // Turn Buzzer ON
    Serial.println("Object Detected! Buzzer ON");
  } else {
    // No object or out of range
    digitalWrite(buzzerPin, LOW);  // Turn Buzzer OFF
  }

  // specific delay to avoid spamming the sensor
  delay(100); 
}