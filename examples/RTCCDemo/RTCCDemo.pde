/*

All code is copyright 2011 Majenko Technologies. 
The code is provided freely with no warranty either implicitly 
or explicity implied. You may freely use the code for whatever 
you wish, but Majenko Technologies will not be liable for any 
damage of any form howsoever caused by the use or misuse of 
this code.

Adapted to new developments by Christophe Dupriez (2014)

*/

#include <RTCC.h>

//                A B C D  E F G H I J K L M N  O  P Q R S T U V W X Y Z
char months[26]= {4,0,0,12,0,2,0,0,0,1,0,0,3,11,10,0,0,0,9,0,0,0,0,0,0,0};

void setup ( )
{
	Serial.begin ( 57600 ) ;
	delay (4000);
	Serial.println("Compiled " __DATE__ " " __TIME__ "\n");

	// I n i t i a l i z e the RTCC module
	RTCC. begin ( ) ;
	// Set the time to something s e n s i b l e
	RTCCValue rv = RTCC.value();
	char aaStr[9] = __TIME__;
	unsigned char hours = (10*(aaStr[0]-'0'))+aaStr[1]-'0';
	unsigned char minutes = (10*(aaStr[3]-'0'))+aaStr[4]-'0';
	unsigned char seconds = (10*(aaStr[6]-'0'))+aaStr[7]-'0';
	rv.time(hours, minutes, seconds);
	
	char bStr[12]= __DATE__;
	unsigned char year = (10*(bStr[9]-'0'))+bStr[10]-'0';
	unsigned char month = months[bStr[0]-'A'];
	if (month == 4 && bStr[1] == 'u') {
		month = 8;
	}
	if (month == 3 && bStr[2] != 'r') {
		month = 5;
	}
	if (month == 1 && bStr[1] == 'u') {
		if (bStr[2] == 'n') month = 6;
		else month = 7;
	}
	if (bStr[4] == ' ') bStr[4] = '0';
	unsigned char day = (10*(bStr[4]-'0'))+bStr[5]-'0';
	rv.date(year, month, day);

	rv.setValidity(RTCC_VAL_GCC); // date/time approximated using compilation time: works for developers only!!!
	RTCC.set(rv);

	rv.seconds(0); // Align alarms to the minute
	RTCC.alarmSet(rv);
	
	// Set the alarm to t r i g g e r every second
	RTCC.alarmMask (AL_10_SECOND) ;
	RTCC.chimeEnable ( ) ;
	RTCC.alarmEnable ( ) ;
	// Attach our routine to send the time through the serial port
	RTCC.attachInterrupt (&outputTime ) ;
}

void loop ( )
{
}

RTCCValue lastChime = RTCCValue();

void outputTime ( )
{	
    // Remember last chime: most useful action of an interrupt routine like this one...
	RTCCValue lastChime = RTCC.value();

	char time [ 50 ] ;
	// Format the time and print it .
	sprintf ( time ,"%02d/%02d/%02d %02d:%02d:%02d\r" ,
			lastChime.day ( ) ,
			lastChime.month ( ) ,
			lastChime.year ( ) ,
			lastChime.hours ( ) ,
			lastChime.minutes ( ) ,
			lastChime.seconds ( )
		) ;
	Serial.print ( time ) ;
}
