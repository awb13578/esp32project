#include <Arduino.h>

#include "app_state/app_state.h"
#include "../test/PH/test_PH.h"
#include "../test/PH/test_PH.cpp"

app_state_t app_state = {};

namespace {

constexpr uint32_t UPDATE_INTERVAL_MS = 1000;

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

    if (!test_PH_init()) {
        Serial.println("ERROR: pH test initialization failed.");
    }
}

void loop() {
    if (!update_interval_elapsed()) {
        return;
    }

    if (test_PH_update()) {
        Serial.print("pH: ");
        Serial.println(app_state.ph, 2);
    } else {
        Serial.println("ERROR: pH reading failed.");
    }
}
