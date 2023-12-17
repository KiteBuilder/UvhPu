#ifndef SRC_USER_PORT_H_
#define SRC_USER_PORT_H_

#include "stm32f3xx_hal.h"

class port {
public:
	port();
	virtual ~port();
	bool status(HAL_StatusTypeDef status);
};

#endif /* SRC_USER_PORT_H_ */
