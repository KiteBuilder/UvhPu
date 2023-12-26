#include "app.h"
#include "iocan.h"
#include "Device.h"
#include "time.h"
#include "atomic.h"
#include <b57861s103.h>
#include "Scheduler.h"

extern CAN_HandleTypeDef hcan;
extern SDADC_HandleTypeDef hsdadc1;
extern SDADC_HandleTypeDef hsdadc3;
#ifdef DEBUG_ENABLED
    extern UART_HandleTypeDef huart1;
#endif

io_can can = {&hcan};//new io_can(&hcan);
Device dev;

adc_t adcIBat 	= {&hsdadc1, 5};	//SDADC_CHANNEL_5 	PB1 CS P (AD8418)
adc_t adcVBat 	= {&hsdadc3, 6};	//SDADC_CHANNEL_6  	PD8
adc_t adcTemp   = {&hsdadc1, 6};	//SDADC_CHANNEL_6	PB0
adc_t adcVLoad 	= {&hsdadc3, 7};	//SDADC_CHANNEL_7
adc_t adcIMon 	= {&hsdadc1, 4};	//SDADC_CHANNEL_4	reserve current

void taskSTROBE(timeUs_t);
void taskCAN(timeUs_t);
void taskPOWER(timeUs_t);
void taskFLAGS(timeUs_t);
void taskFAULT(timeUs_t);
void taskFAULT_LED(timeUs_t);
void taskDEBUG(timeUs_t);

//Task array, consists of all scheduled tasks
task_t tasks[TASK_COUNT] = {
        [TASK_STROBE] = {
                .name = "LED_STROBE",
                .taskHandler = taskSTROBE,
                .taskPeriod = TASK_PERIOD_US(1000), //1000 microseconds, 1KHz
                .taskEnabled = false,
        },

        [TASK_CAN] = {
                .name = "CAN",
                .taskHandler = taskCAN,
                .taskPeriod = TASK_PERIOD_HZ(10), //default value 10Hz, depends on dev.config().fTelemetry variable
                .taskEnabled = false,
        },

        [TASK_POWER] = {
                .name = "POWER",
                .taskHandler = taskPOWER,
                .taskPeriod = TASK_PERIOD_MS(20),//20ms, 50Hz
                .taskEnabled = false,
        },

        [TASK_FLAGS] = {
                .name = "FLAGS",
                .taskHandler = taskFLAGS,
                .taskPeriod = TASK_PERIOD_MS(10), //10ms, 100Hz
                .taskEnabled = false,
        },

        [TASK_FAULT] = {
                .name = "FAULT",
                .taskHandler = taskFAULT,
                .taskPeriod = TASK_PERIOD_MS(1000), //1000 milliseconds, 1Hz
                .taskEnabled = false,
        },

        [TASK_FAULT_LED] = {
                .name = "FAULTLED",
                .taskHandler = taskFAULT_LED,
                .taskPeriod = TASK_PERIOD_MS(500), //500 milliseconds, 2Hz
                .taskEnabled = false,
        },
#ifdef DEBUG_ENABLED
        [TASK_DEBUG] = {
                .name = "DEBUG",
                .taskHandler = taskDEBUG,
                .taskPeriod = TASK_PERIOD_HZ(50), //50Hz, 20ms
                .taskEnabled = false,
        }
#endif
};

TasksQueue taskQueue(tasks, TASK_COUNT);

/**
  * @brief
  * @param None
  * @retval None
  */
