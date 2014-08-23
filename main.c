/*
핀 배치
 
    PE.0~1 - Wi-fi              0번 입력 1번 출력
    PE.2 - 가스레인지 1번 불        on : 'F' / off : 'f'
    PE.3 - 가스레인지 2번 불        on : 'G' / off : 'g'
    PE.4 - 초인종(스위치)
    PE.5 - 램프 인터럽트1
    PE.6 - 램프 인터럽트2
    
    PB.4 - 램프 (5단계) 높을수록 밝음
    PB.5 - 스피커(초인종 소리 또는 경보)
    PB.6 - 가스밸브(서보모터)       on : 'A' / off : 'a'
    PB.7 - 창문(서보모터)           open : 'B' / close : 'b'
                                    
    PD.0~7 - LCD                    

    PF.0 - 온도센서
    PF.1 - 연기센서                 
    PF.2 - 인체감지센서             
    PA.0~2 - RTC    
    PA.5 - 마그네틱 센서1(커튼 위)
    PA.4 - 마그네틱 센서2(커튼 아래)           

    PC.6~7 - 커튼(DC motor)         open : 'C' / close : 'c'
    PC.5 - 형광등                   on : 'D' / off : 'd'
    PC.4 - 현관등              경보 on : 'E' / off : 'e'   //고휘도 LED - 0 : on, 1 : off
*/

#include <mega128.h>
#include <delay.h>
#include <lcd.h>
#include <ds1302.h>
#define LED_ON A
#define LED_OFF a
