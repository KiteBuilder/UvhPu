#include <Device.h>
#include <Converter.h>
#include <string.h>
#include "b57861s103.h"

Device::Device()
{
	m_config = m_flash.getConfig();
}

Device::~Device()
{

}

void Device::saveConfig()
{
	m_flash.setConfig(m_config);
}

void Device::convertAdcData()
{
	float iBat = Converter::generateAverageVar(m_info.iBat) - m_config.iBatOffset;
	float iMon = Converter::generateAverageVar(m_info.iMon);
	float vBat = Converter::generateAverageVar(m_info.vBat);
	float vOut = Converter::generateAverageVar(m_info.vOut);
	float tempBat = Converter::generateAverageVar(m_info.tempBat);

	iBat = Converter::adcToVoltage(iBat, vRef, 0.25);	//0.25 for sigmadelta ADC to 16384 full range
	iMon = Converter::adcToVoltage(iMon, vRef, 0.5);	//0.5  for sigmadelta ADC to 32768 full range
	vBat = Converter::adcToVoltage(vBat, vRef, 0.5);
	vOut = Converter::adcToVoltage(vOut, vRef, 0.5);
	tempBat = Converter::adcToVoltage(tempBat, vRef, 0.5);

	iBat = (iBat - vBias) / (20 * R_SHUNT(m_config.shunt));
	iMon = iMon / (20 * R_SHUNT(m_config.shunt));

	m_info.iBat.val = m_config.iBatK * iBat + m_config.iBatB;
	m_info.iMon.val = m_config.iMonK * iMon + m_config.iMonB;

	m_info.vBat.val = m_config.vBatK * Converter::toDividerVoltage(vBat, 60400, 3600) + m_config.vBatB;
	m_info.vOut.val = m_config.vOutK * Converter::toDividerVoltage(vOut, 60400, 3600) + m_config.vOutB;

	m_info.tempBat.val = m_config.tempBatK * B57861s103::toTemp(2490, tempBat, vRef) + m_config.tempBatB;
	m_info.pBat = m_info.iBat.val * m_info.vBat.val;

	//расчёт cBat eBat через интервал
}

bool Device::checkVbatVload()
{
	float vBat = Converter::generateAverageVar(m_info.vBat);
	float vOut = Converter::generateAverageVar(m_info.vOut);
	float iBatOffset = Converter::generateAverageVar(m_info.iBat);
	bool res = true;

	vOut = Converter::adcToVoltage(vOut, vRef, 0.5);
	vBat = Converter::adcToVoltage(vBat, vRef, 0.5);

	m_info.vBat.val = Converter::toDividerVoltage(vBat, 60400, 3600);
	m_info.vOut.val = Converter::toDividerVoltage(vOut, 60400, 3600);

	res = m_info.vBat.val > m_info.vOut.val;

	if(res){
		m_info.iBatOffset = iBatOffset;
		m_info.iBat.clear();
	}

	return res;
}

void Device::calculateBatCE()
{
	m_info.eBat += m_info.iBat.val * m_info.vBat.val / 3600.0;
	m_info.cBat += m_info.iBat.val / 3600.0;

	if(m_info.eBat > m_config.eInitial)
		m_info.eBat = m_config.eInitial;

	if(m_info.cBat > m_config.cInitial)
		m_info.cBat = m_config.cInitial;
}

