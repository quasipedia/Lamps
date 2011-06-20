/************************************************************************
	ds1302.h

    AVR DS1302 RTC Driver Library
    Copyright (C) 2011 Simon Inns

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Email: simon.inns@gmail.com

************************************************************************/

#ifndef DS1302_H_
#define DS1302_H_

// DS1302 Commands (all WRITE, add 1 for READ)
#define	SECONDS	0x80
#define MINUTES	0x82
#define HOURS	0x84
#define DATE	0x86
#define MONTH	0x88
#define	DAY		0x8A
#define YEAR	0x8C

// WP = Write Protect
#define WP		0x8E

// TCS = Trickle Charge Set
#define TCS		0x90

#define CLOCK_BURST 0xBE
#define RAM_BURST	0xFE

#define RTC_READ	1
#define RTC_WRITE	0

#define US_DELAY	20

// Hardware mapping for the DS1302 Real-time clock
#define	RTC_SCLK_PORT	PORTD
#define	RTC_SCLK_PIN	0
#define RTC_IO_PORT		PORTD
#define RTC_IO_PIN		1
#define RTC_IO_INP		PIND
#define RTC_CE_PORT		PORTD
#define RTC_CE_PIN		2
	
// RTC IO pin port direction
#define RTC_IO_DIR_PORT	DDRD
#define RTC_IO_DIR_PIN	1
	
// Define a global structure for passing the time/date
struct datetimeStruct {
	unsigned int hours;	// 0-23
	unsigned int minutes;	// 0-59
	unsigned int seconds;	// 0-59
	
	unsigned int year;		// 0-99
	unsigned int month;	// 1-12
	unsigned int day;		// 1-31
		
	unsigned int dayNo;	// 1-7
	
	//unsigned char wp;		// Write protect bit
} datetime;
	
// Definitions for EEPROM storage
#define TIMEREAD	0
#define TIMEWRITE	1
	
// Define flags for the clock status
#define CLOCK_UNSET	0
#define CLOCK_SET	1

// function prototypes
void sendByteRTC(unsigned int byte);
unsigned int receiveByteRTC(void);
void initialiseRTC(void);
void setRTC(void);
void readRTC(void);
unsigned int readClockStatus(void);
void writeClockStatus(unsigned int status);

#endif /* DS1302_H_ */