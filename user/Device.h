#ifndef DEVICE_H_
#define DEVICE_H_

#include "Protocol.h"
#include "Flash.h"
#include "def_ports.h"


#define PORT_JUMPER		GPIOB
#define PIN_JUMPER0		GPIO_PIN_3
#define PIN_JUMPER1		GPIO_PIN_4
#define PIN_JUMPER2		GPIO_PIN_5
#define PIN_JUMPER3		GPIO_PIN_6

#define R_SHUNT(flag)	(flag ? 0.0005 : 0.00025)

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
	void calculateBatCE();

private:
	Flash m_flash;

	uint8_t m_txData[CAN_PACK_SIZE] = {0};
	uint8_t m_rxData[CAN_PACK_SIZE] = {0};

	config_t m_config;
	info_t m_info;

	const float vRef = 3.0;
	const float vBias = 1.5;
};

#endif /* DEVICE_H_ */
