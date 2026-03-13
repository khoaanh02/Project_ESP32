#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Motor control pins
#define PWM 33
#define IN4 5
#define IN3 18

// Function prototypes
void setMotor(int dir, int pwmVal);

#endif