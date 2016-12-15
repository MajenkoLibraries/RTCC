/*

All code is copyright 2011 Majenko Technologies. 
The code is provided freely with no warranty either implicitly or explicity implied. 
You may freely use the code for whatever you wish, but Majenko Technologies will not 
be liable for any damage of any form howsoever caused by the use or misuse of this code.

*/

// BEWARE: A 32khz Crystal and capacitors must be installed and SOSCON must be enabled by your bootloader!

#ifndef _RTCC_H_INCLUDED
#define _RTCC_H_INCLUDED

// Alarm and Chimes intervals
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

// Increasing quality of time set in RTCC
// Unitialized
#define RTCC_VAL_NOT 0
// Initialized with compilation date/time
#define RTCC_VAL_GCC 1
// Initialized with date/time received from another computer
#define RTCC_VAL_BEE 2
// Initialized by manual configuration
#define RTCC_VAL_MAN 3
// Initialized from GSM network
#define RTCC_VAL_GSM 4
// Initialized from a NTP server
#define RTCC_VAL_NTP 5

/*
 This class represents a date/time and its validity
 */
class RTCCValue {
	private:
		char _validity;
		uint32_t a_date;
		uint32_t a_time;
		unsigned char bcd2dec(unsigned char);
		unsigned char dec2bcd(unsigned char);
		void bcd2str(unsigned char * res, unsigned char bcd);
	public:
		RTCCValue();
		RTCCValue(uint32_t date, uint32_t time, uint8_t val);
		RTCCValue(const RTCCValue * rv);
		void setValidity(char val);
		char getValidity();

		uint32_t date();
		uint32_t time();
		
		// Check if data in BCD minimally make sense
		// (detect date/time from uninitialised clock)
		bool valid();

		// Set time
		void time(unsigned char hours, unsigned char minutes, unsigned char seconds);
		// Show time hh:mm:ss
		void time(unsigned char * res);
		// Set date jj/mm/yy
		void date(unsigned char year, unsigned char month, unsigned char day);
		// Show date
		void date(unsigned char * res);
		
		// returns 0 to 23
		unsigned char hours();
		// Set hour to 0 to 23
		void hours(unsigned char);
		// returns 0 to 59
		unsigned char minutes();
		// Set minutes
		void minutes(unsigned char);
		// returns 0 to 59
		unsigned char seconds();
		// Set seconds
		void seconds(unsigned char);
		
		// BEWARE 2100 !!!!
		// returns 0 to 99 for 2000 to 2099
		unsigned char year();
		// Set year (0 to 99 for 2000 to 2099)
		void year(unsigned char);
		
		// returns 1 to 12
		unsigned char month();
		// Set month
		void month(unsigned char);
        // returns 1 to 31
		unsigned char day();
		void day(unsigned char);

		unsigned char dayOfWeek();
		void dayOfWeek(unsigned char);

        // Very handy for teletransmission or storage: get an UNSIGNED 32 bits representation of date/time
		uint32_t getInt();
		// Set date/time from UNSIGNED 32 bits
		void setInt(uint32_t timeAsInt);

/* Java code to encode/decode such a date/time into a Java Date class:
public class RTCvalue extends Date {
	@SuppressWarnings("deprecation")
	public void setInt(int data) {
		this.setTime((long)0); // ensures 0 millis

		// Java does not know about unsigned int!
		int datePart = (int)((data >> 1) / (30*60*24));
		this.setDate((datePart % 31)+1) ;
		datePart = datePart / 31;
		this.setMonth(datePart % 12);
		this.setYear((datePart / 12)+100);
		
		datePart = data % (60*60*24);
		if (datePart < 0) datePart += 60*60*24;
		this.setSeconds(datePart % 60);
		datePart = datePart / 60;
		this.setMinutes(datePart % 60);
		this.setHours(datePart / 60);
	}
	
	@SuppressWarnings("deprecation")
	public int getInt() {
		if (this.getYear()<2000) return 0;
		return this.getSeconds()
				+(this.getMinutes()*60)
				+(this.getHours()*3600)
				+(this.getDate()*3600*24)
				+(this.getMonth()*3600*24*31)
				+((this.getYear()-2000)*3600*24*31*12);
	}
}
*/

	    // Still UNTESTED: create Unix like long representing the date/time
		uint32_t getTimestamp();
		
		// NOT IMPLEMENTED BUT WOULD BE NICE !
		void setTimestamp(uint32_t unixTime);
};

class RTCCClass {
	public:
		void begin();
		RTCCValue value();
		void set(RTCCValue setting);

		void setValidity(char val);
		char getValidity();

		void outputEnable();
		void outputEnable(unsigned char);
		void outputDisable();

        void calibrate(int cal);

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
		char _validity;
		void timeSync();
		void alarmSync();
};

extern RTCCClass RTCC;

#endif
