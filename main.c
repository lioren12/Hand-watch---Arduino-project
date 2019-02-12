#include "config.h"
#include "lcd.h"
#include "led.h"
#include "time.h" 
#include "alarm.h" 
#include "utils.h"
#include "rgbled.h"
#include "btn.h"
#include <xc.h>  
#include <sys/attribs.h>
#include <string.h>

//==================================================
//              Global Configuration
//==================================================
// Device Config Bits in  DEVCFG1:	
#pragma config FNOSC =	FRCPLL
#pragma config FSOSCEN =	OFF
#pragma config POSCMOD =	XT // EXCHANGED FROM EC
#pragma config OSCIOFNC =	ON
#pragma config FPBDIV =     DIV_1 // Frequency 80G - can be divided by 2 - "DIV_2"

// Device Config Bits in  DEVCFG2:	
#pragma config FPLLIDIV =	DIV_2
#pragma config FPLLMUL =	MUL_20
#pragma config FPLLODIV =	DIV_1

#pragma config JTAGEN = OFF // DISABLING DIBUGGER
#pragma config FWDTEN = OFF // DISABLING DIBUGGER

#define _XTAL_FREQ 8000000

void Timer4Setup()
{
	static int fTimerInitialised = 0;
	if (!fTimerInitialised)
	{
		//  setup peripheral
		PR4 = 10000;                        //             set period register, generates one interrupt every 1 ms
		TMR4 = 0;                           //             initialize count to 0
		T4CONbits.TCKPS = 3;                //            1:256 prescale value
		T4CONbits.TGATE = 0;                //             not gated input (the default)
		T4CONbits.TCS = 0;                  //             PCBLK input (the default)
		T4CONbits.ON = 1;                   //             turn on Timer1
		IPC4bits.T4IP = 2;                  //             priority
		IPC4bits.T4IS = 0;                  //             subpriority
		IFS0bits.T4IF = 0;                  //             clear interrupt flag
		IEC0bits.T4IE = 1;                  //             enable interrupt
		fTimerInitialised = 1;
	}
}

//==============================================================================
//Interrupt Handler - handled every 1msec
//==============================================================================
long CurrentTime = 0;
long TimeForSetTime = 0;

void __ISR(_TIMER_4_VECTOR, ipl2auto) Timer4SR(void)
{
    if (!ClockTimeSet.Active) {
        CurrentTime++;
		UpdateTime(CurrentTime);
    }else{
        TimeForSetTime++;
    }
    if (Stopper.Active && ((CurrentTime - Stopper.TimeStamp) % HUNDREDTH_IN_MS == 0)) { 
		UpdateStopper();
    }
    if (Alarm.InMiddleOfAlarm) { 
		HandleLeds(CurrentTime);
	}

	IFS0bits.T4IF = 0;  // clear interrupt flag
}

//==============================================================================
//this function initializes all needed components
//==============================================================================
void init() {
	LCD_Init();
	LED_Init();
	BTN_Init();
	//RGBLED_Timer5Setup();
	Timer4Setup();
	SWT_Init();
	RGBLED_Init();
	SSD_Init();
}

void main() {
//==============================================================================
// Internal initializations of needed components
//==============================================================================
	init(); //initialize all needed components from above
    InitTime(); 
	InitTimeSet(&ClockTimeSet); 
	InitTimeSet(&AlarmTimeSet); 
	InitStopper(); 
	DisplayStopper(); 
	InitAlarm();
	InitAudio();

	bool FirstTimeClockIsBackOn = true;
    LCD_WriteStringAtPos("BASYSMX3 Project", 1, 0);
    
//==============================================================================
// Start of program
//==============================================================================
	while (1)
	{
		if ((SWT_GetValue(0) == 0 && SWT_GetValue(1) == 0) || (SWT_GetValue(0) == 1 && SWT_GetValue(1) == 1)) {  //Show time
			DisplayTime(CurrentTime, &FirstTimeClockIsBackOn);
			HandleStopper();
		}
		else {
			SSD_Close();
		}

		if (!Alarm.InMiddleOfAlarm) {  // check if need to start alarm
			Alarm.NeedToStartAlarm = CheckIfNeedToStartAlarm();
			if (Alarm.NeedToStartAlarm) {
				Alarm.StartAlarmTimeStamp = CurrentTime;
				Alarm.InMiddleOfAlarm = true;
			}
		}
		if (Alarm.InMiddleOfAlarm) {
			HandleAlarm(CurrentTime);
		}

		while (SWT_GetValue(0) == 1 && SWT_GetValue(1) == 0) {  //Clock tuning
			SSD_Close();
            FirstTimeClockIsBackOn = true;
			if (!ClockTimeSet.Active) {
				ClockTimeSet.TimeSetTimeStamp = TimeForSetTime;
				ClockTimeSet.CurrentState = On;
				ClockTimeSet.Active = true;
			}
			HandleTimeSet(TimeForSetTime, &ClockTime, &ClockTimeSet, false);
		}
        InitTimeSet(&ClockTimeSet);

		if (SWT_GetValue(0) == 0 && SWT_GetValue(1) == 1) {  // Alarm tuning
			FirstTimeClockIsBackOn = true;
			if (!AlarmTimeSet.Active) {
				AlarmTimeSet.TimeSetTimeStamp = CurrentTime;
				AlarmTimeSet.CurrentState = On;
				AlarmTimeSet.Active = true;
			}
			HandleTimeSet(CurrentTime, &AlarmTime, &AlarmTimeSet, true);
		}
		else {
            InitTimeSet(&AlarmTimeSet);
		}
    }
}




