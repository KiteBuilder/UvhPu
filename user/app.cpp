#include "app.h"
#include "iocan.h"
#include "Device.h"

#include <b57861s103.h>

extern CAN_HandleTypeDef hcan;

extern SDADC_HandleTypeDef hsdadc1;
extern SDADC_HandleTypeDef hsdadc3;

extern TIM_HandleTypeDef htim5;

io_can can = {&hcan};//new io_can(&hcan);
Device dev;

adc_t adcIBat 	= {&hsdadc1, 5};	//SDADC_CHANNEL_5 	PB1 CS P (AD8418)
adc_t adcVBat 	= {&hsdadc3, 6};	//SDADC_CHANNEL_6  	PD8
adc_t adcTemp   = {&hsdadc1, 6};	//SDADC_CHANNEL_6	PB0
adc_t adcVLoad 	= {&hsdadc3, 7};	//SDADC_CHANNEL_7
adc_t adcIMon 	= {&hsdadc1, 4};	//SDADC_CHANNEL_4	reserve current

void initialization()
{
	dev.info().flags.wakeUpHOLD = HOLD_READ();		//from autopilot
	dev.info().flags.wakeUpPB = PB_EN_SIG_READ();	//from button
	dev.info().eBat = 0;
	dev.info().cBat = 0;

	ADC_EN(GPIO_PIN_SET);
	HAL_Delay(100);

	if(dev.checkVbatVload()){
		BAT_CONNECT(GPIO_PIN_SET);

		//выполнять это только когда включаем по кнопке?
		if(!(dev.info().iBatOffset >= dev.config().iBatOffset - 5 &&
		   dev.info().iBatOffset <= dev.config().iBatOffset + 5)){
			dev.config().iBatOffset = dev.info().iBatOffset;
			//dev.saveConfig();
		}
	}
	else dev.info().flags.faultVbatVLoad = true;
}

void exec()
{
	dev.info().flags.faultLTC = FAULT_READ();
	dev.info().flags.wakeUpHOLD = HOLD_READ();
	dev.info().flags.wakeUpPB = PB_EN_SIG_READ();
	dev.info().flags.mcu = BAT_INP_READ();
	dev.info().flags.pbINT = PB_INT_READ();
	dev.info().flags.heater = false;

	LED_CHARGE_SET(dev.info().fullBat ? GPIO_PIN_SET : GPIO_PIN_RESET);
	LED_PWR_SET(dev.info().flags.mcu ? GPIO_PIN_SET : GPIO_PIN_RESET);
	LED_HOLD_SET(dev.info().flags.wakeUpHOLD ? GPIO_PIN_SET : GPIO_PIN_RESET);
	LED_HEATER_SET(dev.info().flags.heater ? GPIO_PIN_SET : GPIO_PIN_RESET);

	if(!dev.info().flags.faultVbatVLoad){
		dev.convertAdcData();
		HAL_Delay(1000 / dev.config().fTelemetry);

		memset(dev.txData(), 0, CAN_PACK_SIZE);
		Protocol::addSFloat(dev.txData(0), dev.info().vBat.val);
		Protocol::addFloat(dev.txData(2), dev.info().iBat.val);
		Protocol::add2Bytes(dev.txData(6), (int16_t)(dev.info().iMon.val * 100));
		can.send(dev.config().id + Command::Pack1, dev.txData(), CAN_PACK_SIZE);

		memset(dev.txData(), 0, CAN_PACK_SIZE);
		Protocol::addSFloat(dev.txData(0), dev.info().vOut.val);
		Protocol::addSFloat(dev.txData(2), dev.info().tempBat.val);
		Protocol::add2Bytes(dev.txData(4), dev.info().pBat);
		Protocol::addFlags(dev.txData(6), dev.info().flags);
		can.send(dev.config().id + Command::Pack2, dev.txData(), CAN_PACK_SIZE);

		memset(dev.txData(), 0, CAN_PACK_SIZE);
		Protocol::addFloat(dev.txData(0), dev.info().cBat);
		Protocol::addFloat(dev.txData(3), dev.info().eBat);
		can.send(dev.config().id + Command::Pack3, dev.txData(), CAN_PACK_SIZE);
	}
	else{
		LED_PWR_SET(GPIO_PIN_SET);
		HAL_Delay(500);
		LED_PWR_SET(GPIO_PIN_RESET);
		HAL_Delay(500);

		memset(dev.txData(), 0, CAN_PACK_SIZE);
		Protocol::addFlags(dev.txData(6), dev.info().flags);
		can.send(dev.config().id + Command::Pack2, dev.txData(), CAN_PACK_SIZE);
	}
}

//----------------------Callback--------------------------

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	uint32_t cmd;
	uint8_t size = 0;

	memset(dev.rxData(), 0, CAN_PACK_SIZE);

	if(!can.read(cmd, dev.rxData(), size) || size != CAN_PACK_SIZE || !dev.info().flags.wakeUpPB)
		return;

	cmd -= dev.config().id;

	if(cmd == Command::ConfSave)
		dev.saveConfig();
	else
		Protocol::parceData((Command)cmd, dev.rxData(), dev.config());
}

void HAL_SDADC_InjectedConvCpltCallback(SDADC_HandleTypeDef* hsdadc)
{
	uint32_t channel;
	uint32_t val;

	if(hsdadc == &hsdadc1){
		val = HAL_SDADC_InjectedGetValue(hsdadc, &channel);
		if(channel == adcIBat.channel)
			Converter::addToAverage(dev.info().iBat, val);
		else if(channel == adcIMon.channel)
			Converter::addToAverage(dev.info().iMon, val);
		else if(channel == adcTemp.channel)
			Converter::addToAverage(dev.info().tempBat, val);
	}
	else if(hsdadc == &hsdadc3){
		val = HAL_SDADC_InjectedGetValue(hsdadc, &channel);
		if(channel == adcVLoad.channel)
			Converter::addToAverage(dev.info().vOut, val);
		else if(channel == adcVBat.channel)
			Converter::addToAverage(dev.info().vBat, val);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	//если  PB_EN_SIG == 0 тоже выключать питание???

	if(GPIO_Pin == PIN_PB_INT)
	{
		//power off
		if(PB_INT_READ() == GPIO_PIN_RESET && !dev.info().flags.wakeUpHOLD){
			ADC_EN(GPIO_PIN_RESET);
			BAT_CONNECT(GPIO_PIN_RESET);
			PB_KILL(GPIO_PIN_SET);
		}
	}
}

// 1 second
void onTim3Triggered()
{
	if(dev.info().vBat.val >= dev.config().vMax && dev.info().iBat.val < dev.config().iFullCharge)
	{
		if(!dev.info().fullBat && dev.info().fullBatCnt++ > 10){
			dev.info().fullBat = true;
			dev.info().fullBatCnt = 0;
		}
	}
	else{
		dev.info().fullBat = false;
		dev.info().fullBatCnt = 0;
	}

	if(!dev.info().flags.faultVbatVLoad)
		dev.calculateBatCE();
}

