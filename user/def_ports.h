#ifndef DEF_PORTS_H_
#define DEF_PORTS_H_

#define GPIO_ADC_EN				GPIOA
#define PIN_ADC_EN				GPIO_PIN_8
#define ADC_EN(flag)			HAL_GPIO_WritePin(GPIO_ADC_EN, PIN_ADC_EN, flag)

//--------------------------------------------------------------------------------
//ltc7003
#define GPIO_BAT_FAULT			GPIOA
#define PIN_BAT_FAULT			GPIO_PIN_11
#define BAT_FAULT_READ()		HAL_GPIO_ReadPin(GPIO_BAT_FAULT, PIN_BAT_FAULT)

#define GPIO_BAT_INP			GPIOA
#define PIN_BAT_INP				GPIO_PIN_12
#define BAT_CONNECT(flag)		HAL_GPIO_WritePin(GPIO_BAT_INP, PIN_BAT_INP, flag)
#define BAT_INP_READ()			HAL_GPIO_ReadPin(GPIO_BAT_INP, PIN_BAT_INP)

#define GPIO_HOLD				GPIOF
#define PIN_HOLD				GPIO_PIN_1
#define HOLD_READ()				HAL_GPIO_ReadPin(GPIO_HOLD, PIN_HOLD)

//--------------------------------------------------------------------------------
//push button controller
#define GPIO_PB_ON				GPIOC
#define PIN_PB_ON				GPIO_PIN_13
#define PB_ON(flag)				HAL_GPIO_WritePin(GPIO_PB_ON, PIN_PB_ON, flag)

#define GPIO_PB_KILL			GPIOF
#define PIN_PB_KILL				GPIO_PIN_6
#define PB_KILL(flag)			HAL_GPIO_WritePin(GPIO_PB_KILL, PIN_PB_KILL, flag)

#define GPIO_PB_INT				GPIOA
#define PIN_PB_INT				GPIO_PIN_15
#define PB_INT_READ()			HAL_GPIO_ReadPin(GPIO_PB_INT, PIN_PB_INT)

#define GPIO_PB_EN_SIG			GPIOF
#define PIN_PB_EN_SIG			GPIO_PIN_7
#define PB_EN_SIG_READ()		HAL_GPIO_ReadPin(GPIO_PB_EN_SIG, PIN_PB_EN_SIG)

//--------------------------------------------------------------------------------
//ltc
#define GPIO_PB_EN				GPIOA
#define PIN_PB_EN				GPIO_PIN_11
#define FAULT_READ()			HAL_GPIO_ReadPin(GPIO_PB_EN, PIN_PB_EN)

//--------------------------------------------------------------------------------
//LEDs
#define GPIO_LED_CHARGE				GPIOA
#define PIN_LED_CHARGE				GPIO_PIN_2
#define LED_CHARGE_SET(flag)	 	HAL_GPIO_WritePin(GPIO_LED_CHARGE, PIN_LED_CHARGE, flag)

#define GPIO_LED_PWR				GPIOB
#define PIN_LED_PWR					GPIO_PIN_3
#define LED_PWR_SET(flag)	 		HAL_GPIO_WritePin(GPIO_LED_PWR, PIN_LED_PWR, flag)

#define GPIO_LED_HOLD				GPIOB
#define PIN_LED_HOLD				GPIO_PIN_4
#define LED_HOLD_SET(flag)	 		HAL_GPIO_WritePin(GPIO_LED_HOLD, PIN_LED_HOLD, flag)

#define GPIO_LED_HEATER				GPIOB
#define PIN_LED_HEATER				GPIO_PIN_5
#define LED_HEATER_SET(flag)	 	HAL_GPIO_WritePin(GPIO_LED_HEATER, PIN_LED_HEATER, flag)

//--------------------------------------------------------------------------------

#endif /* DEF_PORTS_H_ */
