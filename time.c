#include "time.h"

void InitTime();
void InitTimeSet(TimeSetParameters *TimeSet);
void InitStopper();
void UpdateSeconds(long CurrentTime);
void UpdateMinutes(long CurrentTime);
void UpdateHours(long CurrentTime);
void HandleTimeSet(long TimeForSetTime, TimeParameters *Time, TimeSetParameters *TimeSet, bool IsAlarm);
void DisplayTime(long CurrentTime, bool *FirstTime);
void UpdateBTNCNumOfPushsInSetTime(TimeSetParameters *TimeSet);
void CheckIfNeedToChangeDisplay(long TimeForSetTime, TimeSetParameters *TimeSet);
void DisplayTuningTime(bool *FirstTimeSetUpdate, StateEnum HH, StateEnum MM, StateEnum SS, bool IsAlarm);
void HandleLongPush(TimeParameters *Time, TimeSetParameters *TimeSet, bool Increase, long TimeForSetTime);
void UpdateClockByOne(TimeParameters *Time, TimeSetParameters *TimeSet, bool Increase);
void HandleStopper();
void ResetStopper();
void UpdateStopper();
void DisplayStopper();

void InitTime() { 
	ClockTime.Seconds = 0;
	ClockTime.Minutes = 0;
	ClockTime.Hours = 0;
    ClockTime.JustUpdatedWholeMinute = false;
    ClockTime.JustUpdatedWholeHour = false;
	AlarmTime.Seconds = 0;
	AlarmTime.Minutes = 0;
	AlarmTime.Hours = 0;
    AlarmTime.JustUpdatedWholeMinute = false;
    AlarmTime.JustUpdatedWholeHour = false;
}

void InitTimeSet(TimeSetParameters *TimeSet) {
	TimeSet->Active = false;
	TimeSet->TimeSetTimeStamp = 0;
	TimeSet->BTNCNumOfPushs = None;
	TimeSet->CurrentState = NoState;
	TimeSet->DisplayFirstTimeOnNone = true;
	TimeSet->FirstLongPushUpdate = true;
    TimeSet->FirstTimeSetUpdate = true;
	TimeSet->PushTimeStamp = 0;
	TimeSet->NeedToChangeDisplay = false;
}

void InitStopper() {
	Stopper.Seconds = 0;
	Stopper.Hundredths = 0;
	Stopper.Active = false;
}

void UpdateTime(long CurrentTime) {
	UpdateSeconds(CurrentTime);
	UpdateMinutes(CurrentTime);
	UpdateHours(CurrentTime);
}

void UpdateSeconds(long CurrentTime) {
	if (CurrentTime % 1000 == 0) {
		ClockTime.Seconds++;
        if (ClockTime.Seconds == 60){
            ClockTime.JustUpdatedWholeMinute = true;
        }
	}
}

void UpdateMinutes(long CurrentTime) {
	if ((ClockTime.Seconds % 60 == 0) && (CurrentTime >= CLOCK_INIT_OFFSET) && (ClockTime.JustUpdatedWholeMinute)) {
		ClockTime.Minutes++;
		ClockTime.Seconds = 0;
        ClockTime.JustUpdatedWholeMinute = false;
        if (ClockTime.Minutes == 60){
            ClockTime.JustUpdatedWholeHour = true;
        }
	}
}

void UpdateHours(long CurrentTime) { 
	if ((ClockTime.Minutes % 60 == 0) && (CurrentTime >= CLOCK_INIT_OFFSET) && (ClockTime.JustUpdatedWholeHour)) {
		ClockTime.Hours++;
		ClockTime.Minutes = 0;
        ClockTime.JustUpdatedWholeHour = false;
		if (ClockTime.Hours == 24) {
			ClockTime.Hours = 0;
		}
	}
}

void HandleTimeSet(long TimeForSetTime, TimeParameters *Time, TimeSetParameters *TimeSet, bool IsAlarm) {

	UpdateBTNCNumOfPushsInSetTime(TimeSet);
	CheckIfNeedToChangeDisplay(TimeForSetTime, TimeSet);
	if (TimeSet->DisplayFirstTimeOnNone) {
		DisplayTuningTime(&(TimeSet->FirstTimeSetUpdate), On, On, On, IsAlarm);
		TimeSet->DisplayFirstTimeOnNone = false;
	}
	if (TimeSet->NeedToChangeDisplay) {
		TimeSet->NeedToChangeDisplay = false;
		switch (TimeSet->BTNCNumOfPushs) {
		case HH:
			DisplayTuningTime(&(TimeSet->FirstTimeSetUpdate), TimeSet->CurrentState, On, On, IsAlarm);
			break;
		case MM:
			DisplayTuningTime(&(TimeSet->FirstTimeSetUpdate), On, TimeSet->CurrentState, On, IsAlarm);
			break;
		case SS:
			DisplayTuningTime(&(TimeSet->FirstTimeSetUpdate), On, On, TimeSet->CurrentState, IsAlarm);
			break;
		}
	}

	if (TimeSet->BTNCNumOfPushs != None) {
        
		if (BTN_GetValue(3)) { // when button BTNR is pushed
			HandleLongPush(Time, TimeSet, true, TimeForSetTime);
            DisplayTuningTime(&(TimeSet->FirstTimeSetUpdate), On, On, On, IsAlarm);
            TimeSet->DisplayFirstTimeOnNone = true;
		}
		else if (BTN_GetValue(1)) { // when button BTNL is pushed
			HandleLongPush(Time, TimeSet, false, TimeForSetTime);
            DisplayTuningTime(&(TimeSet->FirstTimeSetUpdate), On, On, On, IsAlarm);
            TimeSet->DisplayFirstTimeOnNone = true;
		}
		else {
			TimeSet->FirstLongPushUpdate = true;
		}
	}
}

