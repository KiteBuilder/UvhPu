/*
 * Flash.cpp
 *
 *  Created on: Aug 9, 2021
 *      Author: regula_user
 */

#include <Flash.h>

//*****************************************************************************
//Flash class with NORFlash primitives
//*****************************************************************************
/**
  * @brief Constructor
  * @param None
  * @retval None
  */
Flash::Flash() {
	// TODO Auto-generated constructor stub

}

/**
  * @brief Desstructor
  * @param None
  * @retval None
  */
Flash::~Flash() {
	// TODO Auto-generated destructor stub
}

/**
  * @brief Erase a page and then write a data there
  * @param addr - page aligned 32bit address
  *        data -  pointer to the 32bit data buffer
  *        dataSize - buffer size in bytes
  * @retval None
  */
bool Flash::eraseAndWrite(uint32_t addr, uint32_t *data, uint16_t dataSize)
{
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError = 0;

	HAL_FLASH_Unlock();

	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = addr;
	EraseInitStruct.NbPages     = 2;

	HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

	for (uint16_t i = 0, j = 0; i < dataSize; i+=4, j++)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+i, data[j]) != HAL_OK)
		{
			HAL_FLASH_Lock();
			return false;
		}
	}

	HAL_FLASH_Lock();
	return true;
}

/**
  * @brief Write 16bit data array to the NOR Flash
  * @param addr - 16bit aligned address
  *        data -  pointer to the 16bit data buffer
  *        dataSize - buffer size in 32bit words
  * @retval true if programmed successfully
  */
bool Flash::writeData16(uint32_t addr, uint16_t *data, uint16_t dataSize)
{
    HAL_FLASH_Unlock();

    for (uint16_t i = 0; i < dataSize; i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + (i << 1), data[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return false;
        }
    }

    HAL_FLASH_Lock();
    return true;
}

/**
  * @brief Write 32bit data array to the NOR Flash
  * @param addr - 32bit aligned address
  *        data -  pointer to the 32bit data buffer
  *        dataSize - buffer size in 32bit words
  * @retval true if programmed successfully
  */
bool Flash::writeData32(uint32_t addr, uint32_t *data, uint16_t dataSize)
{
    HAL_FLASH_Unlock();

    for (uint16_t i = 0; i < dataSize; i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + (i << 2), data[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return false;
        }
    }

    HAL_FLASH_Lock();
    return true;
}

/**
  * @brief Erase NOR Flash page/pages
  * @param addr - 32bit address within the  page
  *        num - number of pages to be erased
  *        faultuPageAddr - if erase failed the last page address should be stored there, set to NULL if not necessary
  * @retval None
  */
bool Flash::erasePages(uint32_t addr, uint32_t num, uint32_t *faultyPageAddr)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError = 0;
    bool status = false;

    HAL_FLASH_Unlock();

    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = addr;
    EraseInitStruct.NbPages     = num;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        status = true;
    }
    else
    {
        if (faultyPageAddr != NULL)
        {
            *faultyPageAddr = PAGEError;
        }
    }

    HAL_FLASH_Lock();
    return status;
}

/**
  * @brief Read array of bytes
  * @param addr - address to start reading
  *        data -  pointer to the 8bit data buffer
  *        dataSize - buffer size in bytes
  * @retval None
  */
void Flash::readData8(uint32_t addr, uint8_t *data, uint16_t dataSize)
{
	uint32_t mem_addr;
	uint32_t mem_data_end;
	uint8_t  *data_ptr;

	mem_addr = addr;
	mem_data_end = addr + dataSize;
	data_ptr = data;

	while (mem_addr < mem_data_end)
	{
		*data_ptr = *(uint8_t*)mem_addr;
		mem_addr++;
		data_ptr++;
	}
}

/**
  * @brief Read array of uint16
  * @param addr - 16bit aligned address
  *        data -  pointer to the 16bit data buffer
  *        dataSize - buffer size in 16bit words
  * @retval None
  */
void Flash::readData16(uint32_t addr, uint16_t *data, uint16_t dataSize)
{
    uint32_t mem_addr;
    uint32_t mem_data_end;
    uint16_t  *data_ptr;

    mem_addr = addr;
    mem_data_end = addr + (dataSize << 1);
    data_ptr = data;

    while (mem_addr < mem_data_end)
    {
        *data_ptr = *(uint16_t*)mem_addr;
        mem_addr += 2;
        data_ptr++;
    }
}

