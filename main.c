/*
  핀 배치

    PE.0~1 - Wi-fi              0번 입력 1번 출력
    PE.2 - 가스레인지 1번 불        on : 'F' / off : 'f'
    PE.3 - 가스레인지 2번 불        on : 'G' / off : 'g'
    PE.4 - 초인종(스위치)
    PE.5 - 트라이악 제어 인터럽트1
    PE.6 - 트라이악 제어 인터럽트2
    PE.7 - 차단된 전원 ON switch

    PB.4 - 램프 (5단계) 높을수록 밝음 // '1' ~ '5'높을수록 밝기가 셈
    PB.5 - 스피커(초인종 소리 또는 경보)
    PB.6 - 가스밸브(서보모터)       on : 'A' / off : 'a'
    PB.7 - 창문(서보모터)           open : 'B' / close : 'b'

    PD.0~7 - LCD
    PF.0 - 연기센서
    PF.1 - 온도센서
    PF.2 - 인체감지센서

    PA.0~2 - RTC
    PA.3 - 마그네틱 센서1(커튼 위)
    PA.4 - 마그네틱 센서2(커튼 아래)
    PC.6~7 - 커튼(DC motor)         open : 'C' / close : 'c'
    PC.5 - 형광등                   on : 'D' / off : 'd'
    PC.4 - 현관등              경보 on : 'E' / off : 'e'   //고휘도 LED - 0 : on, 1 : off

    PG.3 - ATmega128 전원 on/off
*/
#include <mega128.h>
#include <delay.h>
#include <stdio.h>
#include <lcd.h>
#include <ds1302.h>
#asm
   .equ __ds1302_port=0x1B ;PORTA
   .equ __ds1302_io=1
   .equ __ds1302_sclk=2
   .equ __ds1302_rst=0
   .equ __lcd_port=0x12 ;PORTD
#endasm
#define MC           1910
#define MD           1702
#define ME           1516
#define MF           1431
#define MG           1275
#define MA           1135
#define MB           1011
#define HC           955
#define HD           850
#define HE           757
#define HF           715
#define HG           637
#define N16          2*3
#define N8           4*3
#define ND4          12*3
#define N1           32*3

#define VALVE_OPEN    'A'
#define VALVE_CLOSE   'a'
#define WINDOW_OPEN   'B'
#define WINDOW_CLOSE  'b'
#define CURTAIN_OPEN  'C'
#define CURTAIN_CLOSE 'c'
#define LED1_ON       'D'
#define LED1_OFF      'd'
#define WARNING_ON    'E'
#define WARNING_OFF   'e'
#define STOVE1_ON     'F'
#define STOVE1_OFF    'f'
#define STOVE2_ON     'G'
#define STOVE2_OFF    'g'
#define LAMP_OFF      '0'
#define LAMP_1        '1'
#define LAMP_2        '2'
#define LAMP_3        '3'
#define LAMP_4        '4'
#define LAMP_5        '5'
#define POWER_SAVE    'S'

void main_init(void);
void communication(void);
void LCD_display();
void ADC_smoke_sensor();
void ADC_temperature();
void ADC_human_check();
void Play_note(unsigned int sound, unsigned int note);
void string(char *p,char code);
void warning_sound();
void power_block();
void ring_bell();
void ring_caution();

unsigned char tempo=4;
unsigned char arr[16];
char arr_t[16];
unsigned char num1,num2,num3,num4,num5,num6;
unsigned char arr1[8]={0x0E, 0x11, 0x0E, 0x04, 0x1F, 0x00, 0x10, 0x1F};
unsigned char arr2[8]={0x00, 0x1E, 0x10, 0x1E, 0x00, 0x04, 0x1F, 0x00};
unsigned char arr3[8]={0x01, 0x13, 0x13, 0x1D, 0x01, 0x08, 0x0E, 0x00};
int alarm=0;             //alarm=1 : 경보 on, alarm=0 : 경보 off
int triac_time;          // 트라이악(램프 제어) 인터럽트에서 시간
char data;               //수신 문자
int ADC_state=0, ADC_temp, ADC_smoke, ADC_human;

