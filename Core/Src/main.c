/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef struct {
int16_t rocker_l_;//左水�?
int16_t rocker_l1;//左数�?
int16_t rocker_r_;//右水�?
int16_t rocker_r1;//右数�?
int16_t SWA;//�?关A
int16_t SWB;//�?关B
int16_t SWC;//�?关C
int16_t SWD;//�?关D
int16_t VRA;//旋钮A
int16_t VRB;//旋钮B
uint8_t error_protect;//故障保护�?活位
uint8_t byte_lose;//真丢失标志位
uint8_t offline;//超过100ms没收到有效帧就置1


}RC_ctrl_t;

static RC_ctrl_t rc_data;//遥控器数�?

int16_t CH[16];//存放解析后的16个�?�道数据
uint8_t start_byte,flag_byte,stop_byte;//代表状�?�的变量
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;

/* USER CODE BEGIN PV */
volatile uint8_t data_ready_flag = 0;
uint8_t buf[25];//DMA接收
uint8_t buf_copy[25];//临时储存
volatile uint32_t last_frame_tick = 0;//最后一次收到有效帧的时间
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 这个函数可以�? buf 里的乱码�? 11 位一组切出来，并减去 1024
//串口发的数据是八位，但是摇杆的位置需�?11�?
//0x07FF:保留11�?
void BufToChannel(void) {
  CH[0] = ((((int16_t)buf_copy[1] >> 0) | ((int16_t)buf_copy[2] << 8)) & 0x07FF) - 1024;
  CH[1] = ((((int16_t)buf_copy[2] >> 3) | ((int16_t)buf_copy[3] << 5)) & 0x07FF) - 1024;
  CH[2] = ((((int16_t)buf_copy[3] >> 6) | ((int16_t)buf_copy[4] << 2) | ((int16_t)buf_copy[5] << 10)) & 0x07FF) - 1024;
  CH[3] = ((((int16_t)buf_copy[5] >> 1) | ((int16_t)buf_copy[6] << 7)) & 0x07FF) - 1024;
  CH[4] = ((((int16_t)buf_copy[6] >> 4) | ((int16_t)buf_copy[7] << 4)) & 0x07FF) - 1024;
  CH[5] = ((((int16_t)buf_copy[7] >> 7) | ((int16_t)buf_copy[8] << 1) | ((int16_t)buf_copy[9] << 9)) & 0x07FF) - 1024;
  CH[6] = ((((int16_t)buf_copy[9] >> 2) | ((int16_t)buf_copy[10] << 6)) & 0x07FF) - 1024;
  CH[7] = ((((int16_t)buf_copy[10] >> 5) | ((int16_t)buf_copy[11] << 3)) & 0x07FF) - 1024;
  CH[8] = ((((int16_t)buf_copy[12] >> 0) | ((int16_t)buf_copy[13] << 8)) & 0x07FF) - 1024;
  CH[9] = ((((int16_t)buf_copy[13] >> 3) | ((int16_t)buf_copy[14] << 5)) & 0x07FF) - 1024;
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
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  //启动串口DMA空闲中断接收，长�?25
HAL_UARTEx_ReceiveToIdle_DMA(&huart3,buf,25);
//关闭半传输中断（设计漏洞�?
__HAL_DMA_DISABLE_IT(huart3.hdmarx,DMA_IT_HT);
last_frame_tick = HAL_GetTick();//先记一下时间，避免一上电就报离线

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if(data_ready_flag==1){
      data_ready_flag=0;

      start_byte = buf_copy[0]; // 帧头 0x0F
        
     BufToChannel(); // 调用解析函数，把数据塞进 CH 数组
        
    flag_byte = buf_copy[23];
        stop_byte = buf_copy[24];
        
        // 把解析出来的值赋值
    rc_data.rocker_r_ = CH[0]; 
    rc_data.rocker_r1 = CH[1]; 
    rc_data.rocker_l1 = CH[2]; 
    rc_data.rocker_l_ = CH[3]; 
    rc_data.SWA       = CH[4];
    rc_data.SWB       = CH[5];
    rc_data.SWC       = CH[6];
    rc_data.SWD       = CH[7];
   rc_data.VRA       = CH[8];
    rc_data.VRB       = CH[9];
        
        // 状态改变标志位（用位运算抠出来）
        rc_data.error_protect = (flag_byte >> 3) & 0x01;
        rc_data.byte_lose     = (flag_byte >> 2) & 0x01;
    }
    //超过100ms没收到新的，就认为遥控器断开了（要求的100ms处理）
  
    if(HAL_GetTick() - last_frame_tick > 100){//本来用的后面发现错了
      rc_data.offline = 1;
    }else{
      rc_data.offline = 0;
    }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 100000;
  huart3.Init.WordLength = UART_WORDLENGTH_9B;
  huart3.Init.StopBits = UART_STOPBITS_2;
  huart3.Init.Parity = UART_PARITY_EVEN;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart == &huart3) {
      if(Size == 25&&buf[0]==0x0F&&buf[24]==0x00){
        for(int i=0;i<25;i++){
          buf_copy[i] = buf[i];
        }
        data_ready_flag=1;
last_frame_tick = HAL_GetTick();
      }
    
    
        
        // 处理完后，必须再次开启接收，否则只能收一次(第一次直接忘了)
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, buf, 25);
        __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
    }
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart3) {
        //先把接收停掉，把错误标志清干净，不然可能一直卡在错误里
        HAL_UART_AbortReceive(&huart3);
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_FEFLAG(huart);
        __HAL_UART_CLEAR_NEFLAG(huart);
        __HAL_UART_CLEAR_PEFLAG(huart);
        huart->ErrorCode = HAL_UART_ERROR_NONE;

        //重新开启接收，启动失败再试一次
        if (HAL_UARTEx_ReceiveToIdle_DMA(&huart3, buf, 25) != HAL_OK) {
            HAL_UART_AbortReceive(&huart3);
            HAL_UARTEx_ReceiveToIdle_DMA(&huart3, buf, 25);
        }
        __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
    }
    //我真的尽力了，最后还是被ai检查出来这个东西没写，过不了关
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
