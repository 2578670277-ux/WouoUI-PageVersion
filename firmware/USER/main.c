/************************include************************************/
#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"
#include "main.h"
#include "led.h"
#include "Shell.h"
#include "shell_port.h"
#include "log.h"
//#include "pack.h"
#include "USART23.h"
#include "oled_port.h" //包含oled驱动文件
#include "WouoUI.h" //包含主要的wouoUI头文件
#include "WouoUI_user.h" //包含用户例子文件
#include "math.h" //引入数学计算sin cos

/************************macro************************************/

//#undef  LOG_ENABLE
//#define LOG_ENABLE  0


/************************variants************************************/
uint16_t fps = 0;
uint16_t sinA = 3, sinB = 0;
float phase1 = 0;
float phase2 = 3.14/2;

/************************function declaration************************************/


int main(void)
{
    SysTick_Config(SystemCoreClock/1000);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE); 
    //使能复用时钟，是为了能够禁用JTAG
    //禁用JTAG接口，只使能SWD,让PB4，PA15，PB3成为普通IO
    // User_Shell_Init(115200); //LetterShell我通常使用来打印log信息调试
    LED_Init();
    USART2_Init(115200); 
    OLED_Init(); //初始化oled
    WouoUI_SelectDefaultUI(); //选择默认UI
    WouoUI_AttachSendBuffFun(OLED_SendBuff); //绑定刷屏函数
    TestUI_Init(); //用户UI的初始化

    while(1)
    {
        if(USARTx_BUFF_AVAIBLE(USART2_RX_STA))
        {
            switch (USART2_RX_BUF[0]) //没有按键，使用串口代替
            {
                case '2': WOUOUI_MSG_QUE_SEND(msg_down); break;
                case '4': WOUOUI_MSG_QUE_SEND(msg_left); break;
                case '5': WOUOUI_MSG_QUE_SEND(msg_click); break;
                case '6': WOUOUI_MSG_QUE_SEND(msg_right); break;
                case '8': WOUOUI_MSG_QUE_SEND(msg_up); break;
                case '9': WOUOUI_MSG_QUE_SEND(msg_return); break;
                default: break;
            }
            if(0x0a == USART2_RX_BUF[0]){  
                //这属于是个补丁了，大学时照着原子写的这个串口接收有点问题(不过没关系，不是这个工程的重点)
                // 没时间细看是啥问题，使用软件动态刷新时才会出现，也是比较奇怪的一点😂之后有时间再回来看下吧
                switch (USART2_RX_BUF[1]) //没有按键，使用串口代替
                {
                    case '2': WOUOUI_MSG_QUE_SEND(msg_down); break;
                    case '4': WOUOUI_MSG_QUE_SEND(msg_left); break;
                    case '5': WOUOUI_MSG_QUE_SEND(msg_click); break;
                    case '6': WOUOUI_MSG_QUE_SEND(msg_right); break;
                    case '8': WOUOUI_MSG_QUE_SEND(msg_up); break;
                    case '9': WOUOUI_MSG_QUE_SEND(msg_return); break;
                    default: break;
                }
            }
            USARTx_BUFF_RESET(&USART2_RX_STA);
        }
        WouoUI_Proc(5); 
        if(task_flag[task_10ms_index]) //sin更新
        {    
            phase1 += 0.1;
            if (phase1 > 6.28)
                phase1 = 0;
            WouoUI_WavePageUpdateVal(&wave_page, 0, sinA * sin(phase1));
            task_flag[task_10ms_index] = 0;
        }
        if(task_flag[task_100ms_index]) //cos更新
        {    
            phase2 += 0.1;
            if (phase2 > 6.28)
                phase2 = 0;
            WouoUI_WavePageUpdateVal(&wave_page, 1, sinB * sin(phase2));
            task_flag[task_100ms_index] = 0;
        }
        if(task_flag[task_500ms_index]) //心跳灯
        {    
            LED_TOGGLE;
            task_flag[task_500ms_index] = 0;
        }
        
    }
}















