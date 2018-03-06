
#include "uart.h"


#define 	PIC_CLK 		    16000000                     // 时钟频率
#define     BAUD                19200                        //波特率
#define     SPBRGx_VAL          ((PIC_CLK/(16UL * BAUD) -1))


#define RX_PIN  TRISC7  //定义数据通讯端口
#define TX_PIN  TRISC6


/*****************************************************************************
 函 数 名  : usart1_init
 功能描述  : USART1初始化，波特率9600
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年5月13日
 作    者  : SJY
 修改内容  : 新生成函数

*****************************************************************************/

void Init_UART1(void)
{
    RX_PIN = 1;
    TX_PIN = 1;
	//配置发送寄存器
    TX1STAbits.TX9D=0;      //无奇偶校验位
    TX1STAbits.TRMT=0;      //发送移位寄存器状态位  0-TSR 已满
    TX1STAbits.BRGH = 1;
    TX1STAbits.SYNC=0;
    TX1STAbits.TXEN=1;
    TX1STAbits.TX9=0;       //发送8位
	//配置接收寄存器
    RC1STAbits.RX9D=0;      //无奇偶校验位
    RC1STAbits.OERR=0;
    RC1STAbits.FERR=0;
    RC1STAbits.ADDEN=0;
    RC1STAbits.CREN=1;      //连续接收使能位
    // RCSTA2bits.SREN=0;
    RC1STAbits.RX9=0;       //接收8位
    RC1STAbits.SPEN=1;      //串口使能位

    SPBRG = SPBRGx_VAL;      //波特率对应初值
    RCIE = 1;                //USART1 接收中断允许位
	TXIE = 0;
	RCIF = 0;
}
/*****************************************************************************
 函 数 名  : usart1_send_byte
 功能描述  : USART发送单个字节
 输入参数  : char ch要发送的字符
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年5月13日
 作    者  : SJY
 修改内容  : 新生成函数

*****************************************************************************/

void uart_send_byte(char dat)
{
    while(!TX1STAbits.TRMT)		//TRMT=0:正在发送，TRMT=1:发送已完成
	{
		continue;
	}
	TXREG=dat;
}
/*****************************************************************************
 函 数 名  : uart_send_str
 功能描述  : 发送字符函数
 输入参数  : uint8 *s
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年5月27日 星期六
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void uart_send_str( uint8 *s)
{
    while(*s!='\0')   // \0表示字符串结束标志，通过检测是否字符串末尾
    {
       uart_send_byte(*s++);
    }
}

/*****************************************************************************
 函 数 名  : send_dat
 功能描述  : 串口发送一串数据
 输入参数  : void *p     --指向串口数据结构体的指针
             uint8 len   --数据长度
             uint8 cnt   --重复次数
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年6月30日 星期五
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/

void send_dat(void *p,uint8 len)
{
    uint8 *temp =p;
    M485_EN_H;
    delay_us(500);
    for(uint8 j=0;j<len;j++)
    {
        uart_send_byte(*temp);
        if(j<len-1) //j<32
        {
            temp++;
        }
    }
    delay_ms(5);
    M485_EN_L;
}

