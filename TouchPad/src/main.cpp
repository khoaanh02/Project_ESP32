#include <Arduino.h>
/*
#define led 2
#define button 4

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  pinMode(button, OUTPUT);

}

void loop() {
  int value = touchRead(button);
  Serial.println(value);
  delay(5);
  if(value < 10){
    digitalWrite(led, HIGH);

  }
  else{
    digitalWrite(led, LOW);
  }
}

*/
//Sayan Electronics(LIKE-SUBSCRIBE-SHARE)
//More Fluid Eye Motion is here 
//Connect Vcc-3.3V, Gnd-Gnd, SCL-D22, SDA-D21 (0.96 inch OLED - ESP32 DEV KIT V1)
//CODE_NO: 02
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Eye position and size
int leftEyeX = 25;       // X position of the left eye
int rightEyeX = 80;      // X position of the right eye
int eyeY = 18;           // Y position of both eyes
int eyeWidth = 25;       // Eye width
int eyeHeight = 30;      // Eye height

// Target offsets for smooth movement
int targetOffsetX = 0;
int targetOffsetY = 0;
int moveSpeed = 3;       // Control speed of movement

// Blinking and animation variables
int blinkState = 0;      // 0 = eyes open, 1 = eyes closed (blinking)
int blinkDelay = 2000;   // Blink delay (4 seconds)
unsigned long lastBlinkTime = 0;
unsigned long moveTime = 0;

void drawEye(int eyeX, int eyeY, int eyeWidth, int eyeHeight) {
  display.fillRoundRect(eyeX, eyeY, eyeWidth, eyeHeight, 5, WHITE);  // Draw rounded rectangle
}

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
}

void loop() {
  unsigned long currentTime = millis();

  // Blinking logic
  if (currentTime - lastBlinkTime > blinkDelay && blinkState == 0) {
    blinkState = 1;  // Blink starts
    lastBlinkTime = currentTime;
  } else if (currentTime - lastBlinkTime > 150 && blinkState == 1) {//Decrease/Increase 150 to decrease/increase blink speed
    blinkState = 0;  // Blink ends
    lastBlinkTime = currentTime;
  }

  // Random eye movement logic
  if (currentTime - moveTime > random(1500, 3000) && blinkState == 0) {
    int movementType = random(0, 8);  // Choose movement: 0 to 7 (diagonal + center)
    if (movementType == 0) {          // Look left
      targetOffsetX = -10;
      targetOffsetY = 0;
    } else if (movementType == 1) {   // Look right
      targetOffsetX = 10;
      targetOffsetY = 0;
    } else if (movementType == 2) {   // Look up-left
      targetOffsetX = -10;
      targetOffsetY = -8;
    } else if (movementType == 3) {   // Look up-right
      targetOffsetX = 10;
      targetOffsetY = -8;
    } else if (movementType == 4) {   // Look down-left
      targetOffsetX = -10;
      targetOffsetY = 8;
    } else if (movementType == 5) {   // Look down-right
      targetOffsetX = 10;
      targetOffsetY = 8;
    } else {                          // Default (center)
      targetOffsetX = 0;
      targetOffsetY = 0;
    }
    moveTime = currentTime;
  }

  // Smooth interpolation for movement
  static int offsetX = 0;
  static int offsetY = 0;
  offsetX += (targetOffsetX - offsetX) / moveSpeed;
  offsetY += (targetOffsetY - offsetY) / moveSpeed;

  // Clear display for redraw
  display.clearDisplay();

  // Draw the left eye
  if (blinkState == 0) {
    drawEye(leftEyeX + offsetX, eyeY + offsetY, eyeWidth, eyeHeight);
  } else {
    // Eye blink (draw a thin horizontal line)
    display.fillRect(leftEyeX + offsetX, eyeY + offsetY + eyeHeight / 2 - 2, eyeWidth, 4, WHITE);
  }

  // Draw the right eye
  if (blinkState == 0) {
    drawEye(rightEyeX + offsetX, eyeY + offsetY, eyeWidth, eyeHeight);
  } else {
    // Eye blink (draw a thin horizontal line)
    display.fillRect(rightEyeX + offsetX, eyeY + offsetY + eyeHeight / 2 - 2, eyeWidth, 4, WHITE);
  }

  // Refresh the display
  display.display();

  // Small delay to prevent flickering
  delay(30);
}

// Function to draw a single eye
