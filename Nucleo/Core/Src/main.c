/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#include "can.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "BMP280.h"
#include <stdio.h>
#include "shell.h"
//#include "shell.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
h_BMP280_t h_BMP280;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int K=1000; //Coefficient de correction de la pression
CAN_TxHeaderTypeDef pHeader; //Header de la trame CAN
uint32_t pTxMailbox; //Mailbox de transmission
uint8_t aData[2];  //Data to send
char BMP280_test=0; //Test du BMP280
extern DMA_HandleTypeDef hdma_uart4_rx; 
uint8_t RxChar;
uint8_t RxBuffer[RX_BUFFER_SIZE];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch) {                                           //redirige la fonction printf
	HAL_UART_Transmit(&huart4, (uint8_t *)&ch, 1, HAL_MAX_DELAY);	 //vers l'uart2 ( pour la connexion serial avec l'ordinateur)
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);	 //uart4 pour le rasppberry Pi zero

	return ch;
}

int I2C_transmit(uint8_t address, uint8_t *p_data, uint16_t size)
{
	if(HAL_I2C_Master_Transmit(&hi2c1, address, p_data, size, 100) != HAL_OK){
		printf("Error transmit");
	}
	return 0;
}

int I2C_receive(uint8_t address, uint8_t *p_data, uint16_t size)
{

	if(HAL_I2C_Master_Receive(&hi2c1, address, p_data, size, 100) != HAL_OK)
	{
		printf("Error receive");
	};
	return 0;
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
	MX_USART2_UART_Init();
	MX_I2C1_Init();
	MX_CAN1_Init();
	MX_UART4_Init();
	/* USER CODE BEGIN 2 */

	//CAN
	HAL_CAN_Start(&hcan1);

	//Définir le sens de rotation du moteur
	aData[0]=45;	//Fonctionnement de base avec un angle de 45°
	aData[1]=0x00;	// Tourne dans le sens positif
	pHeader.StdId= 0x61;  //Contrôle du moteur en mode "commande angulaire"
	pHeader.IDE=CAN_ID_STD; //Format de la trame
	pHeader.RTR=CAN_RTR_DATA;  //Trame de données
	pHeader.DLC=2;        //nombre de paquet en transmission
	pHeader.TransmitGlobalTime=DISABLE; //Désactive le timestamp

	HAL_UARTEx_ReceiveToIdle_DMA(&huart4, RxBuffer, RX_BUFFER_SIZE); //Initialisation de la réception UART4
	__HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT); //Désactive l'interruption de la réception UART4

	//BMP280 init et étalonnage du capteur de pression et de température (voir BMP280.c)	
	h_BMP280.I2C_drv.receive = I2C_receive; //Initialisation de la structure de la fonction de réception I2C
	h_BMP280.I2C_drv.transmit = I2C_transmit; //Initialisation de la structure de la fonction de transmission I2C
	BMP280_init(&h_BMP280); //Initialisation du BMP280 (check d'id ect ....)
	printf("\r\nChip ID = %x\r\n", h_BMP280.chip);
	printf("\r\npower mode = %x\r\n", h_BMP280.power);
	printf("\r\nsample = %x\r\n", h_BMP280.sample);
	/* USER CODE END 2 */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		aData[1]=(aData[1]+1)%2; //Le moteur tourne entre 0 et 180° le modulo sert à inverser le sens de de rotation pour rester dans cette tranche
		if (HAL_CAN_AddTxMessage(&hcan1, &pHeader,aData,&pTxMailbox) != HAL_OK){
			printf("error !");
		}
		HAL_Delay(50);
		//Shell_Loop();
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
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 80;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
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
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart->Instance == UART4)
	{
		RxBuffer[Size]='\r';				//Gestion du retour charriot
		RxBuffer[Size+1]='\n';
		protocol(RxBuffer,Size+2);			//Gestion des commandes
		HAL_UARTEx_ReceiveToIdle_DMA(&huart4, RxBuffer, RX_BUFFER_SIZE);
		__HAL_DMA_DISABLE_IT(&hdma_uart4_rx, DMA_IT_HT);
	}
}

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
