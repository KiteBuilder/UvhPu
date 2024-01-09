#include <Device.h>
#include <Converter.h>
#include <string.h>
#include "types.h"
#include "b57861s103.h"

// x - temperature in Сelsius, y - relative capacity from 0.0 to 1.0
const xy_t Device::TableTempCapacity[] = {
        {-10.0 , 0.2 },
        {-5.0  , 0.38},
        { 0.0  , 0.5 },
        { 5.0  , 0.66},
        { 10.0 , 0.78},
        { 15.0 , 0.85},
        { 20.0 , 0.92},
        { 25.0 , 1.0 },
        { 30.0 , 0.98},
        { 35.0 , 0.94},
        { 40.0 , 0.9 },
        { 45.0 , 0.86}};

/**
  * @brief Constructor
  * @param None
  * @retval None
  */
Device::Device()
{
	m_config = m_flash.getConfig();
}

/**
  * @brief Destructor
  * @param None
  * @retval None
  */
Device::~Device()
{

}

/**
  * @brief
  * @param None
  * @retval None
  */
void Device::saveConfig()
{
	m_flash.setConfig(m_config);
}

/**
  * @brief
  * @param None
  * @retval None
  */
void Device::convertAdcData()
{
	float iBat = Converter::generateAverageVar(m_info.iBat) - m_config.iBatOffset;
	float iMon = Converter::generateAverageVar(m_info.iMon);
	float vBat = Converter::generateAverageVar(m_info.vBat);
	float vOut = Converter::generateAverageVar(m_info.vOut);
	float tempBat = Converter::generateAverageVar(m_info.tempBat);

	iBat = Converter::adcToVoltage(iBat, vRef, 0.5);	//Completely the Bullshit ---> 0.25 for sigmadelta ADC to 16384 full range
	iMon = Converter::adcToVoltage(iMon, vRef, 0.5);	//True ---> 0.5  for sigmadelta ADC to 32768 full range
	vBat = Converter::adcToVoltage(vBat, vRef, 0.5);
	vOut = Converter::adcToVoltage(vOut, vRef, 0.5);
	tempBat = Converter::adcToVoltage(tempBat, vRef, 0.5);

	iBat = iBat / (20 * R_SHUNT(m_config.shunt));
	//Next commented statement is valid if iBatOffset not subtracted and gain is 0.25
	//iBat = (iBat - vBias) / (20 * R_SHUNT(m_config.shunt));
	iMon = iMon / (20 * R_SHUNT(m_config.shunt));

	m_info.iBat.val = m_config.iBatK * iBat + m_config.iBatB;
	m_info.iMon.val = m_config.iMonK * iMon + m_config.iMonB;

	m_info.vBat.val = m_config.vBatK * Converter::toDividerVoltage(vBat, R_DIV1, R_DIV2) + m_config.vBatB;
	m_info.vOut.val = m_config.vOutK * Converter::toDividerVoltage(vOut, R_DIV1, R_DIV2) + m_config.vOutB;

	m_info.tempBat.val = m_config.tempBatK * B57861s103::toTemp(2490, tempBat, vRef) + m_config.tempBatB;
	m_info.pBat = m_info.iBat.val * m_info.vBat.val;
}

/**
  * @brief
  * @param None
  * @retval None
  */
bool Device::checkVbatVload()
{
	float vBat = Converter::generateAverageVar(m_info.vBat);
	float vOut = Converter::generateAverageVar(m_info.vOut);
	float iBatOffset = Converter::generateAverageVar(m_info.iBat);
	bool res = false;

	vOut = Converter::adcToVoltage(vOut, vRef, 0.5);
	vBat = Converter::adcToVoltage(vBat, vRef, 0.5);

	m_info.vBat.val = Converter::toDividerVoltage(vBat, R_DIV1, R_DIV2);
	m_info.vOut.val = Converter::toDividerVoltage(vOut, R_DIV1, R_DIV2);

	if (m_info.vBat.val > m_info.vOut.val)
	{
		m_info.iBatOffset = iBatOffset;
		m_info.iBat.clear();
		res = true;
	}

	return res;
}

/**
  * @brief Calculate battery power consumption
  * @param curremtTime: current time in microseconds
  * @retval None
  */
