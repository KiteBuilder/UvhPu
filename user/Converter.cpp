#include <Converter.h>
#include <math.h>

/**
  * @brief Constructor
  * @param None
  * @retval None
  */
Converter::Converter() {
	// TODO Auto-generated constructor stub

}

/**
  * @brief Destructor
  * @param None
  * @retval None
  */
Converter::~Converter() {
	// TODO Auto-generated destructor stub
}

/**
  * @brief
  * @param None
  * @retval None
  */
void Converter::floatToSfloat(float &Value, sfloat_t &Result){
	float  tmp_fraction;
	double tmp_integer;

	tmp_fraction = modf(Value, &tmp_integer);
	tmp_fraction = abs(tmp_fraction);

	Result.i = (int8_t)tmp_integer;
	Result.f = (int8_t)(100 * tmp_fraction);
}

/**
  * @brief
  * @param None
  * @retval None
  */
void Converter::sFloatToFloat(sfloat_t &Value, float &Result){
	float  tmp_fraction;
	tmp_fraction = 0.01 * Value.f;
	Result = Value.i + tmp_fraction;
}

/**
  * @brief
  * @param None
  * @retval None
  */
float Converter::adcToVoltage(float value, float v_ref, float gain){
	return static_cast<float>((value) * v_ref / (gain * 65536));
}

/**
  * @brief
  * @param None
  * @retval None
  */
float Converter::toDividerVoltage(float value, float r_up, float r_down){
	return static_cast<float>(value * (r_up + r_down)  / r_down);
}

/**
  * @brief
  * @param None
  * @retval None
  */
void Converter::addToAverage(average_t &avrg, float value)
{
	if(avrg.cnt >= AVRG_SIZE)
	{
	    for (int i = 0; i < AVRG_SIZE; i++)
	    {
	        value += avrg.buf[i];
	    }

	    value /= (AVRG_SIZE + 1);
		avrg.cnt = 0;
	}

	avrg.buf[avrg.cnt++] = value;
}

/**
  * @brief
  * @param None
  * @retval None
  */
float Converter::generateAverageVar(average_t &avrg)
{
	float val = 0;
	uint8_t avSize = avrg.cnt;

    if (avSize == 0)
    {
        return 0;
	}

	for (int i = 0; i < avSize; i++)
	{
		val += avrg.buf[i];
	}

	return (val / avSize);
}

/**
  * @brief
  * @param None
  * @retval None
  */
bool Converter::isEqual(float data1, float data2)
{
	int absCheck = abs((data1-data2)*1000);
    if( absCheck < m_min_rank ) return true;
    else return false;
}

/**
  * @brief
  * @param None
  * @retval None
  */
uint8_t Converter::calcCrc(uint8_t *vec, uint16_t size)
{
    uint8_t crc = 0xFF;

    for (int i = 0; i < size; i++)
    {
        crc ^= vec[i];

        for (int j = 0; j < size-1; j++)
        {
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
        }
    }

    return crc;
}