void DisplayTime(long CurrentTime, bool *FirstTime) {
	int H1, H2, M1, M2, S1, S2;
	char H1_buffer[2], H2_buffer[2], M1_buffer[2], M2_buffer[2], S1_buffer[2], S2_buffer[2]; // for changing to string.
	S1 = ClockTime.Seconds / 10;
	S2 = ClockTime.Seconds % 10;
	itoa(S1_buffer, S1, 10);
	itoa(S2_buffer, S2, 10);
	M1 = ClockTime.Minutes / 10;
	M2 = ClockTime.Minutes % 10;
	itoa(M1_buffer, M1, 10);
	itoa(M2_buffer, M2, 10);
	H1 = ClockTime.Hours / 10;
	H2 = ClockTime.Hours % 10;
	itoa(H1_buffer, H1, 10);
	itoa(H2_buffer, H2, 10);
	if (*FirstTime) {
		LCD_WriteStringAtPos("                ", 0, 0);
		LCD_WriteStringAtPos(S2_buffer, 0, 11);
		LCD_WriteStringAtPos(S1_buffer, 0, 10);
		LCD_WriteStringAtPos(M2_buffer, 0, 8);
		LCD_WriteStringAtPos(M1_buffer, 0, 7);
		LCD_WriteStringAtPos(H2_buffer, 0, 5);
		LCD_WriteStringAtPos(H1_buffer, 0, 4);
		LCD_WriteStringAtPos(":", 0, 9);
		LCD_WriteStringAtPos(":", 0, 6);
		SSD_Init();
		*FirstTime = false;
	}
	else {
		if ((CurrentTime % 1000 <= WINDOW_OF_OPERATION) || (CurrentTime % SECOND_IN_MS >= (SECOND_IN_MS - WINDOW_OF_OPERATION))) { // need to update seconds
			LCD_WriteStringAtPos(S2_buffer, 0, 11);
			LCD_WriteStringAtPos(S1_buffer, 0, 10);
		}
		if (ClockTime.Seconds == 0) {  // need to update minutes
			LCD_WriteStringAtPos(M2_buffer, 0, 8);
			LCD_WriteStringAtPos(M1_buffer, 0, 7);
		}
		if (ClockTime.Minutes == 0) {  // need to update hours
			LCD_WriteStringAtPos(H2_buffer, 0, 5);
			LCD_WriteStringAtPos(H1_buffer, 0, 4);
		}
	}
}

void UpdateBTNCNumOfPushsInSetTime(TimeSetParameters *TimeSet) {
	if (BTN_GetValue(2)) { // when button BTNC is pushed
   		while (BTN_GetValue(2)) {
            
        } // wait for user to release button
        TimeSet->BTNCNumOfPushs = (TimeSet->BTNCNumOfPushs + 1);
        if (TimeSet->BTNCNumOfPushs == 4){
            TimeSet->BTNCNumOfPushs = 0;
        }
		if (TimeSet->BTNCNumOfPushs == None) {
			TimeSet->DisplayFirstTimeOnNone = true;
		}
	}
}

void CheckIfNeedToChangeDisplay(long TimeForSetTime, TimeSetParameters *TimeSet) {
    bool InTimeToChangeDiaplay = (TimeForSetTime - TimeSet->TimeSetTimeStamp) % (SECOND_IN_MS / 2) <= OFFSET_FOR_SET_TIME ||
                                 (TimeForSetTime - TimeSet->TimeSetTimeStamp) % (SECOND_IN_MS / 2) >= (SECOND_IN_MS / 2) - OFFSET_FOR_SET_TIME;
    if (InTimeToChangeDiaplay && (TimeSet->BTNCNumOfPushs != None)) {
		TimeSet->NeedToChangeDisplay = true;
		if (TimeSet->CurrentState == On) {
			TimeSet->CurrentState = Off;
		}
		else {
			TimeSet->CurrentState = On;
		}
	}
}

