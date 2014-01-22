/*
 * Copyright (c) 2013, Majenko Technologies
 * All rights reserved.
 *
 * Additions (c) 2014, Christophe Dupriez, DESTIN-Informatique.com
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <WProgram.h>
#include <sys/attribs.h>
#include <peripheral/int.h>
#include "RTCC.h"

RTCCClass RTCC; // Object to access the RTC of the MCU...

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

#define UNLOCK SYSKEY = 0x0; SYSKEY = 0xAA996655; SYSKEY = 0x556699AA;
#define LOCK SYSKEY = 0x0;

void RTCCClass::begin() 
{
    IEC1CLR=0x00008000;
    IFS1CLR=0x00008000; 
    IPC8CLR=0x1f000000; 
    IPC8SET=0x0d000000; 
    IEC1SET=0x00008000; 
/*Ensure the secondary oscillator is enabled and ready, i.e. OSCCON<1>=1, OSCCON<22>=1,
and RTCC write is enabled i.e. RTCWREN (RTCCON<3>) =1;*/
    UNLOCK
    RTCCONbits.RTCWREN = 1;
    RTCCONbits.ON = 1;
    OSCCONSET = 0x00400002;
    LOCK
}


/*
 * Wait at least 32msec... if a crystal is available!
 * FOREVER IF NO CRYSTAL WORKING...
 */
void RTCCClass::timeSync()
{
/* PROBABLY NOT A GOOD IDEA!
    while(RTCCONbits.RTCSYNC==0); // Wait for a transition and 32ms later
*/
    while(RTCCONbits.RTCSYNC==1) { /*yield*/ };
}

void RTCCClass::outputEnable()
{
    timeSync();
    UNLOCK
    RTCCONbits.RTCOE = 1;
    RTCCONbits.RTSECSEL = 1;
    LOCK
} 

void RTCCClass::outputEnable(unsigned char mode)
{
    timeSync();
    UNLOCK
    RTCCONbits.RTCOE = 1;
    RTCCONbits.RTSECSEL = mode;
    LOCK
} 

void RTCCClass::outputDisable()
{
    timeSync();
    UNLOCK
    RTCCONbits.RTCOE = 0;
    LOCK
} 

RTCCValue RTCCClass::value()
{
    timeSync();
	uint32_t our_date = RTCDATE;
	uint32_t our_time = RTCTIME;
	while (1) {
		uint32_t new_date = RTCDATE;
		uint32_t new_time = RTCTIME;
		if (new_date == our_date && new_time == our_time) break;
		our_date = new_date;
		our_time = new_time;
	}
	return RTCCValue(our_date,our_time);
}

void RTCCClass::set(RTCCValue val)
{
    timeSync();
	RTCDATE = val.date();
	RTCTIME = val.time();
}

