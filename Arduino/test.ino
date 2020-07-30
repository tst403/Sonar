#include <Servo.h>

#define SERVO_PIN   8
#define TRIG_PIN    9
#define ECHO_PIN    10

const int SONAR_DELAY = 8;

struct servoData
{
    double          distance,
    unsigned byte   angle;
};

Servo       serv;
servoData   currentData;

void initServo(){
    serv.write(0);
    sleep(2000);
}

double measureDistance(){
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
    
    return distance;
}

void handleTick(int currentAngle){
    double dist = measureDistance();
    currentData.distance = dist;
    currentData.angle = currentAngle;

    Serial.write(currentData);
}

void setup(){
    serv.attach(SERVO_PIN);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    Serial.begin(9600);
    initServo();
}

void loop(){
    int angle;

    for(angle = 0; angle < 180; angle++){
        handleTick(angle);
        delay(SONAR_DELAY);
    }
    
    for(angle = 180; angle > 0; angle--){
        handleTick(angle);
        delay(SONAR_DELAY);
    }
}
