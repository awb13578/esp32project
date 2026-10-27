#include <Arduino.h>

#include "app_control/app_control.h"
#include "app_display/app_display.h"
#include "app_state/app_state.h"

void init_pin_on_off();

bool update_interval_elapsed();

app_state_t app_state = {};

namespace {

constexpr uint32_t UPDATE_INTERVAL_MS = 200;

// void IRAM_ATTR isr_gpio13() {
//     app_state.system_state = true;
// }

// void IRAM_ATTR isr_gpio14() {
//     app_state.system_state = false;
// }

} // namespace

void setup() {
    Serial.begin(115200);

    // init_pin_on_off(); // cần bật 13 để hệ thống hoạt động
    delay(3000);
    app_state.system_state = true; // tạm thời bỏ qua việc bật/tắt hệ thống bằng nút nhấn

    app_state.control_ready = app_control_init();
    if (!app_state.control_ready) {
      Serial.println("ERROR: control initialization failed; pumps disabled.");
    }

    app_state.display_ready = app_display_init();
    if (!app_state.display_ready) {
      Serial.println("ERROR: LCD initialization failed.");
    }
}

void loop() {
    if (!update_interval_elapsed()) {
        return;
    }

    if (app_state.system_state) {
        if (app_state.control_ready) {
            app_control_update();
        }

        if (app_state.display_ready && app_state.ph_updated) {
            app_display_update(app_control_get_ph());
        }
    } else {
        app_control_stop_all();
    }
}

void init_pin_on_off() {
    pinMode(13, INPUT_PULLDOWN);
    pinMode(14, INPUT_PULLDOWN);

    attachInterrupt(digitalPinToInterrupt(13), []() { app_state.system_state = true; }, RISING);
    attachInterrupt(digitalPinToInterrupt(14), []() { app_state.system_state = false; }, RISING);
}

bool update_interval_elapsed() {
    static uint32_t last_update_ms = 0;
    const uint32_t now = millis();
    if (static_cast<uint32_t>(now - last_update_ms) < UPDATE_INTERVAL_MS) {
        return false;
    }
    last_update_ms = now;
    return true;
}
