#ifndef DEVICE_H_
#define DEVICE_H_

#include "Protocol.h"
#include "Flash.h"
#include "def_ports.h"
#include "time.h"
#include "Filters.h"
#include "Interpolate.h"


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
#define R_RC 0.1 //Tau constant for the impedance LPF filter

#define NUM_OF_TABLES 2 //number of tables for capacity correction algorithm

class Device {

public:
	Device();
	virtual ~Device();

	uint8_t* txData(uint8_t index = 0) {return &m_txData[index];};
	uint8_t* rxData(uint8_t index = 0) {return &m_rxData[index];};

    config_t& config() {return m_config;};
	info_t& info() {return m_info;};
	energy_t& energy() {return m_energy;};
	table_t* getTables() {return tables;}

	void saveConfig();
	void saveEnergy();
    void saveTables();
	void convertAdcData();
	bool checkVbatVload();
	void calculateBatConsumption(float delta_time);
	void calculateBatRes(float delta_time);
	void UpdateBattery(timeUs_t currentTimeUs);
	float getCapTempFactor();
	float getLifeFactor();

private:
	ConfigStore m_ConfigStore;
	EnergyStore m_EnergyStore;
	TablesStore m_TablesStore;

	uint8_t m_txData[CAN_PACK_SIZE] = {0};
	uint8_t m_rxData[CAN_PACK_SIZE] = {0};

	config_t m_config;
	info_t m_info;
	energy_t m_energy;

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

    table_t tables[NUM_OF_TABLES]; //[0] capacity from the temperature dependency table
                                   //[1] capacity from the life cycle dependency table

    static xy_t TableTempCapacity[];
    static xy_t TableLifeCapacity[];

    NewtonIntrplt Newton;
    LinearIntrplt Linear;
};

#endif /* DEVICE_H_ */
