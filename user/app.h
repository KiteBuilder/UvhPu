#ifndef APP_H_
#define APP_H_

#include "stm32f3xx_hal.h"

void initialization();
void exec();

void onTim3Triggered();

#define DEBUG_ENABLED


#endif /* APP_H_ */
