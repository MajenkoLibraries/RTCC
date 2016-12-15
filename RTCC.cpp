/*

All code is copyright 2011 Majenko Technologies. 
The code is provided freely with no warranty either implicitly or explicity implied. 
You may freely use the code for whatever you wish, but Majenko Technologies will not 
be liable for any damage of any form howsoever caused by the use or misuse of this code.

Separation of date/time Class (RTCCValue) from RTC management object (RTCCClass) by Christophe Dupriez, DESTIN-Informatique.com 2014

PIC32 version...

*/

#include <WProgram.h>
#include <sys/attribs.h>
#include "RTCC.h"

RTCCClass RTCC; // Object to access the RTC of the MCU...

/*
  Interrupt routine called if Alarm/Chime/RTC interrupt is enabled.
  Calls an user configured procedure.
*/
void (*_RTCCInterrupt)();

void __USER_ISR __RTCCInterrupt(void) {
    if(_RTCCInterrupt) {
        _RTCCInterrupt();
    }
    clearIntFlag(_RTCC_IRQ);
}

/*
 Configuration registries unlock/lock sequence...
 */
#define UNLOCK SYSKEY = 0x0; SYSKEY = 0xAA996655; SYSKEY = 0x556699AA;
#define LOCK SYSKEY = 0x0;

void RTCCClass::begin() 
{
    setIntVector(_RTCC_VECTOR, __RTCCInterrupt);
    setIntPriority(_RTCC_VECTOR, 3, 0);
    clearIntFlag(_RTCC_IRQ);
    setIntEnable(_RTCC_IRQ);

/*Ensure the secondary oscillator is enabled and ready, i.e. OSCCON<1>=1, OSCCON<22>=1,
and RTCC write is enabled i.e. RTCWREN (RTCCON<3>) =1;*/
    UNLOCK
    RTCCONbits.RTCWREN = 1;
    RTCCONbits.ON = 1;
    OSCCONSET = 0x00400002;
    LOCK
	_validity = RTCC_VAL_NOT; // 0: very unsure!
}


/*
 * Wait at least 32msec... if a crystal is available!
 * FOREVER IF NO WORKING CRYSTAL?
 */
void RTCCClass::timeSync()
{
/* THIS IS NOT A GOOD IDEA:
    while(RTCCONbits.RTCSYNC==0); // Wait for a transition and 32ms later
*/
    while(RTCCONbits.RTCSYNC==1) { /*yield*/ };
}

/*
 Enable/Disable an output pin synchronised with RTC
 */
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

void RTCCClass::calibrate(int cal) {
    RTCCONbits.CAL = cal;
}

/*
 get Current date/time.
 RTCCValue class represents a date/time independently from the RTC itself.
 */
RTCCValue RTCCClass::value()
{
    timeSync();
	uint32_t our_date = RTCDATE;
	uint32_t our_time = RTCTIME;
	while (1) { // Ensures there is no date rollover while reading the time
		uint32_t new_date = RTCDATE;
		uint32_t new_time = RTCTIME;
		if (new_date == our_date && new_time == our_time) break;
		our_date = new_date;
		our_time = new_time;
	}
	return RTCCValue(our_date,our_time, _validity);
}

/*
 Set date/time and specify its validity (0: very bad, 5: perfect=NTP)
 */
void RTCCClass::set(RTCCValue setting)
{
    timeSync();
	RTCDATE = setting.date();
	RTCTIME = setting.time();
	_validity = setting.getValidity();
}

/*
 Alarms management...
 */
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

/*
 Chime is an alarm repeating at configured interval (see AL_xxx definitions)
 
 Usage example:
    // Set current time with an RTCCValue "rv"
 	RTCC.set(rv);
	
	// Make an Alarm basis synched to Hours
	rv.seconds(0);
	rv.minutes(0);
	RTCC.alarmSet(rv);

	// Setup a Chime every minute
    RTCC.alarmMask(CHIME_PERIOD);
	RTCC.chimeEnable();
    RTCC.attachInterrupt(&JustWakeUp); // JustWakeUp is a void function() to handle RTC interrupts
	RTCC.alarmEnable();

	//Example of JustWakeUp:
	void JustWakeUp(void)
	{
		// Remember when this occurs!
		lastChime = RTCC.value();
	}
 */
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

/*
 Alarm value is necessary for chimes also as it defined the basis of the repeating chime
 */
RTCCValue RTCCClass::alarmValue() {
	alarmSync();
	return RTCCValue(ALRMDATE,ALRMTIME,0);
}

void RTCCClass::attachInterrupt(void (*i)())
{
    _RTCCInterrupt = i;
}
/*
 Validity is useful to track the source of current date/time reference...
 */
char RTCCClass::getValidity() {
  return _validity;
}

void RTCCClass::setValidity(char val) {
  _validity = val;
}



/* Default constructor */
RTCCValue::RTCCValue() {
	a_date = 0;
	a_time = 0;
	_validity = 0;
}

