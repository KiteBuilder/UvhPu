#ifndef FLASH_H_
#define FLASH_H_

#include "stm32f3xx_hal.h"
#include "Converter.h"

class Flash {
public:
	Flash();
	virtual ~Flash();

	config_t getConfig();
	void setConfig(config_t& config);


private:
	static const uint32_t m_configAddress = 0x0803F000;

	bool write(uint32_t addr, uint32_t *data, uint16_t dataSize);
	void read(uint32_t addr, uint8_t *data, uint16_t dataSize);

};

#endif /* FLASH_H_ */
