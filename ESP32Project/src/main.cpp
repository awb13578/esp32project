#include <Arduino.h>

#include "app_control/app_control.h"
#include "app_display/app_display.h"

namespace {

constexpr uint32_t UPDATE_INTERVAL_MS = 250;

bool control_ready = false;
bool display_ready = false;

} // namespace

void setup() {
    Serial.begin(115200);

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

    if (control_ready) {
        app_control_update();
    }

    if (display_ready) {
        app_display_update(app_control_get_ph());
    }
}
