/*

All code is copyright 2011 Majenko Technologies. 
The code is provided freely with no warranty either implicitly or explicity implied. 
You may freely use the code for whatever you wish, but Majenko Technologies will not 
be liable for any damage of any form howsoever caused by the use or misuse of this code.

*/

#include <WProgram.h>
#include <sys/attribs.h>
#include "RTCC.h"

RTCCClass RTCC;

extern "C"
{
    void (*_RTCCInterrupt)();

    void __ISR(_RTCC_VECTOR, _RTCC_IPL_ISR) __RTCCInterrupt(void) 
    {
        if(_RTCCInterrupt)
        {
            _RTCCInterrupt();
        }
        IFS1CLR=0x00008000;
    }
}

void RTCCClass::begin() 
{
    IEC1CLR=0x00008000;
    IFS1CLR=0x00008000; 
    IPC8CLR=0x1f000000; 
    IPC8SET=0x0d000000; 
    IEC1SET=0x00008000; 

    unlock();
    OSCCONSET = 2;
    RTCCONbits.ON = 1;
    RTCCONbits.RTCWREN = 1;
    lock();
}

void RTCCClass::unlock()
{
    SYSKEY = 0x0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
}

void RTCCClass::lock()
{
    SYSKEY = 0x0;
}

void RTCCClass::timeSync()
{
    while(RTCCONbits.RTCSYNC==1);
}

void RTCCClass::outputEnable()
{
    timeSync();
    unlock();
    RTCCONbits.RTCOE = 1;
    RTCCONbits.RTSECSEL = 1;
    lock();
} 

void RTCCClass::outputEnable(unsigned char mode)
{
    timeSync();
    unlock();
    RTCCONbits.RTCOE = 1;
    RTCCONbits.RTSECSEL = mode;
    lock();
} 

void RTCCClass::outputDisable()
{
    timeSync();
    unlock();
    RTCCONbits.RTCOE = 0;
    lock();
} 

unsigned char RTCCClass::hours()
{
    timeSync();
    return bcd2dec((RTCTIME >> 24) & 0xFF);
}

void RTCCClass::hours(unsigned char hours)
{
    timeSync();
    RTCTIME = (RTCTIME & 0x00FFFFFF) | (dec2bcd(hours)<<24);
}

unsigned char RTCCClass::minutes()
{
    timeSync();
    return bcd2dec((RTCTIME >> 16) & 0xFF);
}

void RTCCClass::minutes(unsigned char minutes)
{
    timeSync();
    RTCTIME = (RTCTIME & 0xFF00FFFF) | (dec2bcd(minutes)<<16);
}

unsigned char RTCCClass::seconds()
{
    timeSync();
    return bcd2dec((RTCTIME >> 8) & 0xFF);
}

void RTCCClass::seconds(unsigned char seconds)
{
    timeSync();
    RTCTIME = (RTCTIME & 0xFFFF00FF) | (dec2bcd(seconds)<<8);
}

unsigned char RTCCClass::year()
{
    timeSync();
    return bcd2dec((RTCDATE >> 24) & 0xFF);
}

void RTCCClass::year(unsigned char year)
{
    timeSync();
    RTCDATE = (RTCDATE & 0x00FFFFFF) | (dec2bcd(year)<<24);
}

unsigned char RTCCClass::month()
{
    timeSync();
    return bcd2dec((RTCDATE >> 16) & 0xFF);
}

void RTCCClass::month(unsigned char month)
{
    timeSync();
    RTCDATE = (RTCDATE & 0xFF00FFFF) | (dec2bcd(month)<<16);
}

unsigned char RTCCClass::day()
{
    timeSync();
    return bcd2dec((RTCDATE >> 8) & 0xFF);
}

void RTCCClass::day(unsigned char day)
{
    timeSync();
    RTCDATE = (RTCDATE & 0xFFFF00FF) | (dec2bcd(day)<<8);
}

unsigned char RTCCClass::dayOfWeek()
{
    timeSync();
    return bcd2dec(RTCDATE & 0xFF);
}

void RTCCClass::dayOfWeek(unsigned char dow)
{
    timeSync();
    RTCDATE = (RTCDATE & 0xFFFFFF00) | dec2bcd(dow);
}

void RTCCClass::alarmEnable()
{
    alarmSync();
    RTCALRMbits.ALRMEN = 1;
}

void RTCCClass::alarmDisable()
{
    alarmSync();
    RTCALRMbits.ALRMEN = 0;
}

void RTCCClass::chimeEnable()
{
    alarmSync();
    RTCALRMbits.CHIME = 1;
}

void RTCCClass::chimeDisable()
{
    alarmSync();
    RTCALRMbits.CHIME = 0;
}

void RTCCClass::alarmMask(unsigned char mask)
{
    alarmSync();
    RTCALRMbits.AMASK = mask;
}

void RTCCClass::alarmRepeat(unsigned char rpt)
{
    alarmSync();
    RTCALRMbits.ARPT = rpt;
}

void RTCCClass::alarmSync()
{
    while(RTCALRMbits.ALRMSYNC==1);
}

unsigned char RTCCClass::dec2bcd(unsigned char decimal)
{
    unsigned char bcd;
    unsigned char tens,units;
    tens = decimal / 10;
    units = decimal - (tens * 10);
    bcd = (tens << 4) | units;
    return bcd;
}

unsigned char RTCCClass::bcd2dec(unsigned char bcd)
{
    unsigned char decimal;
    unsigned char tens,units;
    tens = bcd >> 4;
    units = bcd & 0x0F;
    decimal = (tens * 10) + units;
    return decimal;
}

void RTCCClass::alarmHours(unsigned char hours)
{
    alarmSync();
    ALRMTIME = (ALRMTIME & 0x00FFFFFF) | (dec2bcd(hours)<<24);
}

void RTCCClass::alarmMinutes(unsigned char minutes)
{
    alarmSync();
    ALRMTIME = (ALRMTIME & 0xFF00FFFF) | (dec2bcd(minutes)<<16);
}

void RTCCClass::alarmSeconds(unsigned char seconds)
{
    alarmSync();
    ALRMTIME = (ALRMTIME & 0xFFFF00FF) | (dec2bcd(seconds)<<8);
}

void RTCCClass::alarmMonth(unsigned char month)
{
    alarmSync();
    ALRMDATE = (ALRMDATE & 0xFF00FFFF) | (dec2bcd(month)<<16);
}

void RTCCClass::alarmDay(unsigned char day)
{
    alarmSync();
    ALRMDATE = (ALRMDATE & 0xFFFF00FF) | (dec2bcd(day)<<8);
}

void RTCCClass::alarmDayOfWeek(unsigned char dow)
{
    alarmSync();
    ALRMDATE = (ALRMDATE & 0xFFFFFF00) | dec2bcd(dow);
}

void RTCCClass::attachInterrupt(void (*i)())
{
    _RTCCInterrupt = i;
}


