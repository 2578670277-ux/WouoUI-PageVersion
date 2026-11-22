#include "shell_port.h"
#include "stdio.h"


Shell shell;
char shell_buffer[512];

//log日志的代码，实现日志控制和多色日志
#if SHELL_USING_CMD_LOG

#include "log.h"
#define true 1

void uartLogWrite(char * buffer,short len);
Log uartLog = {
    .write = uartLogWrite,
    .active = true,
    .level = LOG_DEBUG
};
void uartLogWrite(char * buffer,short len)
{
    if(uartLog.shell)
    {
        shellWriteEndLine(uartLog.shell,buffer,len);
    }
}
#endif


//底层代码
static void uart_init(u32 bound)
{
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 
}

int fputc(int /*c*/ch, FILE * /*stream*/f)
{
    while ((USART1->SR & 0X40) == 0); //循环发送,直到发送完毕
    USART1->DR = (u8)ch;
    return ch;
}


// shell接口文件
signed short User_Shell_Write(char * data, unsigned short len)              
{
    int i = 0;
    for (i =0;i<len;i++)
    {
        while ((USART1->SR & 0X40) == 0); //循环发送,直到发送完毕
        USART1->DR = (u8)data[i];
    }
    return i+1;
}

void User_Shell_Init(uint32_t baudrate)
{
    uart_init(baudrate);
    shell.write = User_Shell_Write;
    shellInit(&shell,shell_buffer,512);
#ifdef SHELL_USING_CMD_LOG
     logRegister(&uartLog,&shell);
#endif
}

uint8_t recv_buf = 0;
void USART1_IRQHandler(void) //串口1中断服务程序
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
    {
		recv_buf = USART_ReceiveData(USART1); //读取接收到的数据
        shellHandler(&shell,recv_buf);
    }
}



/*命令导出表*/
//example
//#include "StepMotor.h"

//SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC),
//                StepMotor_stop,StepMotor_stop,\
//                "StepMotor_stop 1/2");