void DisplayTuningTime(bool *FirstTimeSetUpdate, StateEnum HH, StateEnum MM, StateEnum SS, bool IsAlarm) { // display SET HH:MM:SS
    int H1,H2,M1,M2,S1,S2;
    char H1_buffer[2], H2_buffer[2], M1_buffer[2], M2_buffer[2], S1_buffer[2], S2_buffer[2]; // for changing to string.
    TimeParameters Time = IsAlarm ? AlarmTime : ClockTime;
    if (*FirstTimeSetUpdate){
        if (!IsAlarm){
            LCD_WriteStringAtPos("SET             ",0,0);
        }
        else {
            LCD_WriteStringAtPos("ALARM           ",0,0);
        }
        (*FirstTimeSetUpdate) = false;
        LCD_WriteStringAtPos(":",0,11);
        LCD_WriteStringAtPos(":",0,8);
    }
    if (HH == On){
        H1 = Time.Hours / 10;
        H2 = Time.Hours % 10;
        itoa(H1_buffer,H1,10);
        itoa(H2_buffer,H2,10);
        LCD_WriteStringAtPos(H2_buffer,0,7);
        LCD_WriteStringAtPos(H1_buffer,0,6);
    }else {
        LCD_WriteStringAtPos("  ",0,6);
    }
    if (MM == On){
        M1 = Time.Minutes / 10;
        M2 = Time.Minutes % 10;
        itoa(M1_buffer,M1,10);
        itoa(M2_buffer,M2,10); 
        LCD_WriteStringAtPos(M2_buffer,0,10);
        LCD_WriteStringAtPos(M1_buffer,0,9);
    }
    else {
	   LCD_WriteStringAtPos("  ",0,9);
    }
    if (SS == On){
        S1 = Time.Seconds / 10;
        S2 = Time.Seconds % 10;
        itoa(S1_buffer,S1,10);
        itoa(S2_buffer,S2,10);
        LCD_WriteStringAtPos(S2_buffer,0,13);
        LCD_WriteStringAtPos(S1_buffer,0,12);
    }
    else {
	  LCD_WriteStringAtPos("  ",0,12);
	} 
}

void HandleLongPush(TimeParameters *Time, TimeSetParameters *TimeSet, bool Increase, long TimeForSetTime) {
	if (TimeSet->FirstLongPushUpdate) {
		UpdateClockByOne(Time, TimeSet, Increase);
		TimeSet->FirstLongPushUpdate = false;
		TimeSet->PushTimeStamp = TimeForSetTime;
	}
    bool InitialGapReached = (TimeForSetTime - TimeSet->PushTimeStamp) >= TIME_SET_INITIAL_GAP;
    bool SpeedGapUpdate = ((TimeForSetTime - TimeSet->PushTimeStamp) % TIME_SET_SPEED_GAP) >= (TIME_SET_SPEED_GAP - OFFSET_FOR_LONG_PUSH) ||
                          ((TimeForSetTime - TimeSet->PushTimeStamp) % TIME_SET_SPEED_GAP) <= OFFSET_FOR_LONG_PUSH;
    if (InitialGapReached && SpeedGapUpdate) {
		UpdateClockByOne(Time, TimeSet, Increase);
	}
}

void UpdateClockByOne(TimeParameters *Time, TimeSetParameters *TimeSet, bool Increase) {
	int AmountToAdd = Increase ? 1 : -1;
	switch (TimeSet->BTNCNumOfPushs) {
	case HH:
		Time->Hours += AmountToAdd;
		Time->Hours = Time->Hours == 24 ? 0 :
			Time->Hours == -1 ? 23 : Time->Hours;
		break;
	case MM:
		Time->Minutes += AmountToAdd;
		Time->Minutes = Time->Minutes == 60 ? 0 :
			Time->Minutes == -1 ? 59 : Time->Minutes;
		break;
	case SS:
		Time->Seconds += AmountToAdd;
		Time->Seconds = Time->Seconds == 60 ? 0 :
			Time->Seconds == -1 ? 59 : Time->Seconds;
		break;
	}
}

void HandleStopper() {
	if (BTN_GetValue(1)) { // when button BTNL is pushed
        while (BTN_GetValue(1)) {} // wait for user to release button
		ResetStopper();
        DisplayStopper();
	}
	if (BTN_GetValue(3)) { // when button BTNR is pushed
        while (BTN_GetValue(3)) {} // wait for user to release button
		Stopper.Active = !Stopper.Active;
	}
    if (Stopper.Active){
        DisplayStopper();
    }
}

void ResetStopper() {
	Stopper.Seconds = 0;
	Stopper.Hundredths = 0;
}

void UpdateStopper() {
	Stopper.Hundredths++;
	Stopper.Hundredths = Stopper.Hundredths == 100 ? 0 : Stopper.Hundredths;

	Stopper.Seconds = Stopper.Hundredths == 0 ? Stopper.Seconds + 1 : Stopper.Seconds;
	Stopper.Seconds = Stopper.Seconds == 100 ? 0 : Stopper.Seconds;
}

void DisplayStopper() {
	int Q1, Q2, S1, S2, SSDNumber;
	S1 = Stopper.Seconds / 10;
	S2 = Stopper.Seconds % 10;
	Q1 = Stopper.Hundredths / 10;
	Q2 = Stopper.Hundredths % 10;
	SSDNumber = Q2 + Q1 * 16 + S2 * 256 + S1 * 4096;
	SSD_WriteDigitsGrouped(SSDNumber, 100);
}