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
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_customhid.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/**Full key conflict free descriptor:
 *-----------------------------------------------------------------------------
 * Descriptor length:73Bytes
 * Send Bytes Num:15Bytes
 * buffer[0] - bit0: Left CTRL
 * 			 bit1: Left SHIFT
 * 			 bit2: Left ALT
 * 			 bit3: Left GUI
 * 			 bit4: Right CTRL
 * 			 bit5: Right SHIFT
 * 			 bit6: Right ALT
 * 			 bit7: Right GUl
 * buffer[1] - Padding = Always 0x00
 * buffer[2] - (A & a) ~ (H & h)
 * buffer[3] - (I & i) ~ (P & p)
 * buffer[4] - (Q & q) ~ (X & x)
 * buffer[5] - (Y & y) ~ (Z & z) | 1 ~ 6
 * buffer[6] - 7 ~ 0 | Enter | Esc | Backspace | Tab
 * ** This is the number key 1 ~ 0 in the main keyboard area
 * buffer[7] - Space | - | = | [ | ] | \ | \ | ;
 * buffer[8] - ' | ` | , | . | / | Cap | F1 ~ F2
 * buffer[9] - F3 ~ F10
 * buffer[A] - F11 ~ F12 | PRTSRC | ScrollLock | Pause | Insert | Home | PgUp
 * buffer[B] - Delete | End | PgDn | Right | Left | Down | Up | Lock
 * buffer[C] - / | * | - | + | Enter | 1 ~ 3
 * buffer[D] - 4 ~ 0 | .
 * ** This is the number key 1 ~ 0 in the numeric keypad area
 * buffer[E] - (Keypad 6) ~ (Keyboard Application)
 *-----------------------------------------------------------------------------
 **Following is the mapping from:
 *| [ASCII code characters] to [corresponding keyboard descriptors]
 * _____________________________________________________________________
 *|        From ASCII to Keyboard descriptors Mapping Table             |
 *| 1. The upper four bits are the (buffer index:Ah[from 0 to 15]),     |
 *| 2. the highest bit of the lower four bits is                        |
 *|    whether to use the control key shift(Bh+8h/Bh+0h),               |
 *| 3. and the remaining three bits are the                             |
 *|    (byte numbers:Bh[from 0h to 7h])                                 |
 *|    corresponding to                                                 |
 *|    the descriptor of the ASCII character in the buffer array index  |
 *| 4. map[ASCII] = 0x(A)(B/B+8)                                        |
 *|_____________________________________________________________________|
 //uint8_t map[128]={
 //		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 //		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 //		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 //		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 //		0x70, 0x5a, 0x88, 0x5c, 0x5d, 0x5e, 0x68, 0x80,
 //		0x6a, 0x6b, 0x69, 0x7a, 0x82, 0x71, 0x83, 0x84,
 //		0x63, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x60,
 //		0x61, 0x62, 0x7f, 0x77, 0x8a, 0x72, 0x8b, 0x8c,
 //		0x5b, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
 //		0x2f, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
 //		0x3f, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e,
 //		0x4f, 0x58, 0x59, 0x73, 0x75, 0x74, 0x5f, 0x79,
 //		0x81, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
 //		0x27, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
 //		0x37, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
 //		0x47, 0x50, 0x51, 0x7b, 0x7d, 0x7c, 0x89, 0xb0
 //};
 *-----------------------------------------------------------------------------
 **Following are the descriptor set in Func:
 *                CUSTOM_HID_ReportDesc_FS in usbd_custom_hid_if.c
 //  0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)//73U
 //  0x09, 0x06,                    // USAGE (Keyboard)
 //  0xa1, 0x01,                    // COLLECTION (Application)
 //  0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
 //  0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
 //  0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
 //  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
 //  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
 //  0x95, 0x08,                    //   REPORT_COUNT (8)
 //  0x75, 0x01,                    //   REPORT_SIZE (1)
 //  0x81, 0x02,                    //   INPUT (Data,Var,Abs)
 //  0x95, 0x01,                    //   REPORT_COUNT (1)
 //  0x75, 0x08,                    //   REPORT_SIZE (8)
 //  0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
 //  0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
 //  0x19, 0x04,                    //   USAGE_MINIMUM (Keyboard a and A)
 //  0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
 //  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
 //  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
 //  0x95, 0x62,                    //   REPORT_COUNT (98)
 //  0x75, 0x01,                    //   REPORT_SIZE (1)
 //  0x81, 0x02,                    //   INPUT (Data,Var,Abs)
 //  0x95, 0x01,                    //   REPORT_COUNT (1)
 //  0x75, 0x06,                    //   REPORT_SIZE (6)
 //  0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
 //  0x05, 0x08,                    //   USAGE_PAGE (LEDs)
 //  0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
 //  0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
 //  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
 //  0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
 //  0x95, 0x05,                    //   REPORT_COUNT (5)
 //  0x75, 0x01,                    //   REPORT_SIZE (1)
 //  0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
 //  0x95, 0x01,                    //   REPORT_COUNT (1)
 //  0x75, 0x03,                    //   REPORT_SIZE (3)
 //  0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
 //  0xc0                           // END_COLLECTION
 *----------------------------------------------------------------------------
 **Following are init steps about this Project:
 * 1. usbd_conf.h: //can modify to USER CODE position
 *    #define USBD_CUSTOMHID_OUTREPORT_BUF_SIZE    15U
 *    #define USBD_CUSTOM_HID_REPORT_DESC_SIZE     73U
 * 2. usbd_customhid.h:
 *    #ifndef USBD_CUSTOMHID_OUTREPORT_BUF_SIZE
 *    #define USBD_CUSTOMHID_OUTREPORT_BUF_SIZE  0x0fU //15Bytes
 *    #endif  //USBD_CUSTOMHID_OUTREPORT_BUF_SIZE
 *    #ifndef USBD_CUSTOM_HID_REPORT_DESC_SIZE
 *    #define USBD_CUSTOM_HID_REPORT_DESC_SIZE   73U   //73Bytes
 *    #endif  //USBD_CUSTOM_HID_REPORT_DESC_SIZE
 * 3. usbd_customhid.c:
 *    USB CUSTOM_HID device FS Configuration Descriptor:
 *        Descriptor of CUSTOM HID interface:
 *            0x01,  //nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse
 * 4. usbd_custom_hid_if.c:
 *    Modify your code to USER CODE position
 *----------------------------------------------------------------------------*/
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define StrokeSlot 50
uint8_t buffer[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE];