/* Handling a date/time : current RTC, Alarm, logged event, etc. */
RTCCValue::RTCCValue(uint32_t date, uint32_t time, uint8_t val) {
	a_date = date;
	a_time = time;
	_validity = val;
}

RTCCValue::RTCCValue(const RTCCValue * rv) {
	a_date = rv->a_date;
	a_time = rv->a_time;
	_validity = rv->_validity;
}

/*
 Validity is useful to track the source of current date/time reference...
 */
char RTCCValue::getValidity() {
  return _validity;
}

void RTCCValue::setValidity(char val) {
  _validity = val;
}
/*
 Internally RTC is working in good old BCD!
 */
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

/*
  get an UNSIGNED integer representing date/time in 32 bits (NOT Unix date/time representation)
 */
void RTCCValue::setInt(uint32_t timeAsInt) {
   char asec   = timeAsInt % 60;
   timeAsInt = timeAsInt / 60;
   char amin   = timeAsInt % 60;
   timeAsInt = timeAsInt / 60;
   char ahour   = timeAsInt % 24;
   timeAsInt = timeAsInt / 24;
   char aday   = (timeAsInt % 31) + 1;
   timeAsInt = timeAsInt / 31;
   char amonth   = (timeAsInt % 12) + 1;
   char ayear = timeAsInt / 12;
   date (ayear,amonth,aday);
   time (ahour, amin,asec);
}

/*
  Set a date/time using an UNSIGNED integer in 32 bits (NOT Unix date/time representation)
 */
uint32_t RTCCValue::getInt() {
   char ayear  = year();
   char amonth = month();
   char aday   = day();
   char ahour  = hours();
   char amin   = minutes();
   char asec   = seconds();
   if (ayear >= 0 && ayear <= 99 && amonth >= 1 && amonth <= 12 && aday >= 1 && aday <= 31
       && ahour >= 0 && ahour <= 23 && amin >= 0 && amin <= 59 && asec >= 0 && asec <= 59 ) {
	return (uint32_t)asec+((uint32_t)amin*60)+((uint32_t)ahour*3600)+(((uint32_t)aday-1)*3600*24)+(((uint32_t)amonth-1)*3600*24*31)+((uint32_t)ayear*3600*24*31*12);
   } else return 0;
}

// Days per month...
unsigned char calendar [] = {31, 28, 31, 30,
							31, 30, 31, 31,
							30, 31, 30, 31};


#define SECS_PER_MIN  ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY  ((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK ((time_t)(7UL))
#define SECS_PER_WEEK ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((time_t)(SECS_PER_WEEK * 52UL))
#define SECS_YR_2000  ((time_t)(946684800UL)) // the time at the start of y2k

#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

/*
 Still UNTESTED: create Unix like long representing the date/time
 */							
uint32_t RTCCValue::getTimestamp()
{
  int i;
  uint32_t tseconds;

    uint32_t yr = year() + 2000 - 1970;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  tseconds= yr*(SECS_PER_DAY * 365);
  for (i = 0; i < yr; i++) {
    if (LEAP_YEAR(i)) {
      tseconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }

  // add days for this year, months start from 1
  for (i = 1; i < month(); i++) {
    if ( (i == 2) && LEAP_YEAR(yr)) {
      tseconds += SECS_PER_DAY * 29;
    } else {
      tseconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  tseconds+= (day()-1) * SECS_PER_DAY;
  tseconds+= hours() * SECS_PER_HOUR;
  tseconds+= minutes() * SECS_PER_MIN;
  tseconds+= seconds();
  return tseconds;
}

// NOT IMPLEMENTED BUT WOULD BE NICE !
void RTCCValue::setTimestamp(uint32_t unixTime) {
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t _year;
  uint8_t _month, _monthLength;
  uint32_t _time;
  unsigned long _days;

  _time = (uint32_t)unixTime;
  seconds(_time % 60);
  _time /= 60; // now it is minutes
  minutes(_time % 60);
  _time /= 60; // now it is hours
  hours(_time % 24);
  _time /= 24; // now it is days
  dayOfWeek(((_time + 4) % 7) + 1);  // Sunday is day 1

  _year = 0;
  _days = 0;
  while((unsigned)(_days += (LEAP_YEAR(_year) ? 366 : 365)) <= _time) {
    _year++;
  }
  year((1970 + _year) % 100); // year is offset from 1970

  _days -= LEAP_YEAR(_year) ? 366 : 365;
  _time  -= _days; // now it is days in this year, starting at 0

  _days=0;
  _month=0;
  _monthLength=0;
  for (_month=0; _month<12; _month++) {
    if (_month==1) { // february
      if (LEAP_YEAR(_year)) {
        _monthLength=29;
      } else {
        _monthLength=28;
      }
    } else {
      _monthLength = monthDays[_month];
    }

    if (_time >= _monthLength) {
      _time -= _monthLength;
    } else {
        break;
    }
  }
  month(_month + 1);  // jan is month 1
  day(_time + 1);     // day of month

}
