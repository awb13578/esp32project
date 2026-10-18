#include <Arduino.h>

#include "app_control/app_control.h"
#include "app_display/app_display.h"

namespace {

constexpr uint32_t UPDATE_INTERVAL_MS = 250;

bool control_ready = false;
bool display_ready = false;

volatile int system_state = 1;

void IRAM_ATTR isr_gpio13() {
    system_state = 1;
}

void IRAM_ATTR isr_gpio14() {
    system_state = 0;
}

} // namespace

void setup() {
    Serial.begin(115200);

    pinMode(13, INPUT_PULLDOWN);
    pinMode(14, INPUT_PULLDOWN);

    attachInterrupt(digitalPinToInterrupt(13), isr_gpio13, RISING);
    attachInterrupt(digitalPinToInterrupt(14), isr_gpio14, RISING);

    control_ready = app_control_init();
    if (!control_ready) {
      Serial.println("ERROR: control initialization failed; pumps disabled.");
    }

    display_ready = app_display_init();
    if (!display_ready) {
      Serial.println("ERROR: LCD initialization failed.");
    }
}

void loop() {
    static uint32_t last_update_ms = 0;
    const uint32_t now = millis();
    if (static_cast<uint32_t>(now - last_update_ms) < UPDATE_INTERVAL_MS) {
        return;
    }
    last_update_ms = now;

    if (system_state == 1) {
        if (control_ready) {
            app_control_update();
        }

        if (display_ready) {
            app_display_update(app_control_get_ph());
        }
    } else {
        app_control_stop_all();
    }
}
