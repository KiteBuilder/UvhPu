#include <Device.h>
#include <Converter.h>
#include <string.h>
#include "types.h"
#include "b57861s103.h"

/**
  * @brief Constructor
  * @param None
  * @retval None
  */
Device::Device()
{
	m_config = m_flash.getConfig();
	m_info.eBat = 0.0;
	m_info.cBat = 0.0;
	m_info.resBat = 0.0;
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
	float iBat = Converter::generateAverageVar(m_info.iBat); //- m_config.iBatOffset;
	float iMon = Converter::generateAverageVar(m_info.iMon);
	float vBat = Converter::generateAverageVar(m_info.vBat);
	float vOut = Converter::generateAverageVar(m_info.vOut);
	float tempBat = Converter::generateAverageVar(m_info.tempBat);

	iBat = Converter::adcToVoltage(iBat, vRef, 0.25);	//0.25 for sigmadelta ADC to 16384 full range
	iMon = Converter::adcToVoltage(iMon, vRef, 0.5);	//0.5  for sigmadelta ADC to 32768 full range
	vBat = Converter::adcToVoltage(vBat, vRef, 0.5);
	vOut = Converter::adcToVoltage(vOut, vRef, 0.5);
	tempBat = Converter::adcToVoltage(tempBat, vRef, 0.5);

	//iBat = (iBat - vBias) / (20 * R_SHUNT(m_config.shunt));
	const float amp_offset = 15.1;
	const float apm_per_volt = 0.4;
 	iBat = ((iBat - vRef) / (20 * R_SHUNT(m_config.shunt)) - amp_offset) * apm_per_volt;
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
    m_info.eBat += 0.001 * drawn_mah * vBat_filt; //in Wh

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
    // update maximum current seen since startup and protect against divide by zero
    iBat_max = (iBat_max > iBat) ? iBat_max : iBat;
    float iBat_delta = iBat - iBat_filt;

    if (is_zero(iBat_delta))
    {
        return;
    }

    // update reference voltage and current
    if (vBat > vBat_ref)
    {
        vBat_ref = vBat;
        iBat_ref = iBat;
    }

    float res_alpha = 0.1 * fabsf(iBat - iBat_filt)/ iBat_max;

    if (res_alpha > 1.0)
    {
        res_alpha = 1.0;
    }

    float resBat_estimate = (vBat_filt - vBat)/iBat_delta;

    if (is_positive(resBat_estimate))
    {
        m_info.resBat = m_info.resBat + res_alpha * (resBat_estimate - m_info.resBat);
    }

    // calculate maximum resistance
    if ((vBat_ref > vBat) && (iBat > iBat_ref))
    {
        float resBat_max = (vBat_ref - vBat) / (iBat - iBat_ref);
        m_info.resBat =  (m_info.resBat < resBat_max) ? m_info.resBat : resBat_max;
    }
}

/**
  * @brief  Update battery parameters task
  * @param curremtTime: current time in microseconds
  * @retval None
  */
void Device::UpdateButtery(timeUs_t currentTimeUs)
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

    //calculate battery impedance and power consumption
    calculateBatRes(delta_time);
    calculateBatConsumption(delta_time);
}
