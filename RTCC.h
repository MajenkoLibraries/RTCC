/*

All code is copyright 2011 Majenko Technologies. 
The code is provided freely with no warranty either implicitly or explicity implied. 
You may freely use the code for whatever you wish, but Majenko Technologies will not 
be liable for any damage of any form howsoever caused by the use or misuse of this code.

*/

#ifndef _RTCC_H_INCLUDED
#define _RTCC_H_INCLUDED

#define AL_HALF_SECOND  0x0
#define AL_SECOND       0x1
#define AL_10_SECOND    0x2
#define AL_MINUTE       0x3
#define AL_10_MINUTE    0x4
#define AL_HOUR         0x5
#define AL_DAY          0x6
#define AL_WEEK         0x7
#define AL_MONTH        0x8
#define AL_YEAR         0x9

#define ALARM           0x0
#define SECONDS         0x1

class RTCCClass {
	public:
		void begin();
                unsigned char hours();
                void hours(unsigned char);
                unsigned char minutes();
                void minutes(unsigned char);
                unsigned char seconds();
                void seconds(unsigned char);
                unsigned char year();
                void year(unsigned char);
                unsigned char month();
                void month(unsigned char);
                unsigned char day();
                void day(unsigned char);
                unsigned char dayOfWeek();
                void dayOfWeek(unsigned char);

                void outputEnable();
                void outputEnable(unsigned char);
                void outputDisable();

                void alarmEnable();
                void alarmDisable();
                void chimeEnable();
                void chimeDisable();
                void alarmMask(unsigned char);
                void alarmRepeat(unsigned char);

                void alarmHours(unsigned char);
                void alarmMinutes(unsigned char);
                void alarmSeconds(unsigned char);
                void alarmMonth(unsigned char);
                void alarmDay(unsigned char);
                void alarmDayOfWeek(unsigned char);

                void attachInterrupt(void (*)());

        private:
                void unlock();
                void lock();
                void timeSync();
                void alarmSync();
                unsigned char bcd2dec(unsigned char);
                unsigned char dec2bcd(unsigned char);

};

extern RTCCClass RTCC;

#endif
