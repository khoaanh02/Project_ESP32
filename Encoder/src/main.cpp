#include <Arduino.h>
#include "MotorControl.h"
#include "Encoder.h"

// PID constants
const float kp = 4.0;
const float kd = 0.035;
const float ki = 0.0;

// Variables for timing
long prevT = 0;
float eprev = 0;
float eintegral = 0;

// Target position, initialized with a default
volatile int target = 0;

void setup() {
    Serial.begin(9600);
    while (!Serial); // Wait for the serial port to connect. Needed for native USB
    // Serial.println("Enter target angle in degrees:");

    pinMode(ENCODER_1A, INPUT);
    pinMode(ENCODER_1B, INPUT);
    attachInterrupt(digitalPinToInterrupt(ENCODER_1A), encoder1_isr, RISING);

    pinMode(PWM, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
}

void loop() {

    target = 530/2; // 333 xung 1 vong --> 360 do

    long currT = micros();
    float deltaT = ((float)(currT - prevT)) / 1.0e6;
    prevT = currT;

    noInterrupts();
    int pos = posi;
    interrupts();

    // PID algorithm - Gain, integral, D
    float e = pos - target; //P

    float dedt = (e - eprev) / deltaT; //D

    eintegral += e * deltaT; //I

    float u = kp * e + kd * dedt + ki * eintegral;

    int pwr = (int)fabs(u);
    pwr = constrain(pwr, 0, 200); //PWM: 0 - 255

    int dir = (u < 0) ? -1 : 1;

    setMotor(dir, pwr);

    eprev = e;

    Serial.print("Target: "); Serial.print(target);
    Serial.print("\tCurrent position: "); Serial.println(pos);
}
