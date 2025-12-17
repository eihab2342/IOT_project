#include <TM1637Display.h>

// ===== Pins Configuration =====

// Ultrasonic Sensor 1 (Inside)
#define TRIG1 4
#define ECHO1 5

// Ultrasonic Sensor 2 (Outside)
#define TRIG2 6
#define ECHO2 7

// TM1637 Display
#define CLK_PIN 2
#define DIO_PIN 3

// Buzzer
#define BUZZER 8

// ===== Objects =====
TM1637Display display(CLK_PIN, DIO_PIN);

// ===== Variables =====
int peopleCount = 0;           // عدد الأشخاص في الغرفة
bool sensor1Triggered = false; // حالة السنسور الأول (داخل)
bool sensor2Triggered = false; // حالة السنسور الثاني (خارج)
unsigned long lastTriggerTime = 0;

const int DETECTION_DISTANCE = 50; // أقل من 50 سم = في شخص قدام السنسور
const int TIMEOUT = 2000;          // 2 ثانية بين السنسورين

// ===== Helpers =====
int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms

  int distance = duration * 0.034 / 2; // سم

  if (distance == 0) distance = 999;   // قراءة فاضية
  return distance;
}

void resetSensors() {
  sensor1Triggered = false;
  sensor2Triggered = false;
}

void beep(int duration) {
  digitalWrite(BUZZER, HIGH);
  delay(duration);
  digitalWrite(BUZZER, LOW);
}

// ===== Setup =====
void setup() {
  Serial.begin(9600);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(BUZZER, OUTPUT);

  display.setBrightness(0x0f);      // أعلى إضاءة
  display.showNumberDec(0, true);   // ابدأ من 0

  Serial.println("Room Counter Started...");
}

// ===== Main Loop =====
void loop() {
  int d1 = getDistance(TRIG1, ECHO1); // inside
  int d2 = getDistance(TRIG2, ECHO2); // outside

  unsigned long now = millis();

  // لطباعة القيم للتجربة
  Serial.print("Inside: "); Serial.print(d1);
  Serial.print(" cm | Outside: "); Serial.print(d2);
  Serial.print(" cm | Count: "); Serial.println(peopleCount);

  // ===== شخص يدخل: Outside ثم Inside =====
  if (d2 < DETECTION_DISTANCE && !sensor2Triggered) {
    sensor2Triggered = true;
    lastTriggerTime = now;
    Serial.println("Outside sensor triggered");
  }

  if (sensor2Triggered && d1 < DETECTION_DISTANCE && !sensor1Triggered) {
    sensor1Triggered = true;
    if (now - lastTriggerTime < TIMEOUT) {
      // دخول
      peopleCount++;
      if (peopleCount < 0) peopleCount = 0;
      display.showNumberDec(peopleCount, true);
      Serial.println(">>> Person ENTERED");
      beep(80);
      resetSensors();
    }
  }

  // ===== شخص يخرج: Inside ثم Outside =====
  if (d1 < DETECTION_DISTANCE && !sensor1Triggered) {
    sensor1Triggered = true;
    lastTriggerTime = now;
    Serial.println("Inside sensor triggered");
  }

  if (sensor1Triggered && d2 < DETECTION_DISTANCE && !sensor2Triggered) {
    sensor2Triggered = true;
    if (now - lastTriggerTime < TIMEOUT) {
      // خروج
      if (peopleCount > 0) peopleCount--;
      display.showNumberDec(peopleCount, true);
      Serial.println("<<< Person EXITED");
      beep(160);
      resetSensors();
    }
  }

  // لو التسلسل اتلغبط أو الوقت طول
  if (now - lastTriggerTime > TIMEOUT) {
    resetSensors();
  }

  delay(80); // تهدئة للقراءه
}