void Device::calculateBatConsumption(float delta_time)
{
    float drawn_mah = iBat_filt * delta_time * AS_TO_MAH;
    m_info.cBat += drawn_mah; //in mAh
    m_config.cBatMod += fabs(drawn_mah); //accumulated capacity module, includes sum of both consumed and charged capacity

    float vHeatLosses = iBat_filt * m_info.resBat; //Voltage losses on the internal battery resistance
    m_info.eBat += 0.001 * drawn_mah * (vBat_filt + vHeatLosses); //in Wh

    //Calculate the rest power in the battery taking to account current temperature and power that was consumed
    //!!! This calculation takes approximately 650us and can be done preliminary in the beginning for whole
    //temperature range that we need with requested temperature resolution.
    //Suggest temperature step is 1.0 Celsius
    float tempFactor = getCapTempFactor();
    m_info.cBatRest = m_config.cInitial * tempFactor - m_info.cBat;

    //The life cycle counter should be incremented if the accumulated capacity module two times greater than the real battery capacity
    if (m_config.cBatMod >= m_config.cInitial * tempFactor * 2.0)
    {
        m_config.cBatMod = 0; //should be reset to start a new life cycle
        ++m_config.lifeCycles;
    }

    if (m_info.eBat > m_config.eInitial)
    {
        m_info.eBat = m_config.eInitial;
    }

    if (m_info.cBat > m_config.cInitial)
    {
        m_info.cBat = m_config.cInitial;
    }
}

/**
  * @brief Update battery resistance estimate
  * faster rates of change of the current and voltage readings cause faster updates to the resistance estimate
  * the battery resistance is calculated by comparing the latest current and voltage readings to a low-pass filtered current and voltage
  *  high current steps are integrated into the resistance estimate by varying the time constant of the resistance filter
  * @param currenTimeUs: current time in microseconds
  * @retval None
  */
void Device::calculateBatRes(float delta_time)
{
    if (!is_positive(iBat))
    {
        return;
    }

    // update maximum current seen since startup and protect against divide by zero
    iBat_max = (iBat_max > iBat) ? iBat_max : iBat;
    float iBat_delta = iBat - iBat_filt;

    if (is_zero(iBat_delta) || fabsf(iBat_delta) < CUR_DIFF)
    {
        return;
    }

    // update reference voltage and current
    if (vBat > vBat_ref)
    {
        vBat_ref = vBat;
        iBat_ref = iBat;
    }

    float res_alpha =  R_RC * fabsf(iBat_delta)/ iBat_max;

    float resBat_estimate = (vBat_filt - vBat)/iBat_delta;

    if (is_positive(resBat_estimate))
    {
        m_info.resBat = m_info.resBat + res_alpha * (resBat_estimate - m_info.resBat);
    }

    // calculate maximum resistance
    if ((vBat_ref > vBat) && (iBat > iBat_ref))
    {
        float resBat_max = (vBat_ref - vBat) / (iBat - iBat_ref);
        //correct current impedance if calculated maximum became lesser
        m_info.resBat =  (m_info.resBat < resBat_max) ? m_info.resBat : resBat_max;
    }
}

/**
  * @brief  Update battery parameters task
  * @param curremtTime: current time in microseconds
  * @retval None
  */
void Device::UpdateBattery(timeUs_t currentTimeUs)
{
    iBat = m_info.iBat.val;
    vBat = m_info.vBat.val;

    // calculate time since last update
    float delta_time = US2S(currentTimeUs - previousTimeUs);
    previousTimeUs = currentTimeUs;

    // update the filtered voltage and currents
    if (vBatFilter.isFirstLoad())
    {
        vBatFilter.FilterSetVal(vBat);
    }
    vBat_filt = vBatFilter.FilterApply(vBat, delta_time, VBATT_LPF_FREQ);

    if (iBatFilter.isFirstLoad())
    {
        iBatFilter.FilterSetVal(iBat);
    }
    iBat_filt = iBatFilter.FilterApply(iBat, delta_time, IBATT_LPF_FREQ);

    m_info.iBatFilt = iBat_filt;
    m_info.vBatFilt = vBat_filt;

    if (tBatFilter.isFirstLoad())
    {
        tBatFilter.FilterSetVal(m_info.tempBat.val);
    }
    tBat_filt = tBatFilter.FilterApply(m_info.tempBat.val, delta_time, TBATT_LPF_FREQ);

    //calculate battery impedance and power consumption
    calculateBatRes(delta_time);
    calculateBatConsumption(delta_time);

    //Calculate the resting voltage
    m_info.vRest = vBat + iBat *  m_info.resBat;
    m_info.vRest = (m_info.vRest > vBat) ? m_info.vRest : vBat;
}

