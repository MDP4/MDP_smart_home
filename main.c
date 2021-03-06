#include <mega128.h>
#include <delay.h>
#include <stdio.h>
#include <lcd.h>
#include <ds1302.h>
#include <eeprom.h>
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
void ADC_function();
int ADC_func(unsigned char);
void Play_note(unsigned int sound, unsigned int note);
void string(char *p,char code);
void warning_sound();
void power_block();
void ring_bell();
void ring_caution();
void HTTP_Put_Char(char);
void Put_Char(char);
void triac_bright1();

unsigned char tempo=5,tmp;
char arr[16];
char arr_t[16];
char num1,num2,num3,num4,num5,num6,num7;
char arr1[8]={0x0E, 0x11, 0x0E, 0x04, 0x1F, 0x00, 0x10, 0x1F};
char arr2[8]={0x00, 0x1E, 0x10, 0x1E, 0x00, 0x04, 0x1F, 0x00};
char arr3[8]={0x01, 0x13, 0x13, 0x1D, 0x01, 0x08, 0x0E, 0x00};
int alarm;             //alarm=1 : 경보 on, alarm=0 : 경보 off
int triac_time=4,i;          // 트라이악(램프 제어) 인터럽트에서 시간
char data;               //수신 문자
int  ADC_temp, ADC_smoke, ADC_human;
int line;

