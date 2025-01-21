/**
  ******************************************************************************
  * @file		: btn_config.c
  * @author  	: AW		Adrian.Wojcik@put.poznan.pl
  * @version 	: 1.3.0
  * @date    	: Oct 19, 2022
  * @brief   	: Button & switch components configuration file
  *
  ******************************************************************************
  */

/* Private includes ----------------------------------------------------------*/
#include "btn.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Public variables ----------------------------------------------------------*/
BTN_DIO_Handle_TypeDef husrbtn = BTN_DIO_INIT_HANDLE(USER_Btn, BTN_PULL_DOWN);
BTN_DIO_Handle_TypeDef EXT_BTN1 = BTN_DIO_INIT_HANDLE(EXT_BTN1, BTN_PULL_DOWN);
BTN_DIO_Handle_TypeDef EXT_BTN2 = BTN_DIO_INIT_HANDLE(EXT_BTN2, BTN_PULL_DOWN);
/* Private function prototypes -----------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/
