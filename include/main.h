/*--------------------------------------------------------------------------------------
 Demo for RGB panels

 DMD_STM32a example code for STM32F103xxx board
 ------------------------------------------------------------------------------------- */
#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_APP_TEST_MODULE  0
#define CONFIG_APP_CC_SK        1

#define LED_MODULE_INDOOR_P475_HUB08       0
#define LED_MODULE_OUTDOOR_P10_HUB12        1

#define LED_MODULE          LED_MODULE_OUTDOOR_P10_HUB12

#if (LED_MODULE == LED_MODULE_INDOOR_P475_HUB08)
#define LED_ENABLE_RGB_COLOR    1
#endif

#ifdef __cplusplus
}
#endif

#endif // _MAIN_H_