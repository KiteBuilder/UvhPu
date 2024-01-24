#ifndef TYPES_H_
#define TYPES_H_

#include "stm32f3xx_hal.h"
#include <string.h>
#include <math.h>

#define AVRG_SIZE  20

struct sfloat_t{
	int8_t i;                //!< Integer part
	int8_t f;                //!< Fractional part
};

struct average_t{
	float val;
	float buf[AVRG_SIZE];
	uint8_t cnt = 0;

	void clear(){
		val = 0;
		cnt = 0;
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

	float cBatRest = 0.0; //Energy  that left in the battery in mAh
	float eBatRest = 0.0; //Energy that left in the battery in mWh
	float resBat = 0.0; //battery impedance
	float vRest= 0.0;  //voltage with sag removed based on current and resistance estimate in Volt
	                   //resting voltage, should always be greater or equal to the raw voltage
	float vBatFilt = 0.0; //Filtered voltage
	float iBatFilt = 0.0; //Filtered current
	uint16_t iBatOffset = 0;

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
	uint32_t cInitial = 3450;	//in mAh = 3.5 Ah
	uint32_t eInitial = 51060;	//in mWh = 5.8 kWh (cInitial * 3.7 * 4)

	float vLow = 2.5;	//* 6
	float vMax = 4.2;	//* 6
	float iFullCharge = 0.2;
	float iLowCharge = 0.2;
	float tempBatMax = 60;

	uint16_t iBatOffset = 16787;
};
#pragma pack(pop)

#pragma pack(push,1)
//this structure size should be multiple of 4 bytes, otherwise add a reserve field
typedef struct{
    uint32_t index = 0; //index of structure written to the flash memory. 0 if structure should be written for the first time
    float cBat = 0.0;   //battery power draw in mAh
    float eBat = 0.0;   //battery power draw in Wh
    //these two variables should be set to 0 remotely in case of a brand new battery
    float cBatMod = 0.0; //Module of energy  that important to estimate the battery life cycle
    uint16_t lifeCycles = 0; //life cycles counter that increments when cBatMod two times greater than cInitial
    uint16_t checksum = 0; //because this structure should be written to flash we need a checksum to check the validity
    //uint16_t reserve = 0;
} energy_t;
#pragma pack(pop)

//*****************************************************************************
//Here located float primitives
//*****************************************************************************

#define FLT_EPSILON      1.192092896e-07F        // smallest such that 1.0+FLT_EPSILON != 1.0

/*
 * @brief: Check whether a float is zero
 */
inline bool is_zero(const float x)
{
    return fabsf(x) < FLT_EPSILON;
}

/*
 * @brief: Check whether a float is greater than zero
 */
inline bool is_positive(const float x)
{
    return (x >= FLT_EPSILON);
}

/*
 * @brief: Check whether a float is less than zero
 */
inline bool is_negative(const float x)
{
    return (x <= (-1.0 * FLT_EPSILON));
}

// Convert amps milliseconds to milliamp hours
// Amp.millisec to milliAmp.hour = 1/1E3(ms->s) * 1/3600(s->hr) * 1000(A->mA)
#define AMS_TO_MAH 0.000277777778f

// Amps microseconds to milliamp hours
#define AUS_TO_MAH 0.0000002778f

// Amps seconds to milliamp hours
#define AS_TO_MAH 0.2778f


#endif /* TYPES_H_ */