void main()
{
    main_init();
    while(1)
    {
        communication(); 
        LCD_display();  
        ADC_function();
        warning_sound();
        power_block(); 
        triac_bright1();        
        delay_ms(300);
    }
}
void main_init(void)
{
    DDRB=0xFF;
    DDRC=0b11100111;
    DDRD=0xFF;
    DDRE=0x0E;
    DDRF=0x00;
    DDRG=0x08;
    PORTG=0x00;     //PORTG.3 = 1 -> 동작 X / PORTCG.3=0 -> 동작 O
    PORTB.4=0;
    
    UCSR0A=0x00;
    UCSR0B=0xD8;    // R_T_interrupt_en R_T_EN
    UCSR0C=0x06;    //8bit Data
    UBRR0H=0x00;
    UBRR0L=103;     //9600bps 

    lcd_init(16);

    rtc_set_time(9,0,1);
    rtc_set_date(3,11,14);
    rtc_init(0,0,0);
    lcd_gotoxy(0,0);

    EICRB=0b10101010;
    EIMSK=0b00110000;

    TCCR1A = 0x40;
    TCCR1B = 0x18;
    TCCR1C = 0x00;
    
    TCCR0=0b00000111; 
    TIMSK|=1; 
    TCNT0=0;
    
    ADCSRA=0x87;

    SREG=0x80;
    lcd_gotoxy(0,0);
}
void LCD_display()
{
    sprintf(arr_t,": %d",tmp=(int)(ADC_temp*0.4));
    string(arr1,0);
    string(arr2,1);
    string(arr3,2);
    lcd_gotoxy(0,0);
    lcd_putchar(0);
    lcd_putchar(1);
    lcd_puts(arr_t);
    lcd_putchar(0xDF);
    lcd_putsf("C 2014");
    lcd_putchar(2);
    lcd_gotoxy(0,1);
    num1=ds1302_read(0x81);
    num2=ds1302_read(0x83);
    num3=ds1302_read(0x85);
    num4=ds1302_read(0x87);
    num5=ds1302_read(0x89);
    num6=ds1302_read(0x8B);
    num7=ds1302_read(0x8D);
    sprintf (arr,"%02d/%02d %02d:%02d:%02d",(num5&0x0f)+((num5>>4)&0x0f)*10,
            (num4&0x0f)+((num4>>4)&0x02)*10,(num3&0x0f)+((num3>>4)&0x03)*10,
            (num2&0x0f)+((num2>>4)&0x0f)*10,(num1&0x0f)+((num1>>4)&0x0f)*10);
    lcd_puts(arr);
}
void HTTP_Put_Char(char tem)    //보낼 문자
{
    unsigned int gg = 0;                                          
    while((eeprom_read_byte(gg) != '\0')) Put_Char(eeprom_read_byte(gg++));  
    Put_Char(':');
    Put_Char(' ');
    Put_Char('2'); 
    Put_Char('\n');
    Put_Char('\n');
    Put_Char((tem/10)+'0');   
    Put_Char((tem%10)+'0');
}
void communication()
{
    unsigned int j;
    delay_ms(10);
    switch(data)
    {
        case VALVE_OPEN    : for(j=0;j<40;j++){PORTB.6=1; delay_us(200); PORTB.6=0; delay_ms(23);} break;
        case VALVE_CLOSE   : for(j=0;j<40;j++){PORTB.6=1; delay_us(1400); PORTB.6=0; delay_ms(23);} break;
        case WINDOW_OPEN   : for(j=0;j<40;j++){PORTB.7=1; delay_us(200); PORTB.7=0; delay_ms(23);} break;
        case WINDOW_CLOSE  : for(j=0;j<40;j++){PORTB.7=1; delay_us(1400); PORTB.7=0; delay_ms(23);} break;
        case CURTAIN_OPEN  : PORTC.6=0; PORTC.5=1; delay_ms(500); PORTC.6=0; PORTC.5=0;                                break;
        case CURTAIN_CLOSE : PORTC.6=1; PORTC.5=0; delay_ms(500); PORTC.6=0; PORTC.5=0;                                  break;
        case LED1_ON       : PORTC.3=1;                                                             break;
        case LED1_OFF      : PORTC.3=0;                                                             break;
        case WARNING_ON    : alarm=1;                                                               break;
        case WARNING_OFF   : alarm=0;                                                               break;
        case STOVE1_ON     : PORTE.2=0;                                                             break;
        case STOVE1_OFF    : PORTE.2=1;                                                             break;
        case LAMP_OFF      : triac_time=0;                                                          break;
        case LAMP_1        : triac_time=1;                                                          break;
        case LAMP_2        : triac_time=2;                                                          break;
        case LAMP_3        : triac_time=3;                                                          break;
        case LAMP_4        : triac_time=4;                                                          break;
        case LAMP_5        : triac_time=5; 
    }
    data=0;
       
     lcd_init(16);
}
void warning_sound()
{
    if((alarm==0)&&(ADC_human>50))
    {
            PORTC.4=1;
            delay_ms(5000);
            PORTC.4=0;
    }
    else if((alarm==1)&&(ADC_human>50))
        {
            PORTC.5=1;
            while(1)ring_caution();            //경보 (침입자)
        }
}
void power_block()
{
    if((ADC_smoke>300) && ((int)((ADC_temp/5)*1023*0.01)>50))
    {
        ring_caution();                 //경보 (화재)
        //PORTG=0xff;
    }
    //if(PINE.7==0)   PORTG=0x00;
}
void ADC_function()
{
    ADC_smoke=ADC_func(0x00);
    delay_ms(15);
    ADC_temp=ADC_func(0x01);
    delay_ms(15);
    ADC_human=ADC_func(0x02);
    delay_ms(15);
}
int ADC_func(unsigned char adc_input)
{
    ADMUX=adc_input|0x40;
    ADCSRA|=0xc7; 
    delay_ms(20);
    while(!(ADCSRA&0x10));
    ADCSRA|=0x10;
    return ADCW;
}
interrupt [EXT_INT4] void int_bell()
{
    ring_bell();                //초인종
    delay_ms(2);
}

interrupt [TIM0_OVF] void temp()
{
    i++;
      if(i==500)
     {
        HTTP_Put_Char(tmp); 
      }
}  

void triac_bright1()
{
    
    switch(triac_time)
    {
        case 0 :                 break;
        case 1 : delay_us(500);  break;
        case 2 : delay_us(1000); break;
        case 3 : delay_us(3000); break;
        case 4 : delay_us(5000); break;
        case 5 : delay_us(7500);
    }
    PORTB.4=0;
    switch(triac_time)
    {
        case 0 : delay_us(16666); break;
        case 1 : delay_us(16166); break;
        case 2 : delay_us(15666); break;
        case 3 : delay_us(13666); break;
        case 4 : delay_us(1100);  break;
        case 5 : delay_us(9166);
    }  
    PORTB.4=1;
}
void Put_Char(char UART_String)
{
    while (!(UCSR0A & 0x20));
    UDR0 = UART_String;
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
      if(UDR0=='\n') line++;
      if(line>=9)
      {
            data=UDR0;
	   line=0;
      }
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
     lcd_init(16);
}
