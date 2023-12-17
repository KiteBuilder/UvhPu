#ifndef CONVERTER_H_
#define CONVERTER_H_

#include "types.h"

class Converter {
public:
	Converter();
	virtual ~Converter();

	static void floatToSfloat(float &Value, sfloat_t &Result);
	static void sFloatToFloat(sfloat_t &Value, float &Result);

	static float adcToVoltage(float value, float v_ref, float gain);
	static float toDividerVoltage(float value, float r_up, float r_down);

	static void addToAverage(average_t &avrg, float value);
	static float generateAverageVar(average_t &avrg);

	static bool isEqual(float data1, float data2);
	static uint8_t calcCrc(uint8_t *vec, uint16_t size);

private:
	static const uint8_t m_min_rank = 1;
};

#endif /* CONVERTER_H_ */
