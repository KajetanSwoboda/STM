/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "eth.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//_____________________________________________________________________________________________________________________________________________________________
//______________________________________INCLUDY________________________________________________________________________________________________________________
//_____________________________________________________________________________________________________________________________________________________________
#include "pid_controller_config.h"
#include "btn_config.h"
#include "lcd.h"
#include "math.h"
//_____________________________________________________________________________________________________________________________________________________________
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//_____________________________________________________________________________________________________________________________________________________________
//________________________________________ZMIENNE______________________________________________________________________________________________________________
//_____________________________________________________________________________________________________________________________________________________________
int aktualne_obroty = 0;
int zadane_obroty = 1000;
int Duty = 50;
float e = 0;
float e_procent = 0;
_Bool tryb_pracy = 0;
int enc_raw = 0;
int enc_mod = 0;
int wyswietlany_ekran = 0;
int tryb_pracy_python = 0;
uint32_t pomiar_01 = 0;
uint32_t pomiar_02 = 0;
uint32_t roznica = 0;
uint8_t flaga = 0;
uint32_t frequency = 0;
uint8_t buffer[32];
uint16_t dlug_wiad = 5;
//_____________________________________________________________________________________________________________________________________________________________
//____________________________________TACHOMETR________________________________________________________________________________________________________________
//_____________________________________________________________________________________________________________________________________________________________
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
    // Sprawdzenie, czy przerwanie pochodzi od timera 2
    if(htim == &htim2){
        // Włączenie LED jako sygnał zdarzenia
        HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, 1);
        // Sprawdzenie, czy przerwanie pochodzi z kanału 1
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
            // Pierwsze przechwycenie
            if (flaga == 0){
                // Odczytanie pierwszej wartości
                pomiar_01 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                flaga = 1;
            }else{
                // Odczytanie drugiej wartości
                pomiar_02 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                // Obliczenie różnicy
                if (pomiar_02 > pomiar_01){roznica = pomiar_02 - pomiar_01;}
                // Obliczenie częstotliwości
                frequency = HAL_RCC_GetPCLK1Freq() / roznica;
                // Obliczenie obrotów
                aktualne_obroty = frequency * 30;
                // Resetowanie licznika timera
                __HAL_TIM_SET_COUNTER(htim, 0);
                // Resetowanie flagi
                flaga = 0;
            }
        }
    }
}
//_____________________________________________________________________________________________________________________________________________________________
//___________________________STEROWANIE PID, ZMIANA TRYBOW_____________________________________________________________________________________________________
//_____________________________________________________________________________________________________________________________________________________________
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Sprawdzenie, czy przerwanie pochodzi od timera 4
    if(htim == &htim4)
    {
        //____________________________STEROWANIE PID__________________________
        Duty = PID_GetOutput(&hpid1, zadane_obroty, aktualne_obroty); // Obliczanie wyjścia PID
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (uint32_t)(Duty)); // Ustawianie wartości porównania PWM
        e = zadane_obroty - aktualne_obroty; // Obliczanie błędu
        e = fabs(e); // Wartość bezwzględna błędu
        e_procent = (e/zadane_obroty)*100; // Obliczanie procentowego błędu
        //____________________________________________________________________
        //__________________________ZMIANA TRYBU I EKRANU_____________________
        // Zmiana trybu pracy po naciśnięciu przycisku 1
        if(BTN_DIO_EdgeDetected(&EXT_BTN1) == BTN_PRESSED_EDGE){
            if (tryb_pracy == 1)
                tryb_pracy = 0;
            else
                tryb_pracy = 1;
        }
        // Zmiana ekranu po naciśnięciu przycisku 2
        if(BTN_DIO_EdgeDetected(&EXT_BTN2) == BTN_PRESSED_EDGE){
            wyswietlany_ekran = wyswietlany_ekran + 1; // Zwiększenie indeksu ekranu
            if(wyswietlany_ekran == 3) // Jeśli osiągnięto 3 ekrany, wróć do 0
                wyswietlany_ekran = 0;
        }
        //____________________________________________________________________
        //__________________________REGULACJA ENKODER_________________________
        // Odczytanie surowego licznika enkodera
        enc_raw = __HAL_TIM_GET_COUNTER(&htim8);
        // Modyfikacja wartości licznika w zależności od wartości
        if(enc_raw < 100)
            enc_mod = 1000 + enc_raw; // Jeśli mniej niż 100, dodaj 1000
        else
            enc_mod = enc_raw * 10; // W przeciwnym przypadku pomnóż przez 10
        // Ustawienie zadanych obrotów na wartość enkodera, jeśli tryb pracy jest 1
        if(tryb_pracy == 1)
            zadane_obroty = enc_mod;
    }
    //____________________________________________________________________
    //__________________________WYSYLANIE UART____________________________
    // Sprawdzenie, czy przerwanie pochodzi od timera 7
    if(htim == &htim7)
    {
        uint8_t tx_buffer[64];
        // Przygotowanie danych do wysłania w formacie JSON
        int resp_len = sprintf((char*)tx_buffer, "{ \"RPM\":%d, \"RPM_REF\":%d, \"Duty\":%d }\n\r", aktualne_obroty, zadane_obroty, Duty);
        // Wysyłanie danych przez UART
        HAL_UART_Transmit(&huart3, tx_buffer, resp_len, 10);
    }
}
//_____________________________________________________________________________________________________________________________________________________________
//______________________________ODBIOR UART i PYTHOB TRYB______________________________________________________________________________________________________
//_____________________________________________________________________________________________________________________________________________________________
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    // Sprawdzenie, czy przerwanie pochodzi od UART3
    if (huart == &huart3) {
        // Terminator, aby traktować buffer jako string (dodanie końca ciągu)
        buffer[5] = '\0';
        // Odczytanie pierwszych 4 cyfr jako obroty
        sscanf((char*)buffer, "%4d", &zadane_obroty);
        // Odczytanie 5-tego znaku jako tryb
        char mode_char = buffer[4];
        // Ustawienie trybu pracy na podstawie otrzymanego znaku
        tryb_pracy_python = (mode_char == '1') ? 1 : 0;
        // Ponowne uruchomienie odbioru danych przez UART
        HAL_UART_Receive_IT(&huart3, buffer, 5);
        // Przypisanie trybu pracy
        tryb_pracy = tryb_pracy_python;
    }
}
//_____________________________________________________________________________________
//_____________________________________________________________________________________



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_ADC1_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
  //____________________________________________________________________________________________________________________________________________
  //_____________________INICJALIZACJA WSZYSTKIEGO______________________________________________________________________________________________
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_TIM_Base_Start_IT(&htim7);
  HAL_UART_Receive_IT(&huart3, buffer, dlug_wiad);
  HAL_TIM_Encoder_Start(&htim8, TIM_CHANNEL_ALL);
  //____________________________________________________________________________________________________________________________________________
  //_________________________INICJALIZACJA LCD__________________________________________________________________________________________________
  Lcd_PortType ports[] = {D4_GPIO_Port, D5_GPIO_Port, D6_GPIO_Port, D7_GPIO_Port};
  Lcd_PinType pins[] = {D4_Pin, D5_Pin, D6_Pin, D7_Pin};
  Lcd_HandleTypeDef lcd = Lcd_create(ports, pins, RS_GPIO_Port, RS_Pin, E_GPIO_Port, E_Pin, LCD_4_BIT_MODE);
  //____________________________________________________________________________________________________________________________________________
  //____________________________________________________________________________________________________________________________________________

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  //____________________________________________________________________________________________________________________________________________
  //________________________________LCD PRZELACZANIE____________________________________________________________________________________________
	  if(wyswietlany_ekran==0){
		  HAL_Delay(500);
		  Lcd_clear(&lcd);
		  Lcd_cursor(&lcd, 0,1);
		  Lcd_string(&lcd, "RPM: ");
		  Lcd_cursor(&lcd, 0,7);
		  Lcd_int(&lcd, aktualne_obroty);
		  Lcd_cursor(&lcd, 1,1);
		  Lcd_string(&lcd, "REF: ");
		  Lcd_cursor(&lcd, 1,7);
		  Lcd_int(&lcd, zadane_obroty);
	  } else if(wyswietlany_ekran==1){
		  HAL_Delay(500);
		  Lcd_clear(&lcd);
		  Lcd_cursor(&lcd, 0,1);
		  Lcd_string(&lcd, "Tryb: ");
		  Lcd_cursor(&lcd, 0,7);
		  Lcd_int(&lcd, tryb_pracy);
		  Lcd_cursor(&lcd, 1,1);
		  Lcd_string(&lcd, "Duty: ");
		  Lcd_cursor(&lcd, 1,7);
		  Lcd_int(&lcd, Duty);
	  }else if(wyswietlany_ekran==2){
 		  HAL_Delay(500);
 		  Lcd_clear(&lcd);
 		  Lcd_cursor(&lcd, 0,1);
 		  Lcd_string(&lcd, "e: ");
 		  Lcd_cursor(&lcd, 0,7);
 		  Lcd_int(&lcd, e);
 		  Lcd_cursor(&lcd, 1,1);
 		  Lcd_string(&lcd, "e%: ");
 		  Lcd_cursor(&lcd, 1,7);
 		  Lcd_int(&lcd, e_procent);
 	  }




    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
