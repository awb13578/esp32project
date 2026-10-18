#ifndef APP_CONTROL_H_
#define APP_CONTROL_H_

typedef enum {
    APP_CONTROL_STATUS_MOTOR_ON = 0,
    APP_CONTROL_STATUS_MOTOR_OFF,
    APP_CONTROL_STATUS_AXIT_PUMP_ON,
    APP_CONTROL_STATUS_AXIT_PUMP_OFF,
    APP_CONTROL_STATUS_BAZO_PUMP_ON,
    APP_CONTROL_STATUS_BAZO_PUMP_OFF,
    APP_CONTROL_STATUS_WATER_OK,
} app_control_status_e;

bool app_control_set_status(app_control_status_e status);
app_control_status_e app_control_get_status(void);
bool app_control_init(void);
bool app_control_update(void);
float app_control_get_ph(void);

#endif /* APP_CONTROL_H_ */