uint8_t map[128]={
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x70, 0x5a, 0x88, 0x5c, 0x5d, 0x5e, 0x68, 0x80,
		0x6a, 0x6b, 0x69, 0x7a, 0x82, 0x71, 0x83, 0x84,
		0x63, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x60,
		0x61, 0x62, 0x7f, 0x77, 0x8a, 0x72, 0x8b, 0x8c,
		0x5b, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
		0x2f, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
		0x3f, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e,
		0x4f, 0x58, 0x59, 0x73, 0x75, 0x74, 0x5f, 0x79,
		0x81, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
		0x27, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
		0x37, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
		0x47, 0x50, 0x51, 0x7b, 0x7d, 0x7c, 0x89, 0xb0
};


void Get_Descriptor(uint8_t ascii);
void SimulateKeyPress(uint8_t ascii);
void SimulateKeyRelease();
void SimulateKeyStroke(uint8_t ascii);
void SimulateKeyStrokes(char *str, int len);
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
  MX_USART2_UART_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  memset(buffer, 0x00, sizeof(uint8_t)*USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1){
	  HAL_GPIO_EXTI_Callback(Key_Pin);
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void Get_Descriptor(uint8_t ascii){
	memset(buffer, 0x00, sizeof(uint8_t)*USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
	uint8_t pos = map[ascii];
	buffer[(uint8_t)(pos>>4)] |= (1<<((uint8_t)(pos&0x07)));
	if((pos&0x08) == 8)
		buffer[0] |= 0x02;
}

void SimulateKeyPress(uint8_t ascii){
    //get key:ascii Descriptor
    Get_Descriptor(ascii);
    //Sent Descriptor report
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, buffer, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

void SimulateKeyRelease(){
    //set 0
	memset(buffer, 0x00, sizeof(uint8_t)*USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
    //Sent Descriptor report
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, buffer, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

void SimulateKeyStroke(uint8_t ascii){
    SimulateKeyPress(ascii);
    HAL_Delay(StrokeSlot); //Wait StrokeSlot time
    SimulateKeyRelease();
    HAL_Delay(StrokeSlot); //Wait StrokeSlot time
}

void SimulateKeyStrokes(char *str, int len){
    for(int i = 0; i < len; i++){
    	SimulateKeyStroke(str[i]);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	GPIO_PinState key_pin = HAL_GPIO_ReadPin(GPIOA, GPIO_Pin);  // 读取key的状�??
	if(key_pin == GPIO_PIN_SET){
//		buffer[0] = 0x02;
//		buffer[5] = 0x04;
//		USBD_HID_SendReport(&hUsbDeviceFS, buffer, PackageLEN);
//		HAL_Delay(100);
//		buffer[0] = 0x00;
//		buffer[5] = 0x00;
//		USBD_HID_SendReport(&hUsbDeviceFS, buffer, PackageLEN);
		HAL_Delay(500);
		char str[256];
		strcpy(str, "!@#$%^&*()_+1234567890~`{}|:\"<>?[];',./ashdahskdhasjdeuwhuASDJDHJAJKDHBSXAHE");
		SimulateKeyStrokes(str, strlen(str));
//		HAL_GPIO_WritePin(GPIOA, GPIO_Pin, GPIO_PIN_RESET);
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
