#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "stm32f3xx_hal.h"
#include <string.h>
#include "Converter.h"

#define CAN_PACK_SIZE	8

enum Command : uint8_t {
	Pack1 = 0x01,
    Pack2 = 0x02,
    Pack3 = 0x03,
    Pack4 = 0x04,
    Pack5 = 0x05,
    Pack6 = 0x06,

	ConfSave = 0x11,
	ConfDevId = 0x12,
	ConfVbat = 0x13,
	ConfIbat = 0x14,
	ConfVout = 0x15,
	ConfImon = 0x16,
	ConfTbat = 0x17,
	ConfVlimits = 0x18,
	ConfIcharge = 0x19,
	ConfECinit = 0x1A,
	Conf1 = 0x1B,
	ConfNewBat = 0x1C,
};

class Protocol {

public:
	Protocol();
	virtual ~Protocol();

    static void addByte(uint8_t* data, uint8_t value);
    static void add2Bytes(uint8_t* data, int16_t value);
    static void addSFloat(uint8_t* data, float value);
    static void addFloat(uint8_t* data, float value);
    static void addFlags(uint8_t* data, const flags_t& flags);

    static void parceData(Command cmd, uint8_t* data, config_t& config, energy_t& energy);
};

#endif /* PROTOCOL_H_ */
