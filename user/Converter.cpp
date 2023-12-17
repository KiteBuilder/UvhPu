#include <Converter.h>
#include <math.h>

Converter::Converter() {
	// TODO Auto-generated constructor stub

}

Converter::~Converter() {
	// TODO Auto-generated destructor stub
}
//-----------------------------------------------------------------------
void Converter::floatToSfloat(float &Value, sfloat_t &Result){
	float  tmp_fraction;
	double tmp_integer;

	tmp_fraction = modf(Value, &tmp_integer);
	tmp_fraction = abs(tmp_fraction);

	Result.i = (int8_t)tmp_integer;
	Result.f = (int8_t)(100 * tmp_fraction);
}

void Converter::sFloatToFloat(sfloat_t &Value, float &Result){
	float  tmp_fraction;

	tmp_fraction = 0.01 * Value.f;
	Result = Value.i + tmp_fraction;
}
//-----------------------------------------------------------------------

float Converter::adcToVoltage(float value, float v_ref, float gain){
	return static_cast<float>((value) * v_ref / (gain * 65536));
}

float Converter::toDividerVoltage(float value, float r_up, float r_down){
	return static_cast<float>(value * (r_up + r_down)  / r_down);
}
//-----------------------------------------------------------------------

void Converter::addToAverage(average_t &avrg, float value){
	if(avrg.cnt >= AVRG_SIZE) {
		avrg.cnt = 0;
		if(!avrg.fFull) avrg.fFull = true;
	}
	avrg.buf[avrg.cnt++] = value;
}

float Converter::generateAverageVar(average_t &avrg){
	float val = 0;
	uint8_t avSize = 1;
	if(avrg.fFull) avSize = AVRG_SIZE;
	else {
		if(avrg.cnt != 0) avSize = avrg.cnt;
		else return 0;
	}

	for(int i = 0; i < avSize; i++)
		val += avrg.buf[i];
	return (val / avSize);
}

//-----------------------------------------------------------------------

bool Converter::isEqual(float data1, float data2)
{
	int absCheck = abs((data1-data2)*1000);
    if( absCheck < m_min_rank ) return true;
    else return false;
}

uint8_t Converter::calcCrc(uint8_t *vec, uint16_t size){
    uint8_t crc = 0xFF;

    for(int i = 0; i < size; i++) {
        crc ^= vec[i];

        for(int j = 0; j < size-1; j++)
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }

    return crc;
}

