#include <TM1637Display.h>

// ===== Pins =====
#define TRIG_IN   4   // INSIDE
#define ECHO_IN   5
#define TRIG_OUT  6   // OUTSIDE
#define ECHO_OUT  7

#define CLK_PIN 2
#define DIO_PIN 3
#define BUZZER  8

TM1637Display display(CLK_PIN, DIO_PIN);

// ===== Constants =====
const int DIST = 40;
const unsigned long TIMEOUT = 1500;

// ===== State =====
enum State {
  IDLE,
  WAIT_IN,   // Outside -> Inside (ENTER)
  WAIT_OUT   // Inside -> Outside (EXIT)
};

State state = IDLE;

// ===== Variables =====
int peopleCount = 0;
unsigned long tStart = 0;

// ===== Functions =====
int getDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long d = pulseIn(echo, HIGH, 30000);
  if (d == 0) return 999;
  return d * 0.034 / 2;
}

void beep(int ms) {
  digitalWrite(BUZZER, HIGH);
  delay(ms);
  digitalWrite(BUZZER, LOW);
}

// ===== Setup =====
void setup() {
  pinMode(TRIG_IN, OUTPUT);
  pinMode(ECHO_IN, INPUT);
  pinMode(TRIG_OUT, OUTPUT);
  pinMode(ECHO_OUT, INPUT);
  pinMode(BUZZER, OUTPUT);

  display.setBrightness(0x0f);
  display.showNumberDec(peopleCount, true);
}

// ===== Loop =====
void loop() {
  int dIn  = getDistance(TRIG_IN,  ECHO_IN);
  int dOut = getDistance(TRIG_OUT, ECHO_OUT);
  unsigned long now = millis();

  switch (state) {

    case IDLE:
      if (dOut < DIST) {           // Outside first → ENTER
        state = WAIT_IN;
        tStart = now;
      }
      else if (dIn < DIST) {       // Inside first → EXIT
        state = WAIT_OUT;
        tStart = now;
      }
      break;

    case WAIT_IN:
      if (dIn < DIST && now - tStart < TIMEOUT) {
        peopleCount++;
        display.showNumberDec(peopleCount, true);
        beep(80);
        state = IDLE;
      }
      break;

    case WAIT_OUT:
      if (dOut < DIST && now - tStart < TIMEOUT) {
        if (peopleCount > 0) peopleCount--;
        display.showNumberDec(peopleCount, true);
        beep(150);
        state = IDLE;
      }
      break;
  }

  // Timeout reset
  if (state != IDLE && now - tStart > TIMEOUT) {
    state = IDLE;
  }

  delay(80);
}
