#include <Arduino.h>
#include "drv_gpio.h"
#include "drv_i2c.h"
#include "mid_lcd1602.h"

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  drv_i2c_init(I2C_ID_LCD1602, 21, 22); // Initialize I2C with SDA on pin 21 and SCL on pin 22
  LCD1602_Init(); // Initialize the LCD
}

void loop() {
  // put your main code here, to run repeatedly:
  LCD1602_Clear(); // Clear the LCD
  LCD1602_SetCursor(0, 0); // Set cursor to the first row, first column
  LCD1602_Print("Hello, World!"); // Print a message on the LCD
  delay(1000); // Wait for 1 second
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}