/**
  * @brief Read array of uint32
  * @param addr - 32bit aligned address
  *        data -  pointer to the 32bit data buffer
  *        dataSize - buffer size in 32bit words
  * @retval None
  */
void Flash::readData32(uint32_t addr, uint32_t *data, uint16_t dataSize)
{
    uint32_t mem_addr;
    uint32_t mem_data_end;
    uint32_t  *data_ptr;

    mem_addr = addr;
    mem_data_end = addr + (dataSize << 2);
    data_ptr = data;

    while (mem_addr < mem_data_end)
    {
        *data_ptr = *(uint32_t*)mem_addr;
        mem_addr += 4;
        data_ptr++;
    }
}

//*****************************************************************************
//Config storage class
//*****************************************************************************
/**
  * @brief Constructor
  * @param None
  * @retval None
  */

ConfigStore::ConfigStore() {

}

/**
  * @brief Destructor
  * @param None
  * @retval None
  */
ConfigStore::~ConfigStore() {

}

/**
  * @brief Get config from the NOR Flash memory
  * @param None
  * @retval config_t structure
  */
config_t ConfigStore::getConfig()
{
    config_t config;

    readData8(m_configAddress, (uint8_t*)&config, sizeof(config_t));

    if (config.id == 0xFFFFFFFF)
    {
        config_t defConfig;
        return defConfig;
    }

    return config;
}

/**
  * @brief Save config to the NOR Flash
  * @param config_t type referenced variable
  * @retval None
  */
void ConfigStore::setConfig(config_t& config)
{
    eraseAndWrite(m_configAddress, (uint32_t*)&config, sizeof(config_t));
}

//*****************************************************************************
//Energy storage class
//*****************************************************************************

/**
  * @brief Constructor
  * @param None
  * @retval None
  */
EnergyStore::EnergyStore(){

}

/**
  * @brief Destructor
  * @param None
  * @retval None
  */
EnergyStore::~EnergyStore() {

}

/**
  * @brief Read energy data from the NOR flash
  * @param None
  * @retval energy_t structure
  */
energy_t EnergyStore::readData()
{
    uint32_t address = m_energyAddress;
    uint32_t rd_index;
    uint32_t check_index = 0;

    //Trying to find the last written data structure by the index field
    //Unwritten memory should be 0xFFFFFFFF
    do{
        readData32(address, &rd_index, 1);
        if (rd_index != 0xFFFFFFFF)
        {
            if (rd_index == check_index)
            {
                address += sizeof(energy_t);
                ++check_index;
            }
            else
            {
                invalidData = true;
                break;
            }
        }
    } while(rd_index != 0xFFFFFFFF && check_index < max_index);

    energy_t energy;
    if (address > m_energyAddress && !invalidData) //check that was read as minimum as the one valid index
    {
        address -= sizeof(energy_t); //to correct the address to the beginning of the last valid data

        readData8(address, (uint8_t*)&energy, sizeof(energy_t)); //read data

        if (getXorChecksum(&energy) != energy.checksum)
        {
            invalidData = true;

            energy_t defEnergy;
            return defEnergy;
        }
        else
        {
            ++energy.index; //to correct index  for the next write to the flash memory
        }
    }

    return energy;
}

/**
  * @brief Write Energy data to the NOR flash
  * @param energy_t pointer
  * @retval None
  */
bool EnergyStore::writeData(energy_t *energy)
{
    uint32_t address = m_energyAddress;
    uint32_t max_index = FLASH_PAGE_SIZE / sizeof(energy_t);

    if (energy->index >= max_index || invalidData) //check for the full page or invalid data
    {
        //if page is full or the last read data were invalid, set index to 0 and erase the page
        energy->index = 0;
        erasePages(address, 1, NULL);
    }
    else
    {
        //correct the address according to the index
        address += sizeof(energy_t) * energy->index;
    }

    energy->checksum = getXorChecksum(energy);

    return writeData32(address, (uint32_t*)energy, sizeof(energy_t)/sizeof(uint32_t));
}

/**
  * @brief Calculate XOR checksum
  * @param energy_t pointer
  * @retval uint16_t checksum
  */
uint16_t EnergyStore::getXorChecksum(energy_t *energy)
{
    uint16_t *buf = (uint16_t*)energy;
    uint32_t size = sizeof(energy_t)/sizeof(uint16_t) - 1; //-1 to eliminate the checksum field itself
    uint16_t checksum = buf[0];

    for (uint32_t i = 1; i < size; i++)
    {
        checksum ^= buf[i];
    }

    return checksum;
}
