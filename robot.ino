#include <Servo.h>

#define flameLeft   A0
#define flameCenter A1
#define flameRight  A2

#define trigPin 8
#define echoPin 9

#define tempPin A3  
#define ENA 5
#define IN1 2
#define IN2 3
#define IN3 4
#define IN4 7
#define ENB 6


#define relayPin 10


#define servoPin 11


Servo waterServo;

const int MOTOR_SPEED_FORWARD = 150;
const int MOTOR_SPEED_TURN    = 110;   

const int FIRE_DETECTED_THRESHOLD = 650;   
const int FIRE_CLOSE_THRESHOLD    = 320;  

const unsigned long SPRAY_COOLDOWN_MS   = 12000;   
const unsigned long MAX_SPRAY_DURATION  = 7000;   

unsigned long lastSprayTime = 0;

void setup() {
Serial.begin(9600);

pinMode(flameLeft,   INPUT);
pinMode(flameCenter, INPUT);
pinMode(flameRight,  INPUT);

pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT);

pinMode(ENA, OUTPUT);
pinMode(IN1, OUTPUT);
pinMode(IN2, OUTPUT);
pinMode(IN3, OUTPUT);
pinMode(IN4, OUTPUT);
pinMode(ENB, OUTPUT);

pinMode(relayPin, OUTPUT);
digitalWrite(relayPin, HIGH);  

waterServo.attach(servoPin);
delay(200);          
waterServo.write(90); 

stopMotors();
Serial.println("Fire Fighting Robot - Ready");
Serial.println("Flame thresholds: detect < " + String(FIRE_DETECTED_THRESHOLD) +
" | close < " + String(FIRE_CLOSE_THRESHOLD));
}


void loop() {
int left   = readFlame(flameLeft);
int center = readFlame(flameCenter);
int right  = readFlame(flameRight);
long distance = getDistance();



if (distance < 22 && distance > 0) { 
stopMotors();
delay(150);
moveBackward();
delay(600);             
turnRight();          
delay(900);             
stopMotors();
delay(200);
return;
}


if (center < FIRE_CLOSE_THRESHOLD) {
if (millis() - lastSprayTime >= SPRAY_COOLDOWN_MS) {
stopMotors();
delay(300);          
sprayWater();
lastSprayTime = millis();
} else {
stopMotors();           
}
return;
}


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

int readFlame(int pin) {
int sum = 0;
const int samples = 8;
for (int i = 0; i < samples; i++) {
sum += analogRead(pin);
delay(2);
}
return sum / samples;
}


long getDistance() {
digitalWrite(trigPin, LOW);
delayMicroseconds(3);
digitalWrite(trigPin, HIGH);
delayMicroseconds(11);
digitalWrite(trigPin, LOW);

long duration = pulseIn(echoPin, HIGH, 30000); 
if (duration == 0) return 999;                 
return duration * 0.034 / 2;
}

void moveForward() {
analogWrite(ENA, MOTOR_SPEED_FORWARD);
analogWrite(ENB, MOTOR_SPEED_FORWARD);
digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnLeft() {
analogWrite(ENA, MOTOR_SPEED_TURN);
analogWrite(ENB, MOTOR_SPEED_TURN);
digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); 
digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   
}

void turnRight() {
analogWrite(ENA, MOTOR_SPEED_TURN);
analogWrite(ENB, MOTOR_SPEED_TURN);
digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   
digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);  
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

digitalWrite(relayPin, LOW); 

for (int pos = 60; pos <= 120; pos += 4) {
waterServo.write(pos);
delay(35);
}
for (int pos = 120; pos >= 60; pos -= 4) {
waterServo.write(pos);
delay(35);
}

unsigned long sprayStart = millis();
while (millis() - sprayStart < MAX_SPRAY_DURATION) {
delay(400);
int currentCenter = readFlame(flameCenter);
if (currentCenter > FIRE_CLOSE_THRESHOLD + 80) {
break;  
}
}

digitalWrite(relayPin, HIGH);   
waterServo.write(90);          
Serial.println("Spray finished");
}