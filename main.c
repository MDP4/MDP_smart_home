/* 
 * LCD
 * RTC(real time clock)
 * 스피커, 초인종
 * 형광등
 * 현관등
 * 인체감지센서
 * 가스레인지(네온), 밸브(서보)
 * 커튼(DC motor, 마그네틱 센서)
 * 창문(서보)
 * 온도 센서(LM35)
 */
 
/*
  핀 배치
 
    PE.0~1 - Wi-fi              0번 입력 1번 출력
    PE.2 - 가스레인지 1번 불        on : 'F' / off : 'f'
    PE.3 - 가스레인지 2번 불        on : 'G' / off : 'g'
    PE.4 - 초인종(스위치)
    PE.5 - 마그네틱 센서1(커튼 위)
    PE.6 - 마그네틱 센서2(커튼 아래)        
    
    PB.4 - 램프 (5단계) 높을수록 밝음 // '1' ~ '5'높을수록 밝기가 셈
    PB.5 - 스피커(초인종 소리 또는 경보) 
    PB.6 - 가스밸브(서보모터)       on : 'A' / off : 'a'
    PB.7 - 창문(서보모터)           open : 'B' / close : 'b'
                                    
    PD.0~7 - LCD                    

    PF.0 - 연기센서
    PF.1 - 온도센서                 
    PF.2 - 인체감지센서             
    PA.0~2 - RTC                   

    PC.6~7 - 커튼(DC motor)         open : 'C' / close : 'c'
    PC.5 - 형광등                   on : 'D' / off : 'd'
    PC.4 - 현관등              경보 on : 'E' / off : 'e'   //고휘도 LED - 0 : on, 1 : off   
    
    PG.3 - ATmega128 전원 on/off
*/

#include <mega128.h>
#include <delay.h>
#include <lcd.h>
#include <stdio.h>
#asm
   .equ __ds1302_port=0x1B ;PORTA
   .equ __ds1302_io=1
   .equ __ds1302_sclk=2
   .equ __ds1302_rst=0
   .equ __lcd_port=0x12
#endasm
#include <ds1302.h>

unsigned char arr[16];
char arr_t[16];
unsigned char num1,num2,num3,num4,num5,num6,ADC;
unsigned char arr1[8]={0x0E, 0x11, 0x0E, 0x04, 0x1F, 0x00, 0x10, 0x1F};
unsigned char arr2[8]={0x00, 0x1E, 0x10, 0x1E, 0x00, 0x04, 0x1F, 0x00};
unsigned char arr3[8]={0x01, 0x13, 0x13, 0x1D, 0x01, 0x08, 0x0E, 0x00};
int data, i, alarm=0; //alarm=1 : 경보 on, alarm=0 : 경보 off
int ADC_state=0, ADC_temp, ADC_smoke, ADC_human;

void string(char *p,char code);
char rx_char(void);
void main_init(void);
void communication(void);
void LCD_display();
void ADC_temperature();
//void ADC_smoke();
void ADC_human_check();
void ring_bell();



void main()
{
    main_init();     
         
    while(1)
    {
        communication();
        
        ADC_temperature();  
        delay_ms(5);
        //ADC_smoke();   
        delay_ms(5);
        ADC_human_check();   
        delay_ms(5); 
        
        LCD_display();
        //ring_bell();
    }
}

void main_init(void)
{   
    rtc_set_time(7,20,7);
    rtc_set_date(25,8,14);
    rtc_init(0,0,0);

    DDRB=0xff;
    DDRC=0x00;
    DDRD=0xff;
    DDRE=0x0e;
    DDRF=0x00;
    DDRG=0x08; 
    PORTG=0x08;
    
    UCSR0B=0xb8;
    UCSR0A=0x00;
    UCSR0C=0x26;
    UBRR0H=0x00;
    UBRR0L=0x07;  
    
    #asm("sei")
    ADCSRA=0x8f; 
    
    lcd_init(16); 
}

char rx_char(void)
{
    while((UCSR0A&0x80)==0);
    return UDR0;
}

void LCD_display()
{
    ADC=(int)ADCL+((int)ADCH<<8);
    sprintf(arr_t,": %u",((ADC*5)/10)); 
    string(arr1,0);
    string(arr2,1);
    string(arr3,2);
    lcd_gotoxy(0,0);
    lcd_putchar(0);
    lcd_putchar(1);
    lcd_puts(arr_t);
    lcd_putchar(0b11011111);
    lcd_putsf("C 2014");
    lcd_putchar(2);
    lcd_gotoxy(0,1);
    num1=ds1302_read(0x81);
    num2=ds1302_read(0x83);
    num3=ds1302_read(0x85);
    num4=ds1302_read(0x87);
    num5=ds1302_read(0x89);
    num6=ds1302_read(0x8b);
    sprintf(arr,"%02d/%02d %02d:%02d:%02d",(num5&0x0f)+((num5>>4)&0x0f)*10,(num4&0x0f)+((num4>>4)&0x02)*10,(num3&0x0f)+((num3>>4)&0x03)*10,(num2&0x0f)+((num2>>4)&0x0f)*10,(num1&0x0f)+((num1>>4)&0x0f)*10);
    lcd_puts(arr);
}

void communication()
{
    unsigned int j;
    
    data=rx_char();
    switch(data)
    {
        case 'A' : for(j=0;j<60;j++){PORTB.6=1; delay_ms(19); PORTB.6=0; delay_ms(1);} break;
        case 'a' : for(j=0;j<60;j++){PORTB.6=0; delay_ms(19); PORTB.6=1; delay_ms(1);} break;
        case 'B' : for(j=0;j<60;j++){PORTB.7=1; delay_ms(19); PORTB.7=0; delay_ms(1);} break;
        case 'b' : for(j=0;j<60;j++){PORTB.7=0; delay_ms(19); PORTB.7=1; delay_ms(1);} break;
        case 'C' : do{PORTC.6=0; PORTC.7=1; delay_ms(10);}while(PINE.5==0);            break;
        case 'c' : do{PORTC.6=1; PORTC.7=0; delay_ms(10);}while(PINE.6==0);            break;
        case 'D' : alarm=1;                                                            break;
        case 'd' : alarm=0;                                                            break;
        case 'E' : PORTC.5=0;                                                          break;
        case 'e' : PORTC.5=1;                                                          break;
        case 'F' : PORTE.2=0;                                                          break;
        case 'f' : PORTE.2=1;                                                          break;
        case 'G' : PORTE.3=0;                                                          break;
        case 'g' : PORTE.3=1;                                                          break;   
        case '1' :                                                                     break;
        case '2' :                                                                     break;
        case '3' :                                                                     break;
        case '4' :                                                                     break;
        case '5' :                                                                     break;
    }                                                                                  
}
    /*
void ADC_smoke()
{
    ADC_state=0;
    ADMUX=0x00;
    ADCSRA=0xcf;
    delay_ms(20);
} */

void ADC_temperature()
{
    ADC_state=1;
    ADMUX=0x01;
    ADCSRA=0xcf;
    delay_ms(20);
}
void ADC_human_check()
{
    ADC_state=2;
    ADMUX=0x02;
    ADCSRA=0xcf;
    delay_ms(20);
}

interrupt [ADC_INT] void adc_project()
{
    switch(ADC_state)
    {
        case 0 : ADC_smoke=ADCW;
        case 1 : ADC_temp=ADCW;
        case 2 : ADC_human=ADCW;
    }
}

void string(char *p,char code)
{
 char i,a;
 a=(code<<3)|0x40;
 for(i=0;i<8;i++)lcd_write_byte(a++,*p++);
 }
