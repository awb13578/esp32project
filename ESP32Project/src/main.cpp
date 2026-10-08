#include <Arduino.h>
#include "drv_gpio.h"

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  drv_gpio_init(GPIO_ID_A, 5, OUTPUT); // Initialize GPIO pin 5 as OUTPUT
}

void loop() {
  // put your main code here, to run repeatedly:
  drv_gpio_toggle_pin(GPIO_ID_A); // Toggle the state of GPIO pin A
  delay(1000); // Wait for 1 second
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}