#ifndef FLASH_H_
#define FLASH_H_

#include "stm32f3xx_hal.h"
#include "Converter.h"

//*****************************************************************************
//
//*****************************************************************************
class Flash {
public:
	Flash();
	virtual ~Flash();

	bool writeData16(uint32_t addr, uint16_t *data, uint16_t dataSize);
    bool writeData32(uint32_t addr, uint32_t *data, uint16_t dataSize);
    bool erasePages(uint32_t addr, uint32_t num, uint32_t  *faultyPageAddr);
    void readData8(uint32_t addr, uint8_t *data, uint16_t dataSize);
    void readData32(uint32_t addr, uint32_t *data, uint16_t dataSize);
    void readData16(uint32_t addr, uint16_t *data, uint16_t dataSize);
    bool erasePageAndWrite(uint32_t addr, uint32_t *data, uint16_t dataSize);

private:
};

//*****************************************************************************
//
//*****************************************************************************
class ConfigStore : public Flash
{
public:
    ConfigStore();
    virtual ~ConfigStore();

    config_t getConfig();
    void setConfig(config_t& config);

private:
    static const uint32_t m_configAddress = 0x0803F000;
};

//*****************************************************************************
//
//*****************************************************************************
class EnergyStore : public Flash
{
public:
    EnergyStore();
    virtual ~EnergyStore();

    energy_t readData();
    bool writeData(energy_t *energy);

private:
    static const uint32_t m_energyAddress = 0x0803E000;
    static const uint32_t max_index = FLASH_PAGE_SIZE / sizeof(energy_t) ;
    bool invalidData = false;

    uint16_t getXorChecksum(energy_t *p_energy);
};

#endif /* FLASH_H_ */
