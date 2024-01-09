#ifndef DEVICE_H_
#define DEVICE_H_

#include "Protocol.h"
#include "Flash.h"
#include "def_ports.h"
#include "time.h"
#include "Filters.h"


#define PORT_JUMPER		GPIOB
#define PIN_JUMPER0		GPIO_PIN_3
#define PIN_JUMPER1		GPIO_PIN_4
#define PIN_JUMPER2		GPIO_PIN_5
#define PIN_JUMPER3		GPIO_PIN_6

#define R_SHUNT(flag)	(flag ? 0.0005 : 0.00025)
#define R_DIV1 60400
#define R_DIV2 3600

#define VBATT_LPF_FREQ 0.5 //Hz
#define IBATT_LPF_FREQ 0.5 //Hz
#define TBATT_LPF_FREQ 1.0 //Hz

#define CUR_DIFF 0.05 //minimal difference between not filtered and filtered current
                      //values(in Amps) when impedance calculation is possible
#define R_RC 0.1 //Tau constant for the impedance  LPF filter

#pragma pack(push, 1)
typedef struct
{
    float x;
    float y;
} xy_t;
#pragma pack(pop)

class Device {

public:
	Device();
	virtual ~Device();

	uint8_t* txData(uint8_t index = 0) {return &m_txData[index];};
	uint8_t* rxData(uint8_t index = 0) {return &m_rxData[index];};

    config_t& config() {return m_config;};
	info_t& info() {return m_info;};

	void saveConfig();
	void convertAdcData();
	bool checkVbatVload();
	void calculateBatConsumption(float delta_time);
	void calculateBatRes(float delta_time);
	void UpdateBattery(timeUs_t currentTimeUs);
	float getCapTempFactor();

private:
	Flash m_flash;

	uint8_t m_txData[CAN_PACK_SIZE] = {0};
	uint8_t m_rxData[CAN_PACK_SIZE] = {0};

	config_t m_config;
	info_t m_info;

	const float vRef = 3.0;
	const float vBias = 1.5;

    //parameters snapshot, gotten from the m_info
	float iBat = 0; //averaged current snapshot
	float vBat = 0; //averaged voltage snapshot

    // resistance estimate
    float iBat_max = 0;   // maximum current since start-up
    float vBat_ref = 0;   // voltage used for maximum resistance calculation
    float iBat_ref = 0;   // current used for maximum resistance calculation

    //filters
    PT1Filter vBatFilter;
    PT1Filter iBatFilter;
    PT1Filter tBatFilter;
    float iBat_filt = 0;  // filtered current
    float vBat_filt = 0;  // filtered voltage
    float tBat_filt = 0;  // filtered temperature

    timeUs_t previousTimeUs = 0;  // system time of last resistance estimate update

    static const xy_t TableTempCapacity[];

    float IntrpltNewtForward(float x, const xy_t *xy, uint32_t n);
    float IntrpltNewtBackward(float x, const xy_t* xy, uint32_t n);
    float IntrpltNewton(float x, const xy_t* xy, uint32_t n);
};

#endif /* DEVICE_H_ */
