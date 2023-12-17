#ifndef TYPES_H_
#define TYPES_H_

#include "stm32f3xx_hal.h"
#include <string.h>

#define AVRG_SIZE  20

struct sfloat_t{
	int8_t i;                //!< Integer part
	int8_t f;                //!< Fractional part
};

struct average_t{
	float val;
	float buf[AVRG_SIZE];
	uint8_t cnt = 0;
	bool fFull = false;

	void clear(){
		val = 0;
		cnt = 0;
		fFull = false;
	}
};

struct flags_t{
	bool wakeUpPB = false;
	bool wakeUpHOLD = false;
	bool mcu = false;
	bool pbINT = false;
	bool heater = false;
	bool faultLTC = false;
	bool faultVbatVLoad = false;
};

struct info_t{
	average_t vBat;
	average_t iBat;
	average_t iMon;
	average_t vOut;
	average_t tempBat;
	int16_t pBat;
	flags_t flags;

	float cBat;
	float eBat;

	int16_t iBatOffset = 0;

	uint8_t fullBatCnt = 0;
	bool fullBat = false;
};

struct adc_t{
	SDADC_HandleTypeDef *adc;
	uint32_t channel;
};

#pragma pack(push,1)
struct config_t{	//write to flash
	uint32_t id = 0;
	uint8_t fTelemetry = 10;	//Hz
	uint8_t shunt = 0;			//0 - 250 mkOm	 1 - 500 mkOm
	uint8_t resHeater = 10;
	float vOutK = 1;
	float vOutB = 0;
	float vBatK = 1;
	float vBatB = 0;
	float iBatK = 1;
	float iBatB = 0;
	float iMonK = 1;
	float iMonB = 0;
	float tempBatK = 1;
	float tempBatB = 0;
	uint32_t cInitial = 100000;		//100 Ah
	uint32_t eInitial = 5000000;	//5 kWh

	float vLow = 2.5;	//* 6
	float vMax = 4.2;	//* 6
	float iFullCharge = 0.2;
	float iLowCharge = 0.2;
	float tempBatMax = 60;

	int16_t iBatOffset = 16787;
};
#pragma pack(pop)

#endif /* TYPES_H_ */
