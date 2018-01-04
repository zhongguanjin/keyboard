
#include "dbg.h"
/*
char *itoa_my(int value,char *string,int radix)
{
    char zm[37]="0123456789abcdefghijklmnopqrstuvwxyz";
    char aa[100]={0};
    int sum=value;
    char *cp=string;
    int i=0;
    if(radix<2||radix>36)   //�����˶Դ���ļ��
    {
        return string;
    }
    if(value<0)
    {
        return string;
    }
    while(sum>0)
    {
        aa[i++]=zm[sum%radix];
        sum/=radix;
    }

    for(int j=i-1;j>=0;j--)
    {
        *cp++=aa[j];
    }
    *cp='\0';
    return string;
}
*/
/*
* �������� itoa
* ���� ������������ת�����ַ���-�� 18 ҳ-
* ���� �� -radix =10 ��ʾ 10 ���ƣ��������Ϊ 0
* -value Ҫת����������
* -buf ת������ַ���
* -radix = 10
* ��� ����
* ���� ����
*/
static char *itoa(int value, char *string, int radix)
 {
    int i, d;
    int flag = 0;
    char *ptr = string;
    /* This implementation only works for decimal numbers. */
    if (radix != 10){
        *ptr = 0;
        return string;
    }
    if (!value){
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }
    /* if this is a negative value insert the minus sign. */
    if (value < 0){
        *ptr++ = '-';
        /* Make the value positive. */
        value *= -1;
    }
    for (i = 10000; i > 0; i /= 10){
        d = value / i;
        if (d || flag){
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }
    /* Null terminate the string. */
    *ptr = 0;
    return string;
 } /* NCL_Itoa */

/*
    * �������� dbg
    * ���� ����ʽ������������� C ���е� printf��������û���õ� C ��
    * ���� �� -USARTx ����ͨ��������ֻ�õ��˴��� 1���� USART1
    * -Data Ҫ���͵����ڵ����ݵ�ָ��
    * -... ��������
    * ��� ����
    * ���� ����
*/
 void my_dbg(uint8 *Data,...)
 {
    const char *s;
    int d;
    char buf[16];
    va_list ap;
    va_start(ap, Data);
    while ( *Data != 0) // �ж��Ƿ񵽴��ַ���������
    {
        if ( *Data == 0x5c ) //'\'
        {
            switch ( *++Data )
            {
                case 'r': //�س���
                uart_send_byte(0x0d);
                Data ++;
                break;
                case 'n': //���з�
                uart_send_byte(0x0a);
                Data ++;
                break;
                default:
                Data ++;
                break;
            }
        }
    else if ( *Data == '%')
    { //
        switch ( *++Data )
        {
            case 's': //�ַ���
                s = va_arg(ap, const char *);
                for ( ; *s; s++)
                {
                    uart_send_byte(*s);
                }
                Data++;
                break;
            case 'd': //ʮ����
                d = va_arg(ap, int);
                itoa(d, buf, 10);
                for (s = buf; *s; s++)
                {
                    uart_send_byte(*s);
                }
                Data++;
                break;
            default:
                Data++;
                break;
        }
    } /* end of else if */
    else
        uart_send_byte(*Data++);
    }
 }


