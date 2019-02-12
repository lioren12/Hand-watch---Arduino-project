#include "led.h"
#include "time.h"
#include "alarm.h"
#include "rgbled.h"
#include <xc.h>
#include <sys/attribs.h>

void InitAlarm();
bool CheckIfNeedToStartAlarm();
void HandleAlarm(long CurrentTime);
void HandleRingtone();
void HandleLeds(long CurrentTime);
void DisplayLED();
void PlayAudio();
void InitAudio();
void UpdateAudio();
void StopAudio();

void InitAlarm() {
	Alarm.CurrentLEDToDisplay = 0; // 0 - 7
	Alarm.NeedToStartAlarm = false;
	Alarm.InMiddleOfAlarm = false;
	Alarm.StartAlarmTimeStamp = 0;
	Alarm.TurningLEDsFromLowToHigh = true;
    Alarm.FirstPlayFlag = true;
}

bool CheckIfNeedToStartAlarm() {
	return ClockTime.Seconds == AlarmTime.Seconds &&
		ClockTime.Minutes == AlarmTime.Minutes &&
		ClockTime.Hours == AlarmTime.Hours;
}

void HandleAlarm(long CurrentTime) {
	HandleRingtone();
	if (((CurrentTime - Alarm.StartAlarmTimeStamp) >= (MINUTE_IN_MS - WINDOW_OF_OPERATION)) && (CurrentTime - Alarm.StartAlarmTimeStamp <= (MINUTE_IN_MS + WINDOW_OF_OPERATION))) {
		InitAlarm();
		LED_SetGroupValue(0);  // turn all LEDs off
        StopAudio();  // stop ringtone
	}
}

void HandleRingtone() { 
	if (SWT_GetValue(7) == 1) { // play
        if(Alarm.FirstPlayFlag){
            PlayAudio();
            Alarm.FirstPlayFlag = false;
        }
        UpdateAudio();
	}
	else { // stop
        StopAudio();
        Alarm.FirstPlayFlag = true;
	}
}

void HandleLeds(long CurrentTime) {
	if (SWT_GetValue(6) == 1) { 
        if ((CurrentTime - Alarm.StartAlarmTimeStamp) % SECOND_IN_MS == OFFSET_FOR_LEDS){
            DisplayLED();
		}
	}
	else {
		LED_SetGroupValue(0);
	}
}

void DisplayLED() {
	
	int LEDToTurnOff = Alarm.TurningLEDsFromLowToHigh ? Alarm.CurrentLEDToDisplay - 1 : Alarm.CurrentLEDToDisplay + 1;

    LED_SetValue(Alarm.CurrentLEDToDisplay,1);

	Alarm.CurrentLEDToDisplay = Alarm.TurningLEDsFromLowToHigh ? Alarm.CurrentLEDToDisplay + 1 : Alarm.CurrentLEDToDisplay - 1;
	if (LEDToTurnOff == -1) {
		return;
	}

    LED_SetValue(LEDToTurnOff,0);
    
    if (Alarm.CurrentLEDToDisplay == 8) {
		Alarm.TurningLEDsFromLowToHigh = false;
		Alarm.CurrentLEDToDisplay = 6;
	}
	else if (Alarm.CurrentLEDToDisplay == -1) {
		Alarm.TurningLEDsFromLowToHigh = true;
		Alarm.CurrentLEDToDisplay = 1;
	}
}

void __ISR(_TIMER_3_VECTOR, IPL7AUTO) Timer3ISR(void) 
{  
    IFS0bits.T3IF = 0;      // clear Timer3 interrupt flag
}

void PlayAudio()
{    
    T3CONbits.ON = 1;       // turn on Timer3
    OC1CONbits.ON = 1;      // Turn on OC1
}

void StopAudio()
{
    IFS0bits.T3IF = 0;      // clear Timer3 interrupt flag    
    T3CONbits.ON = 0;       // turn off Timer3
    OC1CONbits.ON = 0;      // Turn off OC1
    IFS0bits.T3IF = 0;      // clear Timer3 interrupt flag
}

int AudioPeriod = 40000000; //number of cycles between Timer3 interrupts.  40000000 is just for initialization
void InitAudio(){        
    tris_A_OUT = 0;     // Configure AUDIO output as digital output.
    rp_A_OUT = 0x0C;    // 1100 = OC1
    ansel_A_OUT = 0;    // // disable analog (set pins as digital)   
    
    PR3 = AudioPeriod; //number of cycles between Timer3 interrupts
    OC1R = AudioPeriod / 2;
    OC1RS = AudioPeriod / 2;
    
    TMR3 = 0;
    T3CONbits.TCKPS = 1;     //1:256 prescale value
    T3CONbits.TGATE = 0;     //not gated input (the default)
    T3CONbits.TCS = 0;       //PCBLK input (the default)
    T3CONbits.ON = 0;        //turn on Timer3
 
    OC1CONbits.ON = 0;       // Turn off OC1 while doing setup.
    OC1CONbits.OCM = 6;      // PWM mode on OC1; Fault pin is disabled
    OC1CONbits.OCTSEL = 1;   // Timer3 is the clock source for this Output Compare module
    OC1CONbits.ON = 1;       // Start the OC1 module  

    IPC3bits.T3IP = 7;      // interrupt priority
    IPC3bits.T3IS = 3;      // interrupt subpriority
//    IEC0bits.T3IE = 1;      // enable Timer3 interrupt    
    IEC0bits.T3IE = 0;      // disable Timer3 interrupt    
    IFS0bits.T3IF = 0;      // clear Timer3 interrupt flag
    macro_enable_interrupts();  // enable interrupts at CPU
}

void UpdateAudio(){
    CurrentTimeAudio=_CP0_GET_COUNT();
    int NoteTime=40000000/8;   //   sec/8=128msec    e.c every 128msec move to next note.
    if(CurrentTimeAudio-AudioTimestamp>NoteTime || AudioTimestamp>CurrentTimeAudio){   
        AudioTimestamp=(CurrentTimeAudio/NoteTime)*NoteTime;
        if(++DurationCount<Duration[NoteIndex])
            return;
        DurationCount=0;
        
        NoteIndex=(NoteIndex+1)%SONG_LENGTH; // Song is cyclic
        
        //calculate number of cycles between interrupts: (0.5*clock_rate)/note_frequency
        AudioPeriod=20000000/Song[NoteIndex];   
        PR3 = AudioPeriod;
        OC1R = AudioPeriod / 2;
        OC1RS = AudioPeriod / 2;
    }
}