#include <SdAdc.h>

SdAdc::SdAdc() {
	// TODO Auto-generated constructor stub

}

SdAdc::~SdAdc() {
	// TODO Auto-generated destructor stub
}

int16_t SdAdc::getValue(SDADC_HandleTypeDef *hsdadc, uint32_t Channel, uint32_t ConfIndex){
	HAL_SDADC_AssociateChannelConfig(hsdadc, Channel, ConfIndex);
	HAL_SDADC_ConfigChannel(hsdadc, Channel, SDADC_CONTINUOUS_CONV_OFF);
	HAL_SDADC_Start(hsdadc);
	HAL_SDADC_PollForConversion(hsdadc, HAL_MAX_DELAY);
	return static_cast<int16_t>(HAL_SDADC_GetValue(hsdadc));
}