void initialization()
{

    ATOMIC_BLOCK(NVIC_PRIO_MAX)
    {
        usTicks = SystemCoreClock / 1000000;
    }

	dev.info().flags.wakeUpHOLD = HOLD_READ();		//from the autopilot
	dev.info().flags.wakeUpPB = PB_EN_SIG_READ();	//from the button
	dev.info().eBat = 0;
	dev.info().cBat = 0;
	dev.info().resBat = 0;

	ADC_EN(GPIO_PIN_SET);
	HAL_Delay(100);

	if(dev.checkVbatVload()){
		BAT_CONNECT(GPIO_PIN_SET);

		//выполнять это только когда включаем по кнопке?
		if(!(dev.info().iBatOffset >= dev.config().iBatOffset - 5 && dev.info().iBatOffset <= dev.config().iBatOffset + 5))
		{
			dev.config().iBatOffset = dev.info().iBatOffset;
			//dev.saveConfig();
		}
	}
	else
	{
	    dev.info().flags.faultVbatVLoad = true;
	}

	taskQueue.taskEnable(TASK_STROBE);
    taskQueue.taskEnable(TASK_POWER);
    taskQueue.taskEnable(TASK_FLAGS);
#ifdef DEBUG_ENABLED
    taskQueue.taskEnable(TASK_DEBUG);
#endif

    if (dev.info().flags.faultVbatVLoad == true)
    {
        taskQueue.taskDisable(TASK_CAN);
        taskQueue.taskEnable(TASK_FAULT);
        taskQueue.taskEnable(TASK_FAULT_LED);
    }
    else
    {
        taskQueue.taskEnable(TASK_CAN);
        taskQueue.taskDisable(TASK_FAULT);
        taskQueue.taskDisable(TASK_FAULT_LED);
    }
}

/**
  * @brief
  * @param None
  * @retval None
  */
void exec()
{
    taskQueue.scheduler();
}

//----------------------Callback--------------------------
/**
  * @brief
  * @param None
  * @retval None
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	uint32_t cmd;
	uint8_t size = 0;

	memset(dev.rxData(), 0, CAN_PACK_SIZE);

	if (!can.read(cmd, dev.rxData(), size) || size != CAN_PACK_SIZE || !dev.info().flags.wakeUpPB)
	{
		return;
	}

	cmd -= dev.config().id;

	if (cmd == Command::ConfSave)
	{
		dev.saveConfig();
	}
	else
	{
	    uint8_t fTelemetry = dev.config().fTelemetry;
		Protocol::parceData((Command)cmd, dev.rxData(), dev.config());
		if(fTelemetry != dev.config().fTelemetry)
		{
		    taskQueue.reschedule(TASK_CAN, TASK_PERIOD_HZ(dev.config().fTelemetry));
		}

	}
}

/**
  * @brief
  * @param None
  * @retval None
  */
void HAL_SDADC_InjectedConvCpltCallback(SDADC_HandleTypeDef* hsdadc)
{
	uint32_t channel;
	uint32_t val;

	if(hsdadc == &hsdadc1){
		val = HAL_SDADC_InjectedGetValue(hsdadc, &channel);
		if(channel == adcIBat.channel)
			Converter::addToAverage(dev.info().iBat, (float)val);
		else if(channel == adcIMon.channel)
			Converter::addToAverage(dev.info().iMon, (float)val);
		else if(channel == adcTemp.channel)
			Converter::addToAverage(dev.info().tempBat, (float)val);
	}
	else if(hsdadc == &hsdadc3){
		val = HAL_SDADC_InjectedGetValue(hsdadc, &channel);
		if(channel == adcVLoad.channel)
			Converter::addToAverage(dev.info().vOut, (float)val);
		else if(channel == adcVBat.channel)
			Converter::addToAverage(dev.info().vBat, (float)val);
	}
}

/**
  * @brief
  * @param None
  * @retval None
  */
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

/**
  * @brief
  * @param
  * @retval
  */
void taskSTROBE(timeUs_t currentTimeUs)
{
    static uint32_t strbCnt = 0;

    UNUSED(currentTimeUs);
    switch (strbCnt)
    {
        case 0:
            LED_STROBE_SET(GPIO_PIN_SET);
            break;

        case 50:
            LED_STROBE_SET(GPIO_PIN_RESET);
            break;

        case 1000:
            strbCnt = 0;
            return;
    }

    ++strbCnt;
}

