#include <Servo.h>

// ------------------- FLAME SENSORS -------------------
#define flameLeft   A0
#define flameCenter A1
#define flameRight  A2

// ------------------- ULTRASONIC ----------------------
#define trigPin 8
#define echoPin 9

// ------------------- TEMPERATURE ---------------------
#define tempPin A3   // LM35 (not used in logic now)

// ------------------- MOTOR DRIVER (L298N) ------------
#define ENA 5
#define IN1 2
#define IN2 3
#define IN3 4
#define IN4 7
#define ENB 6

// ------------------- RELAY ---------------------------
#define relayPin 10

// ------------------- SERVO ---------------------------
#define servoPin 11

// ------------------- VARIABLES -----------------------
Servo waterServo;

const int MOTOR_SPEED_FORWARD = 150;
const int MOTOR_SPEED_TURN    = 110;   // slower when turning

// Flame thresholds (tune these after testing!)
const int FIRE_DETECTED_THRESHOLD = 650;   // below = fire somewhere
const int FIRE_CLOSE_THRESHOLD    = 320;   // below = close enough to spray

// Timings
const unsigned long SPRAY_COOLDOWN_MS   = 12000;   // min time between sprays
const unsigned long MAX_SPRAY_DURATION  = 7000;    // safety timeout

unsigned long lastSprayTime = 0;

// ===================================================
//                  SETUP
// ===================================================
void setup() {
Serial.begin(9600);

// Flame sensors
pinMode(flameLeft,   INPUT);
pinMode(flameCenter, INPUT);
pinMode(flameRight,  INPUT);

// Ultrasonic
pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT);

// Motors
pinMode(ENA, OUTPUT);
pinMode(IN1, OUTPUT);
pinMode(IN2, OUTPUT);
pinMode(IN3, OUTPUT);
pinMode(IN4, OUTPUT);
pinMode(ENB, OUTPUT);

// Relay (assuming active LOW = ON)
pinMode(relayPin, OUTPUT);
digitalWrite(relayPin, HIGH);  // pump OFF

// Servo
waterServo.attach(servoPin);
delay(200);           // give servo time to initialize
waterServo.write(90); // center position

stopMotors();
Serial.println("Fire Fighting Robot - Ready");
Serial.println("Flame thresholds: detect < " + String(FIRE_DETECTED_THRESHOLD) +
" | close < " + String(FIRE_CLOSE_THRESHOLD));
}

// ===================================================
//                  MAIN LOOP
// ===================================================
void loop() {
// Read sensors
int left   = readFlame(flameLeft);
int center = readFlame(flameCenter);
int right  = readFlame(flameRight);
long distance = getDistance();

// Optional debug output (uncomment when tuning)
// Serial.print("L:"); Serial.print(left);
// Serial.print(" C:"); Serial.print(center);
// Serial.print(" R:"); Serial.print(right);
// Serial.print(" Dist:"); Serial.println(distance);

// ================= PRIORITY 1: OBSTACLE AVOIDANCE =================
if (distance < 22 && distance > 0) {   // 22 cm safety margin
stopMotors();
delay(150);
moveBackward();
delay(600);               // back up first
turnRight();              // then turn
delay(900);               // longer turn helps escape corners
stopMotors();
delay(200);
return;
}

// ================= PRIORITY 2: EXTINGUISH FIRE =================
if (center < FIRE_CLOSE_THRESHOLD) {
if (millis() - lastSprayTime >= SPRAY_COOLDOWN_MS) {
stopMotors();
delay(300);             // let robot settle
sprayWater();
lastSprayTime = millis();
} else {
stopMotors();           // wait out cooldown
}
return;
}

// ================= PRIORITY 3: MOVE TOWARD FIRE =================
if (center < FIRE_DETECTED_THRESHOLD) {
moveForward();
}
else if (left < FIRE_DETECTED_THRESHOLD) {
turnLeft();
}
else if (right < FIRE_DETECTED_THRESHOLD) {
turnRight();
}
else {
stopMotors();
}
}

// ===================================================
//                  HELPER FUNCTIONS
// ===================================================

// Read flame sensor with averaging (reduces noise)
int readFlame(int pin) {
int sum = 0;
const int samples = 8;
for (int i = 0; i < samples; i++) {
sum += analogRead(pin);
delay(2);
}
return sum / samples;
}

// Ultrasonic distance (cm) with timeout protection
long getDistance() {
digitalWrite(trigPin, LOW);
delayMicroseconds(3);
digitalWrite(trigPin, HIGH);
delayMicroseconds(11);
digitalWrite(trigPin, LOW);

long duration = pulseIn(echoPin, HIGH, 30000); // ~5m max
if (duration == 0) return 999;                 // timeout → very far
return duration * 0.034 / 2;
}

// Motor control functions
void moveForward() {
analogWrite(ENA, MOTOR_SPEED_FORWARD);
analogWrite(ENB, MOTOR_SPEED_FORWARD);
digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnLeft() {
analogWrite(ENA, MOTOR_SPEED_TURN);
analogWrite(ENB, MOTOR_SPEED_TURN);
digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);  // left backward
digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // right forward
}

void turnRight() {
analogWrite(ENA, MOTOR_SPEED_TURN);
analogWrite(ENB, MOTOR_SPEED_TURN);
digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // left forward
digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);  // right backward
}

void moveBackward() {
analogWrite(ENA, MOTOR_SPEED_TURN);
analogWrite(ENB, MOTOR_SPEED_TURN);
digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void stopMotors() {
analogWrite(ENA, 0);
analogWrite(ENB, 0);
digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void sprayWater() {
Serial.println(">>> SPRAYING WATER <<<");

digitalWrite(relayPin, LOW);  // pump ON

// Servo sweep - left to right and back
for (int pos = 60; pos <= 120; pos += 4) {
waterServo.write(pos);
delay(35);
}
for (int pos = 120; pos >= 60; pos -= 4) {
waterServo.write(pos);
delay(35);
}

// Safety: don't spray forever
unsigned long sprayStart = millis();
while (millis() - sprayStart < MAX_SPRAY_DURATION) {
delay(400);
int currentCenter = readFlame(flameCenter);
if (currentCenter > FIRE_CLOSE_THRESHOLD + 80) {
break;  // fire seems weaker/out
}
}

digitalWrite(relayPin, HIGH);   // pump OFF
waterServo.write(90);           // return to center
Serial.println("Spray finished");
}

Is this code correct?