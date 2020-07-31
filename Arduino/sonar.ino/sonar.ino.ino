#include <Servo.h>

#define SERVO_PIN   8
#define TRIG_PIN    9
#define ECHO_PIN    10
#define SPK_PIN    11

#define SOUND

const int SONAR_DELAY = 30;
const int threshold = 30;
char active = 0;

struct servoData
{
  int          distance;
  int           angle;
};

Servo       serv;
servoData   currentData;

void beepStart() {
#ifdef SOUND
  tone(SPK_PIN, 800);
  delay(50);
  noTone(SPK_PIN);
  delay(50);
  tone(SPK_PIN, 800);
  delay(50);
  noTone(SPK_PIN);
  delay(50);
  tone(SPK_PIN, 800);
  delay(50);
  noTone(SPK_PIN);
  delay(50);
#endif
}

void initServo() {
  delay(500);

  int ang = serv.read();
  for (int i = ang; i > 0; i--) {
    serv.write(i);
    delay(15);
  }

  delay(500);
}

double measureDistance() {
  double duration;
  double distance;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor

  if (distance <= threshold) {
    if (active == 0) {
      triggerDown();
    }
    active = 1;
  }
  else {
    active = 0;
  }

  return active == 0 ? -1 : distance;
}

void triggerDown() {
#ifdef SOUND
  tone(SPK_PIN, 400, 200);
#endif
}

void handleTick(int currentAngle) {
  double dist = measureDistance();
  currentData.distance = dist;
  currentData.angle = currentAngle;

  Serial.print(currentAngle);
  Serial.print('@');
  Serial.println(dist);
  serv.write(currentAngle);
}

void setup() {
  serv.attach(SERVO_PIN);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SPK_PIN, OUTPUT);

  Serial.begin(9600);
  initServo();
  beepStart();
}

void loop() {
#ifdef SOUND
  tone(SPK_PIN, 850, 50);
#endif
  int angle;

  for (angle = 0; angle < 180; angle++) {
    handleTick(angle);
    delay(SONAR_DELAY);
  }
#ifdef SOUND
  tone(SPK_PIN, 850, 50);
#endif
  for (angle = 180; angle > 0; angle--) {
    handleTick(angle);
    delay(SONAR_DELAY);
  }
}