/**
  * @brief
  * @param
  * @retval
  */
void taskCAN(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    if(!dev.info().flags.faultVbatVLoad)
    {
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

        memset(dev.txData(), 0, CAN_PACK_SIZE);
        Protocol::addFloat(dev.txData(0), dev.info().resBat);
        Protocol::addFloat(dev.txData(3), dev.info().cBat);
        can.send(dev.config().id + Command::Pack3, dev.txData(), CAN_PACK_SIZE);
    }
}

/**
  * @brief
  * @param
  * @retval
  */
void taskPOWER(timeUs_t currentTimeUs)
{
    if (!dev.info().flags.faultVbatVLoad)
    {
        dev.convertAdcData();
        dev.UpdateBattery(currentTimeUs);
    }


    if( dev.info().vBat.val >= dev.config().vMax && dev.info().iBat.val < dev.config().iFullCharge)
    {
        if (!dev.info().fullBat && dev.info().fullBatCnt++ > 10){
            dev.info().fullBat = true;
            dev.info().fullBatCnt = 0;
        }
    }
    else {
        dev.info().fullBat = false;
        dev.info().fullBatCnt = 0;
    }
}

/**
  * @brief
  * @param
  * @retval
  */
void taskFLAGS(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);
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
}

/**
  * @brief
  * @param
  * @retval
  */
void taskFAULT(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    if(dev.info().flags.faultVbatVLoad)
    {
        memset(dev.txData(), 0, CAN_PACK_SIZE);
        Protocol::addFlags(dev.txData(6), dev.info().flags);
        can.send(dev.config().id + Command::Pack2, dev.txData(), CAN_PACK_SIZE);
    }
}

/**
  * @brief
  * @param
  * @retval
  */
void taskFAULT_LED(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    if(dev.info().flags.faultVbatVLoad)
    {
        HAL_GPIO_TogglePin(GPIO_LED_PWR, PIN_LED_PWR);
    }

}

#ifdef DEBUG_ENABLED
//*****************************************************************************
//Here part for sending debug info with the UART
//*****************************************************************************
#define DLE 0x10
#define ETX 0x03
#define ID 0x01

#define DEBUG_PACK_SIZE 64
uint8_t debugBuff[DEBUG_PACK_SIZE];

#define PACK_SIZE 5

#pragma pack(push, 1)
typedef union
{
    float flt;
    uint8_t bt[sizeof(float)];
} byte_float_t;
#pragma pack(pop)

byte_float_t debugPack[PACK_SIZE];

bool f_TxReady = true;

/**
  * @brief Debug task sends selected amount of values by the serial interface
  *        and uses simple protocol that consists of next fields:
  *        |DLE|ID|DATA|DLE|ETX| where DLE symbol in the DATA field
  *        should be doubled
  * @param
  * @retval
  */
void taskDEBUG(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    debugPack[0].flt = dev.info().iBat.val;
    debugPack[1].flt = dev.info().vBat.val;
    debugPack[2].flt = dev.info().cBat;
    debugPack[3].flt = dev.info().eBat;
    debugPack[4].flt = dev.info().resBat;

    uint32_t n = 0;
    debugBuff[n++] = DLE;
    debugBuff[n++] = ID;

    for (uint32_t i = 0; i < PACK_SIZE; i++)
    {
        for (uint32_t j = 0; j < sizeof(byte_float_t); j++)
        {
           debugBuff[n++] = debugPack[i].bt[j];

           if (debugPack[i].bt[j] == DLE)
           {
               debugBuff[n++] = DLE;
           }
        }
    }

    debugBuff[n++] = DLE;
    debugBuff[n++] = ETX;

    do
    {
        continue;
    }while(!f_TxReady);

    f_TxReady = false;
    HAL_UART_Transmit_IT(&huart1, debugBuff, n);
}

/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    f_TxReady = true;
}

#endif
