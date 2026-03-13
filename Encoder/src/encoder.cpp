#include "encoder.h"

volatile int posi = 0; // Position updated by the encoder

void encoder1_isr() {
    int B = digitalRead(ENCODER_1B);
    if (B > 0) {
        posi++;
    } else {
        posi--;
    }
}