/**
  * @brief Forward Newton Interpolation method
  * @param x: x-axis value for which we have to find teh y value
  *        xy_t: array with (x,y) control points
  *        n: amount of control points
  * @retval float: Calculated y value
  */
float Device::IntrpltNewtForward(float x, const xy_t *xy, uint32_t n)
{
    //Allocat memory for two dimensions array
    float **y  = new float*[n];
    for (uint32_t i = 0; i < n; i++)
    {
        y[i] = new float[n];
    }

    //To load allocated array with a f(x) values
    for (uint32_t i = 0; i < n; i++)
    {
        y[i][0] = xy[i].y;
    }

    //Calculate the forward difference table
    for (uint32_t i = 1; i < n; i++)
    {
        for (uint32_t j = 0; j < n-i; j++)
        {
            y[j][i] = y[j+1][i-1] - y[j][i-1];
        }
    }

    //Interpolate
    float sum = y[0][0];
    float u = (x - xy[0].x) / (xy[1].x - xy[0].x);
    float p = 1.0;
    for (uint32_t i = 1; i < n; i++)
    {
        p *= (u - i + 1)/i;
        sum = sum + p * y[0][i];
    }

    //To free allocated memory
    for (uint32_t i = 0; i < n; i++) {
        delete[] y[i];
    }
    delete[] y;

    return sum;
}

/**
  * @brief Backward Newton Interpolation method
  * @param x: x-axis value for which we have to find teh y value
  *        xy_t: array with (x,y) control points
  *        n: amount of control points
  * @retval float: Calculated y value
  */
float Device::IntrpltNewtBackward(float x, const xy_t* xy, uint32_t n)
{
    //Allocat memory for two dimensions array
    float** y = new float* [n];
    for (uint32_t i = 0; i < n; i++)
    {
        y[i] = new float[n];
    }
    //To load allocated array with a f(x) values
    for (uint32_t i = 0; i < n; i++)
    {
        y[i][0] = xy[i].y;
    }

    //Calculate the backard difference table
    for (uint32_t i = 1; i < n; i++)
    {
        for (uint32_t j = i; j < n; j++)
        {
            y[j][i] = y[j][i - 1] - y[j - 1][i - 1];
        }
    }

    //Interpolate
    float sum = y[n-1][0];
    float u = (x - xy[n-1].x) / (xy[1].x - xy[0].x);
    float p = 1.0;
    for (uint32_t i = 1; i < n; i++)
    {
        p *= (u + i - 1) / i;
        sum = sum + p * y[n - 1][i];
    }

    //To free allocated memory
    for (uint32_t i = 0; i < n; i++) {
        delete[] y[i];
    }
    delete[] y;

    return sum;
}

/**
  * @brief Newton interpolation method
  * @param x: x-axis value for which we have to find teh y value
  *        xy_t: array with (x,y) control points
  *        n: amount of control points
  * @retval float: Calculated y value
  */
float Device::IntrpltNewton(float x, const xy_t* xy, uint32_t n)
{
     uint32_t k = 0;
    while (k < n)
    {
        if (x < xy[k].x)
        {
            break;
        }
        ++k;
    }

    float result = 0.0;
    if (k < (n >> 1))
    {
        result = IntrpltNewtForward(x, xy, n);
    }
    else
    {
        result = IntrpltNewtBackward(x, xy, n);
    }
    return result;
}

/**
  * @brief This method returns relative dependence of the battery capacity on the
  *        measured  battery temperature -  from 0.0 to 1.0,
  * @param
  * @retval float: factor value from 0.0 to 1.0
  */
float Device::getCapTempFactor()
{
    return IntrpltNewton(tBat_filt, TableTempCapacity, sizeof(TableTempCapacity)/sizeof(xy_t));
}
