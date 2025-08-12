/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hagl.h"
#include "font6x9.h"
#include "rgb565.h"
#include <stdio.h>
#include "icd.h"
#include "hagl_hal.h"
#include <wchar.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PATFORM_START_PIXEL 79
#define BALL_START_PIXEL_X 81
#define BALL_START_PIXEL_Y 45
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
struct direction {
	uint16_t valueOX;
	uint16_t valueOY;
	int DirectionOX;
	int DirectionOY;
	int loss;
	int ballSpeed;
};
int threeBestScores[] = {0, 0 ,0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi2)
	{
		lcd_transfer_done();
	}
}

int __io_putchar(int ch)
{
  if (ch == '\n') {
    __io_putchar('\r');
  }

  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);

  return 1;
}
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
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while(1) {
	  // Variables - they need to be in main function so they would be "restarted" together with game
	  wchar_t scoreBuffer[20];
	  wchar_t bestScorebuffer[5];
	  wchar_t secdScorebuffer[5];
	  wchar_t thrdScorebuffer[5];
	  int score = 0;
	  volatile static uint16_t joystick[2];
	  const wchar_t* text1 = L"Hello! Press JoyStick";
	  const wchar_t* text2 = L"to start the game :)";
	  const wchar_t* text3 = L"Game Over. Your score:";
	  const wchar_t* text4;
	  const wchar_t* text5 = L"Press JoyStick to restart";
	  const wchar_t* text6 = L"the game.";
	  const wchar_t* text7 = L"The best 3 scores yet:";
	  const wchar_t* text9 = L"No. 1:";
	  const wchar_t* text10 = L"No. 2:";
	  const wchar_t* text11 = L"No. 3:";
	  uint16_t platformPosition = PATFORM_START_PIXEL;
	  uint16_t ballPositionOX = BALL_START_PIXEL_X;
	  uint16_t ballPositionOY = BALL_START_PIXEL_Y;
	  uint16_t platformSpeed = SPEED_NO_1;
	  uint16_t ballSpeed = SPEED_NO_1;
	  uint16_t basicSpeed = SPEED_NO_1;
	  uint16_t joyStickDirection;
	  char one = '1';
	  char two = '2';
	  char three = '3';
	  const wchar_t* start = L"Start!";
	  struct direction ballDirection;

	  /* Generation of Welcome Screen */
	  lcd_init();
	  // Ramka planszy
	  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)joystick, 2);
	  for (int i = 0; i < 5; i++) {
	    hagl_draw_rounded_rectangle(i, i, 160-i, 127-i, 0, WHITE);
	  }
	  // Wymazanie dolnej części ramki
	  hagl_fill_rectangle(4, 4, 154, 128, BLACK);
	  // Generowanie platformy
	  hagl_fill_rectangle(55, 121, 104, 124, WHITE);
	  // Tekst początkowy
	  hagl_put_text(text1, 18, 20, 0xffff, font6x9);
	  hagl_put_text(text2, 21, 30, 0xffff, font6x9);
	  lcd_copy();

	  /* If user press JoyStick, start the game */
	  while (!joyStickPressed());
	  HAL_Delay(100);
	  while (joyStickPressed());

	  /* Clear starting text */
	  hagl_fill_rectangle(5, 5, 150, 110, BLACK);

	  /* Generate ball and drop it in random direction */
	  hagl_fill_circle(BALL_START_PIXEL_X, BALL_START_PIXEL_Y, 2, WHITE);

	  lcd_copy();

	  // Wybieramy losowy startowy kierunek z jakim zacznie poruszać się piłka
	  ballDirection = chooseRandomDirection(ballPositionOX, ballPositionOY);

	  // Odlicznie
	  hagl_put_char(three, 79, 30, WHITE, font6x9);
	  HAL_Delay(1000);
	  lcd_copy();
	  hagl_put_char(two, 79, 30, WHITE, font6x9);
	  HAL_Delay(1000);
	  lcd_copy();
	  hagl_put_char(one, 79, 30, WHITE, font6x9);
	  HAL_Delay(1000);
	  lcd_copy();
	  hagl_put_text(start, 65, 30, WHITE, font6x9);
	  HAL_Delay(1000);
	  lcd_copy();
	  hagl_put_text(start, 65, 30, BLACK, font6x9);
	  HAL_Delay(100);
	  lcd_copy();

	  while (1)
	  {
		  // Przesyłamy DMA tylko wtedy kiedy zakończyło się przesyłanie poprzednich danych
		  if (lcd_is_busy() == false) {
			  // Pobieramy informacje odnośnie kierunku JoySticka
			  joyStickDirection = joyStickVRX(joystick[0]);
			  // Ruszamy platformą w zależności od położenia JoySticka
			  platformPosition = movePlatform(platformPosition, joyStickDirection, platformSpeed);
			  // Ruszay piłką
			  ballDirection = ballMovement(ballDirection.valueOX, ballDirection.valueOY, ballSpeed, ballDirection.DirectionOX, ballDirection.DirectionOY, platformPosition, ballDirection.loss, joyStickDirection, basicSpeed);
			  // Jeżeli gracz odbije piłkę to zwiększamy ilość zdobytych punktów o 1
			  if ((ballDirection.valueOY + ballSpeed > 118 && ((platformPosition - 25 < ballDirection.valueOX) && (ballDirection.valueOX < platformPosition + 25)))) {
				  score++;
			  }
			  // Jeżeli gracz przegra to wyświetlamy kończowe napisy i czekamy aż gracz zretartuje grę
			  if (ballDirection.valueOX == 0 && ballDirection.valueOY == 0 && ballDirection.DirectionOX == 0 &&ballDirection.DirectionOY == 0 && ballDirection.loss == 0) {
				  hagl_put_text(text3, 5, 20, WHITE, font6x9);
				  hagl_put_text(text4, 138, 20, WHITE, font6x9);
				  hagl_put_text(text5, 5, 30, WHITE, font6x9);
				  lcd_copy();
				  HAL_Delay(100);
				  // Sprawdzamy czy wynik gracza był jednym z 3 najlepszych - jeżeli tak to zapisujemy go w tabeli
				  for (int i = 0; i < 3; i++) {
					  if (score > threeBestScores[0]) {
						  threeBestScores[i+2] = threeBestScores[i+1];
						  threeBestScores[i+1] = threeBestScores[i];
						  threeBestScores[i] = score;
						  break;
					  }
					  else if (score > threeBestScores[1]) {
						  threeBestScores[i+2] = threeBestScores[i+1];
						  threeBestScores[i+1] = score;
						  break;
					  }
					  else if (score > threeBestScores[2]) {
						  threeBestScores[i+2] = score;
						  break;
					  }
					  else {
						  continue;
					  }
				  }
				  hagl_put_text(text3, 5, 20, WHITE, font6x9);
				  hagl_put_text(text4, 138, 20, WHITE, font6x9);
				  hagl_put_text(text5, 5, 30, WHITE, font6x9);
				  hagl_put_text(text6, 5, 40, WHITE, font6x9);
				  hagl_put_text(text7, 5, 50, WHITE, font6x9);
				  hagl_put_text(text9, 5, 60, WHITE, font6x9);
				  hagl_put_text(text10, 5, 70, WHITE, font6x9);
				  hagl_put_text(text11, 5, 80, WHITE, font6x9);
				  swprintf(bestScorebuffer, 20, L"%u", threeBestScores[0]);
				  text9 = bestScorebuffer;
				  hagl_put_text(text9, 42, 60, WHITE, font6x9);
				  swprintf(secdScorebuffer, 20, L"%u", threeBestScores[1]);
				  text10 = secdScorebuffer;
				  hagl_put_text(text10, 42, 70, WHITE, font6x9);
				  swprintf(thrdScorebuffer, 20, L"%u", threeBestScores[2]);
				  text11 = thrdScorebuffer;
				  hagl_put_text(text11, 42, 80, WHITE, font6x9);
				  lcd_copy();
				  HAL_Delay(100);
				  // If user press JoyStick they restart the game
				  while (!joyStickPressed());
				  HAL_Delay(100);
				  while (joyStickPressed());
				  break;
			  }
			  // W zależności od ilości odbić zmieniamy prędkość piłki (forma utrudnienia gry)
			  if (10 < score && score <= 20) {
				  basicSpeed = SPEED_NO_2;
			  }
			  else if (20 < score && score <= 30) {
				  basicSpeed = SPEED_NO_3;
			  }
			  else if (30 < score && score <= 40) {
				  basicSpeed = SPEED_NO_4;
			  }
			  else if (40 < score && score <= 50) {
				  basicSpeed = SPEED_NO_5;
			  }
			  else if (50 < score && score <= 60) {
				  basicSpeed = SPEED_NO_6;
			  }
			  else if (score > 60) {
				  basicSpeed = SPEED_NO_6;
			  }
			  ballSpeed = basicSpeed;
			  // Co każdą kratkę rysujemy obecny wynik
			  swprintf(scoreBuffer, 20, L"%u", score);
			  text4 = scoreBuffer;
			  hagl_put_text(text4, 78, 20, WHITE, font6x9);
			  // Aktualizujemy wartości dot. położenia piłki
			  ballPositionOX = ballDirection.valueOX;
			  ballPositionOY = ballDirection.valueOY;
			  ballSpeed = ballDirection.ballSpeed;
		  }
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
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
#ifdef USE_FULL_ASSERT
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
   * 9
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
