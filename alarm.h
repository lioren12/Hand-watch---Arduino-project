#ifndef ALARM_H
#define ALARM_H

#include <stdbool.h>

#define _XTAL_FREQ 8000000
#define NC 523
#define ND 587
#define NE 659
#define NF 698
#define NG 784
#define NA 880
#define NB 987
#define NC2 1046
#define SONG_LENGTH 30

typedef struct _AlarmParameters {
	int CurrentLEDToDisplay;
	bool NeedToStartAlarm;
	bool InMiddleOfAlarm;
	long StartAlarmTimeStamp;
	bool TurningLEDsFromLowToHigh;
    bool FirstPlayFlag;
} AlarmParameters;

AlarmParameters Alarm;


void InitAlarm();
bool CheckIfNeedToStartAlarm();
void HandleAlarm(long CurrentTime);

unsigned int AudioTimestamp = 0;
int Song[SONG_LENGTH] = {NC, ND, NE, NG, NG, NA, NG, NE, NC, ND, NE, NE, ND, NC, ND,
                NC, ND, NE, NG, NG, NA, NG, NE, NC, ND, NE, NE, ND, NC, NC};
int Duration[SONG_LENGTH] = {1, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 2, 2, 6, 
                    1, 1, 2, 2, 3, 1, 2, 2, 3, 1,2, 2, 2, 2, 6};

unsigned int CurrentTimeAudio;
int NoteIndex=0;
int DurationCount=0;

#endif