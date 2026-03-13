#include "motorControl.h"

void setMotor(int dir, int pwmVal) 
{
    analogWrite(PWM, pwmVal);
    if (dir == 1) {
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
    } else if (dir == -1) {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
    } else {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
    }
}
