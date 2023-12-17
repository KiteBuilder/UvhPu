#include <Protocol.h>

Protocol::Protocol() {
	// TODO Auto-generated constructor stub

}

Protocol::~Protocol() {
	// TODO Auto-generated destructor stub
}

void Protocol::addByte(uint8_t* data, uint8_t value)
{
	data[0] = value;
}

void Protocol::add2Bytes(uint8_t* data, int16_t value)
{
	memcpy(&data[0], &value, 2);
}

void Protocol::addSFloat(uint8_t* data, float value)
{
	sfloat_t val = {0,0};

	Converter::floatToSfloat(value, val);

	memcpy(&data[0], &val, 2);
}

void Protocol::addFloat(uint8_t* data, float value)
{
	memcpy(&data[0], &value, 4);
}

void Protocol::addFlags(uint8_t* data, const flags_t& flags)
{
	uint8_t byte = 0;

	if(flags.wakeUpPB) 		 byte |= 0x01;
	if(flags.wakeUpHOLD) 	 byte |= 0x02;
	if(flags.mcu) 			 byte |= 0x04;
	if(flags.pbINT) 		 byte |= 0x08;
	if(flags.heater) 	  	 byte |= 0x0F;
	if(flags.faultLTC) 		 byte |= 0x10;
	if(flags.faultVbatVLoad) byte |= 0x20;

	data[0] = byte;
}

void Protocol::parceData(Command cmd, uint8_t* data, config_t& config)
{
	switch(cmd){

	case Command::ConfDevId:
		memcpy(&config.id, &data[0], 4);
		break;

	case Command::ConfVbat:
		memcpy(&config.vBatK, &data[0], 4);
		memcpy(&config.vBatB, &data[4], 4);
		break;

	case Command::ConfIbat:
		memcpy(&config.iBatK, &data[0], 4);
		memcpy(&config.iBatB, &data[4], 4);
		break;

	case Command::ConfVout:
		memcpy(&config.vOutK, &data[0], 4);
		memcpy(&config.vOutB, &data[4], 4);
		break;

	case Command::ConfImon:
		memcpy(&config.iMonK, &data[0], 4);
		memcpy(&config.iMonB, &data[4], 4);
		break;

	case Command::ConfTbat:
		memcpy(&config.tempBatK, &data[0], 4);
		memcpy(&config.tempBatB, &data[4], 4);
		break;

	case Command::ConfVlimits:
		memcpy(&config.vLow, &data[0], 4);
		memcpy(&config.vMax, &data[4], 4);
		break;

	case Command::ConfIcharge:
		memcpy(&config.iLowCharge, &data[0], 4);
		memcpy(&config.iFullCharge, &data[4], 4);
		break;

	case Command::ConfECinit:
		memcpy(&config.eInitial, &data[0], 4);
		memcpy(&config.cInitial, &data[4], 4);
		break;

	case Command::Conf1:
		config.shunt = data[0];
		config.fTelemetry = data[1];
		config.resHeater = data[2];
		memcpy(&config.tempBatMax, &data[3], 4);
		break;

	default: break;
	}
}

