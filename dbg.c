
#include "dbg.h"

void printchar(const char ch);
void printstr(const char *str);
void printint(const int in);
void printlong(const long lon);
void printfloat(const float fl);
void printhex (const unsigned int x);
char *itoa_my(long value,char *string,int radix);


/* ���� �� -radix =10 ��ʾ 10 ���ƣ��������Ϊ 0
 * -value Ҫת����������
 * -string ת������ַ���
 * -radix = 10
 * ��� ����
 * ���� ����
*/
char *itoa_my(long value,char *string,int radix)
{
    char zm[37]="0123456789abcdefghijklmnopqrstuvwxyz";
    char aa[80]={0};
    char *cp=string;
    int i=0;
    if(radix<2||radix>36)//�����˶Դ���ļ��
    {
        return string;
    }
    if(value<0)
    {
        return string;
    }
    else if(value ==0)
    {
        *cp=0x30;
        cp++;
        *cp =0x30;
        cp++;
        *cp='\0';
        return string;
    }
    if((value<=0x0f)&&(value>0))
    {
        aa[i++]=zm[value%radix];
        aa[i++]=zm[0];
    }
    if (value>0x0f)
    {
        long temp =value;
        while(temp>0)
        {

            aa[i++]=zm[temp%radix];
            temp/=radix;
        }
    }
    for(int j=i-1;j>=0;j--)
    {
        *cp++=aa[j];
    }
    *cp='\0';
    return string;
}

/*
������յ����ַ����� ��ʹ��putchar()�������Ļ
*/
void printchar(const char ch)
{
    usart2_send_byte(ch);
}
/*
�������������ͨ���ݹ齫��ÿһλת��Ϊ�ַ����
*/
void printint(const int in)
{
    char *s;
   char buf[16];
   itoa_my( in, buf,  10);
   for(s=buf;*s;s++)
   {
     usart2_send_byte(*s);
   }
}

void printlong(const long lon)
{
    char *s;
   char buf[16];
   itoa_my( lon, buf,  10);
   for(s=buf;*s;s++)
   {
     usart2_send_byte(*s);
   }
}

//16�������
void printhex (const unsigned int x)
{
    char *s;
    char buf[16];
    itoa_my( x, buf,  16);
    for(s=buf;*s;s++)
    {
        usart2_send_byte(*s);
    }

}
/*
������ַ��������ַ���������־
*/
void printstr(const char *str)
{
    while (*str)
        usart2_send_byte(*str++);
}
/*
��������
���ȴ�����������
****************
�ڴ���С������
����ֻ����С�����5λ������λȱʡ��
*/
void printfloat(const float fl)
{
    int tmpint = (int)fl;
    uint16 tmpflt = (int)(100000 * (fl - tmpint));
    if(tmpflt % 10 >= 5)
    {
        tmpflt = tmpflt / 10 + 1;
    }
    else
    {
        tmpflt = tmpflt / 10;
    }
    printint(tmpint);
    usart2_send_byte('.');
    printint(tmpflt);
}
/*
�Լ���printf��������ʵ��
*/

void my_printf(const char *format,...)
{
    va_list arg;
    char ch;
    int in;
    long lon;
    char *str;
    float fl;
    unsigned int x;
    va_start(arg, format);  //4
    while (*format)
    {
        if (*format != '%')
        {
            usart2_send_byte(*format);
            format++;
        }
        else
        {
            format++;
            switch (*format)
            {
            case 'c':
                ch = va_arg(arg, char);
                printchar(ch);
                format++;
                break;
            case 'l':
                lon = va_arg(arg, long);
                printlong(lon);
                format++;
                break;
            case 'd':
                in = va_arg(arg, int);
                printint(in);
                format++;
                break;
            case 's':
                str = va_arg(arg, char*);
                printstr(str);
                format++;
                break;
            case 'f':
                fl = va_arg(arg, double);
                printfloat(fl);
                format++;
                break;
            case 'x':
                x = va_arg(arg, unsigned int);
                printhex(x);
                format++;
                break;
            default:
                format--;
                usart2_send_byte(*format);
                format++;
                break;
            }
        }
    }
    va_end(arg);
}

void dbg_hex(char *buf,char len)
{
    for(uint8 i=0;i<len;i++)
    {
        printhex(*buf++);
        usart2_send_byte(' ');
    }
    my_printf("\r\n");
}





