#include "iocan.h"
#include "string.h"

io_can::io_can(CAN_HandleTypeDef *can) {
	// TODO Auto-generated constructor stub
	this->can = can;
}

io_can::~io_can() {
	// TODO Auto-generated destructor stub
}

CAN_HandleTypeDef* io_can::instance(void){ return can; }

//-------------------------------------------------------------------------

void io_can::initFilter(uint32_t idHigh, uint32_t idLow, uint32_t maskIdHigh, uint32_t maskIdLow){
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = idHigh;
	sFilterConfig.FilterIdLow = idLow;
	sFilterConfig.FilterMaskIdHigh = maskIdHigh;
	sFilterConfig.FilterMaskIdLow = maskIdLow;
	sFilterConfig.FilterFIFOAssignment=CAN_FILTER_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	HAL_CAN_ConfigFilter(can, &sFilterConfig);
	HAL_CAN_Start(can);
	HAL_CAN_ActivateNotification(can, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void io_can::disable(){HAL_CAN_Stop(can);}

//-------------------------------------------------------------------------

bool io_can::send(uint32_t thisDevAdr, uint8_t *data, uint8_t size){
	HAL_StatusTypeDef halStatus = HAL_OK;

	canTxHeader.StdId = thisDevAdr;
	canTxHeader.ExtId = 0x00;
	canTxHeader.RTR = CAN_RTR_DATA;
	canTxHeader.IDE = CAN_ID_STD;
	canTxHeader.DLC = size;
	canTxHeader.TransmitGlobalTime = DISABLE;

	if(size > cPackSize) size = cPackSize;

	memset(txBuf, 0, cPackSize);
	memcpy(txBuf, data, size);
	halStatus = HAL_CAN_AddTxMessage(can, &canTxHeader, txBuf, &TxMailBox);
	return status(halStatus);
}

//-------------------------------------------------------------------------

bool io_can::read(uint32_t &txDevAdr, uint8_t *data, uint8_t size){
	HAL_StatusTypeDef halStatus = HAL_OK;

	halStatus = HAL_CAN_GetRxMessage(can, CAN_RX_FIFO0, &canRxHeader, data);
	txDevAdr = canRxHeader.StdId;
	size = canRxHeader.DLC;
	return status(halStatus);
}

//-------------------------------------------------------------------------



//-------------------------------------------------------------------------



//-------------------------------------------------------------------------
