/*
 * lab7_Alex_Tran.c
 *
 *  Created on: Nov 7, 2023
 *      Author: minhn
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "lab4.h"
#include "lab5.h"
#include "lab6.h"
#include "eeprom.h"
#define FOOD    (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 4*4))) //PE4
#define FOOD_MASK 16

uint32_t timesec;
uint32_t hour;
uint32_t minute;
uint32_t time;
char str1[110];
char text [100];
char text3[100];
uint32_t nextEvent;
uint32_t smallestEventIndex = 0;
uint32_t timesec;
uint32_t FEEDING;

void findNextFeedingEvent(void)
{
    uint32_t i;
    uint32_t time_now = 0;
    int smallestEventTime = -1; // Initialize to maximum value

    while(!(HIB_CTL_R & HIB_CTL_WRC));
       time_now = HIB_RTCC_R;

    for (i = 1; i < 6; i++)
    {
        uint32_t eventTime = (readEeprom(16 * i + 3) * 3600) + (readEeprom(16 * i + 4) * 60);

        if (eventTime > time_now && eventTime < smallestEventTime && eventTime != 0xFFFFFFFF)
        {
            smallestEventTime = eventTime;
            smallestEventIndex = i;
        }
    }

    if (smallestEventTime != -1)
    {
        while(!(HIB_CTL_R & HIB_CTL_WRC));
        HIB_RTCM0_R = smallestEventTime;
        char text[70];
        hour = smallestEventTime / 3600;
        minute = (smallestEventTime % 3600) / 60;
        snprintf(text, sizeof(text), "alarm set at feeding %d, set for HH:%d MM:%d \n", smallestEventIndex, hour, minute);
        putsUart0(text);
        FEEDING = smallestEventIndex;

    }

}

void check1()
{
    snprintf(text3, sizeof(text3), "  feeding set at event %d %d %d: \n", FEEDING,readEeprom(16 * FEEDING + 1), readEeprom(16 * FEEDING+ 2) );
    putsUart0(text3);
    snprintf(text3, sizeof(text3), "  time set at %d:%d\n", readEeprom(16*FEEDING+3), readEeprom(16*FEEDING+4 ));
    putsUart0(text3);
}

void initHw8()
{
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R2;
    //SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R2;
    SYSCTL_RCGCHIB_R |= SYSCTL_RCGCHIB_R0;
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0| SYSCTL_RCGCGPIO_R1 | SYSCTL_RCGCGPIO_R5|SYSCTL_RCGCGPIO_R4 ;
    _delay_cycles(3);
    while(!(HIB_CTL_R & HIB_CTL_WRC));
        HIB_CTL_R |= HIB_CTL_RTCEN | HIB_CTL_CLK32EN;
    while(!(HIB_CTL_R & HIB_CTL_WRC));
        HIB_IM_R |= HIB_IM_RTCALT0;

    while(!(HIB_CTL_R & HIB_CTL_WRC));
            HIB_RTCLD_R = 0;

    while(!(HIB_CTL_R & HIB_CTL_WRC));
        NVIC_EN1_R |= 1 << (INT_HIBERNATE-16-32);

    GPIO_PORTE_AFSEL_R |= FOOD_MASK;
    GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE4_M0PWM4; //M0PWM4 GEN 2
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R0;                // reset PWM1 module
    SYSCTL_SRPWM_R = 0;                              // leave reset state
                                // turn-off PWM1 generator 1 (drives outs 4 and 5)
    PWM0_2_GENA_R = PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTLOAD_ZERO; // output 5 on PWM4, gen 2b, cmpb

    PWM0_2_LOAD_R = 1024;                            // set frequency to 40 MHz sys clock / 2 / 1024 = 19.53125 kHz

    PWM0_2_CMPA_R = 0;                            // red off (0=always low, 1023=always high)

    PWM0_2_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM1 generator 2

    PWM0_ENABLE_R = PWM_ENABLE_PWM4EN;
}

void timer2isr()
{
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;
    PWM0_2_CMPA_R = 0;
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
}
void timer2isralarm()
{
    while(!(HIB_CTL_R & HIB_CTL_WRC));
        uint32_t time = HIB_RTCC_R;
    speakerON();
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;
    TIMER2_TAMR_R |= TIMER_TAMR_TAMR_1_SHOT |  TIMER_TAMR_TACDIR ;
    TIMER2_TAILR_R = 40000000* readEeprom((16*FEEDING)+1);
    TIMER2_CTL_R |= TIMER_CTL_TAEN;
    TIMER2_IMR_R = TIMER_IMR_TATOIM;
    NVIC_EN0_R = 1 << (INT_TIMER2A -16);

    PWM0_2_CMPA_R = (uint32_t)((float)(readEeprom(16 * FEEDING + 2) * 1023) / 100);
    while(!(HIB_CTL_R & HIB_CTL_WRC));

        HIB_IC_R |= HIB_IM_RTCALT0;

    while(~HIB_CTL_R & HIB_CTL_WRC);
    HIB_RTCM0_R = 3600*readEeprom(16*FEEDING+3) + 60*readEeprom(16*FEEDING+4);

    putsUart0("Hibernate RTC Configured\n");
    findNextFeedingEvent();

}
int main(void)
{
    initHw4();
    initUart0();
    initHw5();
    //initHw6();
    initHw8();
    initEeprom();
    USER_DATA data;


    while(true)
    {
        findNextFeedingEvent();
        check1();

        bool valid = false;
        data.fieldCount = 0;
            // Get the string from the user
        getsUart0(&data);
            // Echo back to the user of the TTY interface for testing
        #ifdef DEBUG
        //putsUart0(data.buffer);
        putcUart0('\n');
        #endif
            // Parse fields
        parseFields(&data);
            // Echo back the parsed field data (type and fields)
        #ifdef DEBUG
        uint8_t i;
        for (i = 0; i < data.fieldCount; i++)
        {
            //putcUart0(data.fieldType[i]);
            //putcUart0('\t');
            //putsUart0(&data.buffer[data.fieldPosition[i]]);
            //putcUart0('\n');
        }
        #endif
        if (isCommand(&data, "time", 2))
        {
            valid = true;

            uint32_t hours = getFieldInteger(&data, 1);
            timesec = hours*3600;
            uint32_t minutes = getFieldInteger(&data, 2);
            timesec += minutes * 60;
            while(!(HIB_CTL_R & HIB_CTL_WRC));
                HIB_RTCLD_R = timesec;
        }
        else if (isCommand(&data, "time", 0))
        {
            valid = true;
            time = HIB_RTCC_R;
            char text[40];

            hour = time / 3600;
            minute = (time % 3600) / 60;
            uint32_t second = time % 60;
            snprintf(text, sizeof(text), "HH:%d MM:%d SS:%d\n", hour, minute, second);
            putsUart0(text);

        }
        else if (isCommand(&data, "feed", 5))
        {
            valid = true;
            FEEDING = getFieldInteger(&data, 1);
            uint32_t DURATION = getFieldInteger(&data, 2);
            uint32_t PWM = getFieldInteger(&data, 3);
            uint32_t HOURS = getFieldInteger(&data, 4);
            uint32_t MINUTES = getFieldInteger(&data, 5);

            writeEeprom((16*FEEDING), FEEDING);
            writeEeprom((16*FEEDING)+1,DURATION );
            writeEeprom((16*FEEDING)+2, PWM);
            writeEeprom((16*FEEDING)+3, HOURS);
            writeEeprom((16*FEEDING)+4,MINUTES);
            snprintf(str, sizeof(str), "feeding %d, duration %d, pwm %d, hour %d, minutes %d\n",readEeprom(16*FEEDING), readEeprom(16*FEEDING+1), readEeprom(16*FEEDING+2), readEeprom(16*FEEDING+3), readEeprom(16*FEEDING+4) );
            putsUart0(str);
            findNextFeedingEvent();
        }
        else if(isCommand(&data, "feed", 1))
        {
            valid = true;
            uint8_t j;
            for (j = 0; j <5; j++)
            {
                writeEeprom((16 * getFieldInteger(&data, 1))+j, 0xFFFFFFFF);

            }
            findNextFeedingEvent();
        }
        else if(isCommand(&data, "show", 0))
        {
            valid = true;
            uint8_t t  ;
            for(t = 1; t <6; t++)
            {
                snprintf(str1, sizeof(str1), "feeding %d, duration %d, pwm %d, hour %d, minutes %d\n",readEeprom(16*t), readEeprom(16*t+1), readEeprom(16*t+2), readEeprom(16*t+3), readEeprom(16*t+4) );
                putsUart0(str1);
            }
        }

        if (!valid)
            putsUart0("Invalid command\n");
    }
}
