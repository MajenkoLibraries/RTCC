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

class RTCCValue {
	private:
		uint32_t a_date;
		uint32_t a_time;
		unsigned char bcd2dec(unsigned char);
		unsigned char dec2bcd(unsigned char);
		void bcd2str(unsigned char * res, unsigned char bcd);
	public:
		RTCCValue(uint32_t date, uint32_t time);
		uint32_t date();
		uint32_t time();
		bool valid();
		void time(unsigned char hours, unsigned char minutes, unsigned char seconds);
		void time(unsigned char * res);
		void date(unsigned char year, unsigned char month, unsigned char day);
		void date(unsigned char * res);
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
};

class RTCCClass {
	public:
		void begin();
		RTCCValue value();
		void set(RTCCValue val);

		void outputEnable();
		void outputEnable(unsigned char);
		void outputDisable();

		RTCCValue alarmValue();
		void alarmEnable();
		void alarmDisable();
		void chimeEnable();
		void chimeDisable();
		void alarmMask(unsigned char);
		void alarmRepeat(unsigned char);

		void alarmSet(RTCCValue val);
		void attachInterrupt(void (*)());

	private:
		void timeSync();
		void alarmSync();
};

extern RTCCClass RTCC;

#endif
