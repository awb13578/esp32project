#include <Arduino.h>
#include <math.h>

#include "app_state/app_state.h"
#include "../test/test_chuc_nang/test_PH+LCD.h"
#include "../test/test_chuc_nang/test_PH+LCD.cpp"

app_state_t app_state = {};

namespace {

constexpr uint32_t UPDATE_INTERVAL_MS = 200;

bool ph_lcd_ready = false;

bool update_interval_elapsed() {
    static uint32_t last_update_ms = 0;
    const uint32_t now = millis();
    if (static_cast<uint32_t>(now - last_update_ms) < UPDATE_INTERVAL_MS) {
        return false;
    }
    last_update_ms = now;
    return true;
}

} // namespace

void setup() {
    Serial.begin(115200);
    delay(3000);

    ph_lcd_ready = test_PH_LCD_init();
    if (!ph_lcd_ready) {
        Serial.println("ERROR: pH or LCD test initialization failed.");
    }
}

void loop() {
    if (!update_interval_elapsed()) {
        return;
    }

    if (!test_PH_LCD_update()) {
        Serial.println("ERROR: pH reading or LCD update failed.");
    }

    if (isfinite(app_state.ph)) {
        Serial.print("pH: ");
        Serial.println(app_state.ph, 2);
    }
}