void main()
{
    main_init();

    while(1)
    {
       // communication();
        ADC_smoke_sensor();
        ADC_temperature();
        ADC_human_check();
        warning_sound();
        power_block();
        LCD_display();
    }
}
void main_init(void)
{
    DDRB=0xff;
    DDRC=0x00;
    DDRD=0xff;
    DDRE=0x0e;
    DDRF=0x00;
    DDRG=0x08;
    PORTG=0x00;     //PORTG.3 = 1 -> 동작 X / PORTCG.3=0 -> 동작 O
    PORTB.4=0;

   // UCSR0B=0xb8;
   // UCSR0A=0x00;
   // UCSR0C=0x26;
   // UBRR0H=0x00;
   // UBRR0L=0x07;
    lcd_init(16);
    lcd_puts("init_OK");

    rtc_set_time(7,20,7);
    rtc_set_date(25,8,14);
    rtc_init(0,0,0);
    lcd_gotoxy(0,0);
    lcd_gotoxy(0,0);
    lcd_puts("init_OK2");

    EICRB=0b10101010;
    EIMSK=0b00110000;

    TCCR1A = 0x40;
    TCCR1B = 0x18;
    TCCR1C = 0x00;
    ADCSRA=0x87;

    SREG=0x80;
    lcd_gotoxy(0,0);
    lcd_puts("init_OK3");
}
void LCD_display()
{
    //ADC=(int)ADCL+((int)ADCH<<8);
    sprintf(arr_t,": %u",ADC_temp);
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
    sprintf (arr,"%02d/%02d %02d:%02d:%02d",(num5&0x0f)+((num5>>4)&0x0f)*10,
            (num4&0x0f)+((num4>>4)&0x02)*10,(num3&0x0f)+((num3>>4)&0x03)*10,
            (num2&0x0f)+((num2>>4)&0x0f)*10,(num1&0x0f)+((num1>>4)&0x0f)*10);
    lcd_puts(arr);
}
void communication()
{
    unsigned int j;

    switch(data)
    {
        case VALVE_OPEN    : for(j=0;j<60;j++){PORTB.6=1; delay_us(1050); PORTB.6=0; delay_ms(23);} break;
        case VALVE_CLOSE   : for(j=0;j<60;j++){PORTB.6=0; delay_ms(1950); PORTB.6=1; delay_ms(23);} break;
        case WINDOW_OPEN   : for(j=0;j<60;j++){PORTB.7=1; delay_ms(1050); PORTB.7=0; delay_ms(23);} break;
        case WINDOW_CLOSE  : for(j=0;j<60;j++){PORTB.7=0; delay_ms(1950); PORTB.7=1; delay_ms(23);} break;
        case CURTAIN_OPEN  : do{PORTC.6=0; PORTC.7=1; delay_ms(10);}while(PINE.5==0);               break;
        case CURTAIN_CLOSE : do{PORTC.6=1; PORTC.7=0; delay_ms(10);}while(PINE.6==0);               break;
        case LED1_ON       : PORTC.5=0;                                                             break;
        case LED1_OFF      : PORTC.5=1;                                                             break;
        case WARNING_ON    : alarm=1;                                                               break;
        case WARNING_OFF   : alarm=0;                                                               break;
        case STOVE1_ON     : PORTE.2=0;                                                             break;
        case STOVE1_OFF    : PORTE.2=1;                                                             break;
        case STOVE2_ON     : PORTE.3=0;                                                             break;
        case STOVE2_OFF    : PORTE.3=1;                                                             break;
        case LAMP_OFF      : triac_time=0;                                                          break;
        case LAMP_1        : triac_time=1;                                                          break;
        case LAMP_2        : triac_time=2;                                                          break;
        case LAMP_3        : triac_time=3;                                                          break;
        case LAMP_4        : triac_time=4;                                                          break;
        case LAMP_5        : triac_time=5;                                                          break;
    }
}
void warning_sound()
{
    if(alarm==0)
    {
        if(ADC_human>500)
        {
            PORTC.4=0;
            delay_ms(5000);
            PORTC.4=1;
        }
        else
            PORTC.4=1;
    }
    else
    {
        if(ADC_human>500)
        {
            PORTC.5=1;
            ring_caution();            //경보 (침입자)
        }
    }
}
void power_block()
{
    if((ADC_smoke>300) && (ADC_temp>50))
    {
        ring_caution();                 //경보 (화재)
        PORTG=0xff;
    }
    if(PINE.7==0)
        PORTG=0x00;
}
void ADC_smoke_sensor()
{
    ADC_state=0;
    ADMUX=0x40;
    ADCSRA=0xcf;
    delay_ms(100);
    ADCSRA=0xc7;
    delay_ms(100);
}
void ADC_temperature()
{
    ADC_state=1;
    ADMUX=0x01;
    ADCSRA=0xcf;
    delay_ms(100);
    ADCSRA=0xc7;
    delay_ms(100);
}
void ADC_human_check()
{
    ADC_state=2;
    ADMUX=0x02;
    ADCSRA=0xcf;
    delay_ms(100);
    ADCSRA=0xc7;
    delay_ms(100);
}
interrupt [ADC_INT] void adc_project(void)
{
    switch(ADC_state)
    {
        case 0 : ADC_smoke=ADCW;
        case 1 : ADC_temp=ADCW;
        case 2 : ADC_human=ADCW;
    }
    ADC_temp=(int)(((ADC_temp*5)/1023)*0.01);
}
interrupt [EXT_INT4] void int_bell()
{
    EIMSK=0x20;
    ring_bell();                //초인종
    delay_ms(10);
    EIMSK=0x30;
}
interrupt [EXT_INT5] void triac_bright1()
{
    PORTB.4=1;
    switch(triac_time)
    {
        case 0 : delay_us(0);    break;
        case 1 : delay_us(500);  break;
        case 2 : delay_us(1000); break;
        case 3 : delay_us(3000); break;
        case 4 : delay_us(5000); break;
        case 5 : delay_us(7500);
    }
    PORTB.4=0;
}
void Play_note(unsigned int sound, unsigned int note)
{
    ICR1= sound;
    TCNT1 = 0x0000;
    TCCR1B = 0x1A;
    delay_ms(note*tempo*7);
    TCCR1B = 0x18;
}
interrupt [USART0_RXC] void a(void)
{
    data=UDR0;
    delay_us(5);
}
void string(char *p,char code)
{
    char i,a;
    a=(code<<3)|0x40;
    for(i=0;i<8;i++)lcd_write_byte(a++,*p++);
}
void ring_caution()
{
    Play_note(HE,N1);
}
void ring_bell()
{
    Play_note(HG,N8);
    Play_note(HE,N16);
    Play_note(HF,N16);
    Play_note(HG,N8);
    Play_note(HE,N16);
    Play_note(HF,N16);
    Play_note(HG,N16);
    Play_note(MG,N16);
    Play_note(MA,N16);
    Play_note(MB,N16);
    Play_note(HC,N16);
    Play_note(HD,N16);
    Play_note(HE,N16);
    Play_note(HF,N16);
    Play_note(HE,N8);
    Play_note(HC,N16);
    Play_note(HD,N16);
    Play_note(HE,N8);
    Play_note(ME,N16);
    Play_note(MF,N16);
    Play_note(MG,N16);
    Play_note(MA,N16);
    Play_note(MG,N16);
    Play_note(MF,N16);
    Play_note(MG,N16);
    Play_note(ME,N16);
    Play_note(MF,N16);
    Play_note(MG,N16);
    Play_note(MF,N8);
    Play_note(MA,N16);
    Play_note(MG,N16);
    Play_note(MF,N8);
    Play_note(ME,N16);
    Play_note(MD,N16);
    Play_note(ME,N16);
    Play_note(MD,N16);
    Play_note(MC,N16);
    Play_note(MD,N16);
    Play_note(ME,N16);
    Play_note(MF,N16);
    Play_note(MG,N16);
    Play_note(MA,N16);
    Play_note(MF,N8);
    Play_note(MA,N16);
    Play_note(MG,N16);
    Play_note(MA,N8);
    Play_note(MB,N16);
    Play_note(HC,N16);
    Play_note(MG,N16);
    Play_note(MA,N16);
    Play_note(MB,N16);
    Play_note(HC,N16);
    Play_note(HD,N16);
    Play_note(HE,N16);
    Play_note(HF,N16);
    Play_note(HG,N16);
    Play_note(HE,N8);
    Play_note(HC,N16);
    Play_note(HD,N16);
    Play_note(HE,N8);
    Play_note(HD,N16);
    Play_note(HC,N16);
    Play_note(HD,N16);
    Play_note(MB,N16);
    Play_note(HC,N16);
    Play_note(HD,N16);
    Play_note(HE,N16);
    Play_note(HD,N16);
    Play_note(HC,N16);
    Play_note(MB,N16);
    Play_note(HC,N8);
    Play_note(MA,N16);
    Play_note(MB,N16);
    Play_note(HC,N8);
    Play_note(MC,N16);
    Play_note(MD,N16);
    Play_note(ME,N16);
    Play_note(MF,N16);
    Play_note(ME,N16);
    Play_note(MD,N16);
    Play_note(ME,N16);
    Play_note(HC,N16);
    Play_note(MB,N16);
    Play_note(HC,N16);
    Play_note(MA,N8);
    Play_note(HC,N16);
    Play_note(MB,N16);
    Play_note(MA,N8);
    Play_note(MG,N16);
    Play_note(MF,N16);
    Play_note(MG,N16);
    Play_note(MF,N16);
    Play_note(ME,N16);
    Play_note(MF,N16);
    Play_note(MG,N16);
    Play_note(MA,N16);
    Play_note(MB,N16);
    Play_note(HC,N16);
    Play_note(MA,N8);
    Play_note(HC,N16);
    Play_note(MB,N16);
    Play_note(HC,N8);
    Play_note(MB,N16);
    Play_note(MA,N16);
    Play_note(MB,N16);
    Play_note(HC,N16);
    Play_note(HD,N16);
    Play_note(HC,N16);
    Play_note(MB,N16);
    Play_note(HC,N16);
    Play_note(MA,N16);
    Play_note(MB,N16);
    Play_note(HC,ND4);
}
