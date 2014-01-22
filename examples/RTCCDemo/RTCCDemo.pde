/*

All code is copyright 2011 Majenko Technologies. 
The code is provided freely with no warranty either implicitly 
or explicity implied. You may freely use the code for whatever 
you wish, but Majenko Technologies will not be liable for any 
damage of any form howsoever caused by the use or misuse of 
this code.

*/

#include <RTCC.h>

void setup()
{
  Serial.begin(9600);

  // Initialize the RTCC module
  RTCC.begin();
  
  // Set the time to something sensible
  RTCC.hours(9);
  RTCC.minutes(59);
  RTCC.seconds(0);
  RTCC.year(11);
  RTCC.month(05);
  RTCC.day(9);
  
  // Set the alarm to trigger every second
  RTCC.alarmMask(AL_SECOND);
  RTCC.chimeEnable();
  RTCC.alarmEnable();
  
  // Attach our routine to send the time through the serial port
  RTCC.attachInterrupt(&outputTime);
}

void loop()
{
}

void outputTime()
{
  char time[50];
  
  // Format the time and print it.
  sprintf(time,"%02d/%02d/%02d %02d:%02d:%02d\n",
    RTCC.day(),
    RTCC.month(),
    RTCC.year(),
    RTCC.hours(),
    RTCC.minutes(),
    RTCC.seconds()
  );
  Serial.print(time); 
}
