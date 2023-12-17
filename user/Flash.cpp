/*
 * Flash.cpp
 *
 *  Created on: Aug 9, 2021
 *      Author: regula_user
 */

#include <Flash.h>

Flash::Flash() {
	// TODO Auto-generated constructor stub

}

Flash::~Flash() {
	// TODO Auto-generated destructor stub
}

config_t Flash::getConfig()
{
	config_t config;

	read(m_configAddress, (uint8_t*)&config, sizeof(config_t));

	if(config.id == 0xFFFFFFFF){
		config_t defConfig;
		return defConfig;
	}

	return config;
}

void Flash::setConfig(config_t& config)
{
	write(m_configAddress, (uint32_t*)&config, sizeof(config_t));
}

bool Flash::write(uint32_t addr, uint32_t *data, uint16_t dataSize)
{
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError = 0;

	HAL_FLASH_Unlock();

	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = addr;
	EraseInitStruct.NbPages     = 2;

	HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);

	for(uint16_t i = 0, j = 0; i < dataSize; i+=4, j++)
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+i, data[j]) != HAL_OK){
			HAL_FLASH_Lock();
			return false;
		}

	HAL_FLASH_Lock();
	return true;
}

void Flash::read(uint32_t addr, uint8_t *data, uint16_t dataSize)
{
	uint32_t mem_ptr;
	uint32_t mem_data_end;
	uint8_t  *data_ptr;

	mem_ptr = addr;
	mem_data_end = addr + dataSize;
	data_ptr = data;

	while(mem_ptr < mem_data_end){
		*data_ptr = *(uint8_t*)mem_ptr;
		mem_ptr++;
		data_ptr++;
	}
}
