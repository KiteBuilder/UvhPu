#ifndef SRC_USER_IOCAN_H_
#define SRC_USER_IOCAN_H_

#include "port.h"

class io_can: private port {
public:
	io_can(CAN_HandleTypeDef *can);
	virtual ~io_can();

	CAN_HandleTypeDef *instance(void);
	void initFilter(uint32_t idHigh=0x0000, uint32_t idLow=0x0000,
					uint32_t maskIdHigh=0x0000, uint32_t maskIdLow=0x0000);
	void disable();

	bool send(uint32_t thisDevAdr, uint8_t *data, uint8_t size=cPackSize);
	bool read(uint32_t &txDevAdr, uint8_t *data, uint8_t size=cPackSize);

private:
	static const uint8_t cPackSize = 8;
	CAN_HandleTypeDef *can;

	CAN_TxHeaderTypeDef canTxHeader;
	CAN_RxHeaderTypeDef canRxHeader;
	CAN_FilterTypeDef sFilterConfig;
	uint32_t TxMailBox = 0;
	uint8_t txBuf[cPackSize] = {0};
};

#endif /* SRC_USER_IOCAN_H_ */
