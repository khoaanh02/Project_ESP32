#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

// Encoder pins
#define ENCODER_1A 34
#define ENCODER_1B 35

// Global variables
extern volatile int posi;

// Function prototypes
void encoder1_isr();

#endif