void RTCCClass::alarmSync()
{
    while(RTCALRMbits.ALRMSYNC==1) { /*yield*/ };
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

void RTCCClass::alarmSet(RTCCValue val) {
	alarmSync();
	ALRMDATE = val.date();
	ALRMTIME = val.time();
}

RTCCValue RTCCClass::alarmValue() {
	alarmSync();
	return RTCCValue(ALRMDATE,ALRMTIME);
}

void RTCCClass::attachInterrupt(void (*i)())
{
    _RTCCInterrupt = i;
}

/* Handling a date/time : current RTC, Alarm, logged event, etc. */
RTCCValue::RTCCValue(uint32_t date, uint32_t time) {
	a_date = date;
	a_time = time;
}

unsigned char RTCCValue::dec2bcd(unsigned char decimal)
{
    unsigned char bcd;
    unsigned char tens,units;
    tens = decimal / 10;
    units = decimal - (tens * 10);
    bcd = (tens << 4) | units;
    return bcd;
}

unsigned char RTCCValue::bcd2dec(unsigned char bcd)
{
    unsigned char decimal;
    unsigned char tens,units;
    tens = bcd >> 4;
    units = bcd & 0x0F;
    decimal = (tens * 10) + units;
    return decimal;
}

void RTCCValue::bcd2str(unsigned char * res, unsigned char bcd)
{
    unsigned char tens,units;
    tens = bcd >> 4;
    units = bcd & 0x0F;
    res[0] = tens +'0';
	res[1] = units+ '0';
}

uint32_t RTCCValue::date()
{
	return a_date;
}

uint32_t RTCCValue::time()
{
	return a_time;
}

// sizeof res must be 9 or bigger
void RTCCValue::date(unsigned char * res)
{
	unsigned char ss[2];
	bcd2str(ss, (a_date >> 24) & 0xFF);
	res[0] = ss[0];
	res[1] = ss[1];
	res[2] = '-';
	bcd2str(ss, (a_date >> 16) & 0xFF);
	res[3] = ss[0];
	res[4] = ss[1];
	res[5] = '-';
	bcd2str(ss, (a_date >> 8) & 0xFF);
	res[6] = ss[0];
	res[7] = ss[1];
	res[8] = 0;
}

// sizeof res must be 9 or bigger
void RTCCValue::time(unsigned char * res)
{
	unsigned char ss[2];
	bcd2str(ss, (a_time >> 24) & 0xFF);
	res[0] = ss[0];
	res[1] = ss[1];
	res[2] = ':';
	bcd2str(ss, (a_time >> 16) & 0xFF);
	res[3] = ss[0];
	res[4] = ss[1];
	res[5] = ':';
	bcd2str(ss, (a_time >> 8) & 0xFF);
	res[6] = ss[0];
	res[7] = ss[1];
	res[8] = 0;
}

unsigned char RTCCValue::year()
{
    return bcd2dec((a_date >> 24) & 0xFF);
}

unsigned char RTCCValue::month()
{
    return bcd2dec((a_date >> 16) & 0xFF);
}

unsigned char RTCCValue::day()
{
    return bcd2dec((a_date >> 8) & 0xFF);
}

unsigned char RTCCValue::dayOfWeek()
{
    return bcd2dec(a_date & 0xFF);
}

unsigned char RTCCValue::hours()
{
    return bcd2dec((a_time >> 24) & 0xFF);
}


unsigned char RTCCValue::minutes()
{
    return bcd2dec((a_time >> 16) & 0xFF);
}

unsigned char RTCCValue::seconds()
{
    return bcd2dec((a_time >> 8) & 0xFF);
}

void RTCCValue::time(unsigned char hours, unsigned char minutes, unsigned char seconds)
{
    a_time = (a_time & 0xFF) | (dec2bcd(hours)<<24) | (dec2bcd(minutes)<<16) | (dec2bcd(seconds)<<8);
}

void RTCCValue::hours(unsigned char hours)
{
    a_time = (a_time & 0x00FFFFFF) | (dec2bcd(hours)<<24);
}
void RTCCValue::minutes(unsigned char minutes)
{
    a_time = (a_time & 0xFF00FFFF) | (dec2bcd(minutes)<<16);
}

void RTCCValue::seconds(unsigned char seconds)
{
    a_time = (a_time & 0xFFFF00FF) | (dec2bcd(seconds)<<8);
}

void RTCCValue::date(unsigned char year, unsigned char month, unsigned char day)
{
    a_date = (a_date & 0xFF) | (dec2bcd(year)<<24) | (dec2bcd(month)<<16) | (dec2bcd(day)<<8);
}

void RTCCValue::year(unsigned char year)
{
    a_date = (a_date & 0x00FFFFFF) | (dec2bcd(year)<<24);
}

void RTCCValue::month(unsigned char month)
{
    a_date = (a_date & 0xFF00FFFF) | (dec2bcd(month)<<16);
}

void RTCCValue::day(unsigned char day)
{
    a_date = (a_date & 0xFFFF00FF) | (dec2bcd(day)<<8);
}

void RTCCValue::dayOfWeek(unsigned char dow)
{
    a_date = (a_date & 0xFFFFFF00) | dec2bcd(dow);
}

bool RTCCValue::valid()
{
   char ayear  = year();
   char amonth = month();
   char aday   = day();
   char ahour  = hours();
   char amin   = minutes();
   char asec   = seconds();
   return (ayear >= 0 && ayear <= 99 && amonth >= 1 && amonth <= 12 && aday >= 1 && aday <= 31
           && ahour >= 0 && ahour <= 23 && amin >= 0 && amin <= 59 && asec >= 0 && asec <= 59 );
}
