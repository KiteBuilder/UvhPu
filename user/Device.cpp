#include <Device.h>
#include <Converter.h>
#include <string.h>
#include "types.h"
#include "b57861s103.h"

//These tables were built and based on the NCR18650GA characteristics

// x - temperature in Сelsius, y - relative capacity from 0.0 to 1.0
const xy_t Device::TableTempCapacity[] = {
        {-10.0 , 0.87 },
        {-5.0  , 0.90},
        { 0.0  , 0.92 },
        { 5.0  , 0.94},
        { 10.0 , 0.96},
        { 15.0 , 0.98},
        { 20.0 , 0.99},
        { 25.0 , 1.0},
        { 30.0 , 1.0},
        { 35.0 , 0.99},
        { 40.0 , 0.99},
        { 45.0 , 0.98},
        { 50.0 , 0.98}};

// x - life cycles, y - relative capacity from 0.0 to 1.0
const xy_t Device::TableLifeCapacity[] = {
        { 0.0    , 1.0 },
        { 50.0   , 0.92},
        { 100.0  , 0.82},
        { 150.0  , 0.80},
        { 200.0  , 0.76},
        { 250.0  , 0.74},
        { 300.0  , 0.69}};

/**
  * @brief Constructor
  * @param None
  * @retval None
  */
Device::Device(): Newton(TableTempCapacity, sizeof(TableTempCapacity)/sizeof(xy_t)), Linear(TableLifeCapacity, sizeof(TableLifeCapacity)/sizeof(xy_t))
{
    //Newton.Init(TableTempCapacity, sizeof(TableTempCapacity)/sizeof(xy_t));
    //Linear.Init(TableTempCapacity, sizeof(TableTempCapacity)/sizeof(xy_t));

	m_config = m_ConfigStore.getConfig();
	m_energy = m_EnergyStore.readData();

	//if previously was charged more than it was drawn all counters should be reset because
	//it's logically correct that on the fully charged battery we have to start from the scratch
	if (is_negative(m_energy.cBat))
	{
	    m_energy.cBat = 0;
	}

    if (is_negative(m_energy.eBat))
    {
        m_energy.eBat = 0;
    }
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
	m_ConfigStore.setConfig(m_config);
}

/**
  * @brief
  * @param None
  * @retval None
  */
void Device::saveEnergy()
{
    m_EnergyStore.writeData(&m_energy);
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
    float drawn_mah = iBat * delta_time * AS_TO_MAH;
    m_energy.cBat += drawn_mah; //in mAh
    m_energy.cBatMod += fabs(drawn_mah); //accumulated capacity module, includes sum of both consumed and charged capacity

    float vHeatLosses = iBat * m_info.resBat; //Voltage losses on the internal battery resistance
    m_energy.eBat += 0.001 * drawn_mah * (vBat + vHeatLosses); //in Wh

    //The life cycle counter should be incremented if the accumulated capacity module two times greater than the real battery capacity
    if (m_energy.cBatMod >= m_config.cInitial * 2.0)
    {
        m_energy.cBatMod = 0; //should be reset to start a new life cycle
        ++m_energy.lifeCycles;
    }

    //Calculate the rest power in the battery taking to account current temperature and power that was consumed
    //!!! This calculation takes approximately 650us and can be done preliminary in the beginning for whole
    //temperature range that we need with requested temperature resolution.
    //Suggest temperature step is 1.0 Celsius
    float tempFactor = getCapTempFactor();
    float lifeFactor = getLifeFactor();
    float k = (tempFactor + lifeFactor - 1.0);

    m_info.cBatRest = (m_config.cInitial * k - m_energy.cBat);
    m_info.eBatRest = (m_config.eInitial * k * 0.001 - m_energy.eBat);

    if (is_negative(m_info.cBatRest))
    {
        m_info.cBatRest = 0;
    }

    if (is_negative(m_info.eBatRest))
    {
        m_info.eBatRest = 0;
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
    /*if (!is_positive(iBat))
    {
        return;
    }*/

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
  * @brief This method returns relative dependence of the battery capacity on the
  *        measured  battery temperature -  from 0.0 to 1.0,
  * @param
  * @retval float: factor value from 0.0 to 1.0
  */
float Device::getCapTempFactor()
{
    return Newton.GetVal(tBat_filt);
}

/**
  * @brief This method returns relative dependence of the battery capacity on the
  *        life cycles amount - from 0.0 to 1.0,
  * @param
  * @retval float: factor value from 0.0 to 1.0
  */
float Device::getLifeFactor()
{
    return Linear.GetValOnInterval(m_energy.lifeCycles);
}
