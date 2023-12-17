#ifndef SDADC_H_
#define SDADC_H_

#include "stm32f3xx_hal.h"

class SdAdc {
public:
	SdAdc();
	virtual ~SdAdc();

	static int16_t getValue(SDADC_HandleTypeDef *hsdadc, uint32_t Channel, uint32_t ConfIndex = SDADC_CONF_INDEX_0);
};

#endif /* SDADC_H_ */
