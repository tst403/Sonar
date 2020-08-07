#include <Servo.h>

#define SERVO_PIN   8
#define TRIG_PIN    9
#define ECHO_PIN    10
#define SPK_PIN    12

#define SOUND

Servo serv;

class Target {

  private:
    const int MAX_TRIES = 4;
    const int trackDelay = 45;
    const int angleVary = 15;
    int currentAngle;
    int iterAngle;
    void (* targetFoundCallback)();
    bool (* isTarget)(int angle, int maxRange);
    bool active = true;
    bool rightSweep = true;
    int cycles = MAX_TRIES;
    int threshold;

  public:
    bool iterate() {
      // Upon discover, update current angle and iterate angle(from left)
      if (this->isTarget(this->iterAngle, threshold)) {

        this->currentAngle = this->iterAngle;
        this->iterAngle = this->currentAngle - angleVary;
        this->cycles = MAX_TRIES;
        Serial.print("TARGET_FOUND ==> ");
        Serial.println(this->currentAngle);
        tone(SPK_PIN, 3000, 100);
        return true;
      }
      else {
        // On target loss, update to inactive
        if (((rightSweep) && (iterAngle >= this->currentAngle + this->angleVary) ) ||
            ((!rightSweep) && (iterAngle <= this->currentAngle - this->angleVary))) {
          if (this->cycles == 0) {
            this->active = false;
          }
          else {
            this->iterAngle = rightSweep ? this->currentAngle - this->angleVary : this->currentAngle + this->angleVary;
            this->cycles--;
            rightSweep = !rightSweep;
          }
        }
        else {
          if (rightSweep) {
            this->iterAngle++;
          }
          else {
            this->iterAngle--;
          }
        }
      }

      return false;
    }

    void track() {

      int curAng = this->currentAngle;
      bool active = true;
      const int MAX_TRIES = 4;
      int tries = MAX_TRIES;
      int vary = 10;
      int _min, _max;
      bool hasFound;

      while (active) {
        hasFound = false;
        _min = curAng;
        _max = curAng;

        Serial.println("Start");
        for (int i = curAng; i < curAng + vary; i++) {
          if (this->isTarget(this->iterAngle, threshold)) {
            if (i < _min) _min = i;
            
            if (i > _max) _max = i;
            hasFound = true;
          }
          serv.write(i);
          delay(10);
        }

        for (int i = curAng + vary; i > curAng - vary; i--) {
          if (this->isTarget(this->iterAngle, threshold)) {
            if (i < _min) _min = i;
            if (i > _max) _max = i;
            hasFound = true;
          }
          serv.write(i);
          delay(10);
        }

        for (int i = curAng - vary; i > curAng; i--) {
          if (this->isTarget(this->iterAngle, threshold)) {
            if (i < _min) _min = i;
            if (i > _max) _max = i;
            hasFound = true;
          }

          serv.write(i);
          delay(10);
        }

        if (!hasFound) {
          if (--tries == 0) {
            active = false;
          }
        }
        else {
          tone(SPK_PIN, 3000, 100);
          Serial.print("Locked --> ");
          Serial.println(curAng);
          curAng = (int)((_min + _max) / 2);
          tries = MAX_TRIES;
        }
      }
    }

    Target(int ang, void (* callback)(), bool (* checkFunc)(int angle, int maxRange), int threshold ) {
      this->currentAngle = ang;
      this->iterAngle = ang - angleVary;
      Serial.println("XXX");
      Serial.println(this->iterAngle);
      this->targetFoundCallback = callback;
      this->isTarget = checkFunc;
      this->threshold = threshold;
    }

    bool isActive() {
      return this->active;
    }

    int getIterAng() {
      return this->iterAngle;
    }
};


const int SONAR_DELAY = 30;
int threshold = 100;
const int PING_FREQ = 1500;
char active = 0;
// maxTimeout defualt value is 4 meters
int maxTimeout = 23323;

int state = 0;
int angle;
bool cw = true;
bool halt = false;
Target *activeTarget;

struct servoData
{
  int          distance;
  int           angle;
};

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

void soundLockStart() {
  tone(SPK_PIN, 200, 50);
}

void soundLockLost() {
  tone(SPK_PIN, 100, 200);
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
  duration = pulseIn(ECHO_PIN, HIGH, maxTimeout);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor

  if (distance <= threshold && distance != 0) {
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
if(state == 0){
    tone(SPK_PIN, PING_FREQ, 200);
}
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

int getMaxRange() {
  String inString = "";

  while (Serial.available() == 0) {
    delay(1);
  }

  return Serial.parseInt();
}

void handler() {

}

bool func1(int ang, int border) {
  double dist = measureDistance();
  return dist != -1 && dist <= border;
}

void handleCommand(String cmd) {
  if (cmd == "X\n") {
    halt = true;
  }
  if (cmd[0] == 'T') {
    String num = cmd.substring(1);
    int angTrack = angle;
    Serial.print("Tracking: ");
    Serial.println(angTrack);
    activeTarget = new Target(angTrack, handler, func1, threshold);
    state = 1;
    soundLockStart();
  }
}

void setup() {
  serv.attach(SERVO_PIN);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SPK_PIN, OUTPUT);
  Serial.begin(9600);

  threshold = getMaxRange();
  maxTimeout = (int)(((threshold / 100.0f) / (343.0f / 2)) * (1000000 * 1.2f));

  initServo();
  beepStart();
}

void loop() {
  if (state == 0) {
    if (halt) {
      delay(1000);
      return;
    }

    handleTick(angle);
    delay(SONAR_DELAY);

    if (cw) {
      angle++;
    }
    else {
      angle--;
    }

    if (angle == 0 || angle == 180) {
#ifdef SOUND
      tone(SPK_PIN, 850, 50);
#endif
      cw = !cw;
    }
  }
  else if (state == 1) {
    activeTarget->track();
    soundLockLost();
    state = 0;

    /*if (activeTarget->iterate()) {
      Serial.println("Target found");
      }
      else {
      Serial.println("Searching...");
      }*/


    delay(SONAR_DELAY);
  }

  if (Serial.available() > 0) {
    String temp;

    while (Serial.available() > 0) {
      temp += (char)Serial.read();
    }

    handleCommand(temp);
  }

}
