#ifndef APP_DISPLAY_H_
#define APP_DISPLAY_H_

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "drv_config.h"
#include "mid_lcd1602.h"

bool app_display_init(void);
bool app_display_update(float ph);

#endif /* APP_DISPLAY_H_ */