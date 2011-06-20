/************************************************************************
	ds1302.c

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

// Includes
#include <avr/io.h>
#include "hardware.h"
#include "ds1302.h"
#include <util/delay.h>

// Global for clock status
unsigned int clockStatus = CLOCK_UNSET;

// Send a byte to the RTC LSB first
void sendByteRTC(unsigned int byte)
{
	// Set IO (PD1) to output
	sbi(RTC_IO_PORT, RTC_IO_PIN); // Disable pull up by clearing PORT
	sbi(RTC_IO_DIR_PORT, RTC_IO_DIR_PIN); // Configure as output by setting DDR to 1
	_delay_us(US_DELAY);

	for(unsigned int bitNo = 0; bitNo < 8; bitNo++)
	{
		// SCLK low
		cbi(RTC_SCLK_PORT, RTC_SCLK_PIN);
		_delay_us(US_DELAY);
		
		// Set the bit on the pin
		if(byte & 0x01) sbi(RTC_IO_PORT, RTC_IO_PIN);
		else cbi(RTC_IO_PORT, RTC_IO_PIN);
		
		// SCLK high
		_delay_us(US_DELAY);
		sbi(RTC_SCLK_PORT, RTC_SCLK_PIN);
		_delay_us(US_DELAY);
		byte >>= 1;
	}
	
	// Clear the IO pin
	cbi(RTC_IO_PORT, RTC_IO_PIN);
}

// Receive a byte from the RTC LSB first
unsigned int receiveByteRTC(void)
{
	unsigned int byte = 0;

	// Set IO (PD1) to input
	cbi(RTC_IO_PORT, RTC_IO_PIN); // Disable pull up by clearing PORT
	cbi(RTC_IO_DIR_PORT, RTC_IO_DIR_PIN); // Configure as input by setting DDR to 0
	_delay_us(US_DELAY);

	for (unsigned int bitNo = 0; bitNo < 8; bitNo++)
	{
		// Strobe the serial clock
		sbi(RTC_SCLK_PORT, RTC_SCLK_PIN);
		_delay_us(US_DELAY);
		cbi(RTC_SCLK_PORT, RTC_SCLK_PIN);
		_delay_us(US_DELAY);
		
		// Read the pin status and write the bit into the byte
		if (RTC_IO_INP & (1 << RTC_IO_PIN)) byte |= 0x01 << bitNo;
	}

	return byte;
}

// Send a command word to the RTC
void sendCommandRTC(unsigned int command, unsigned int data)
{
	// Chip enable high
	sbi(RTC_CE_PORT, RTC_CE_PIN);
	_delay_us(US_DELAY);
	
	sendByteRTC(command);
	sendByteRTC(data);
	
	// Chip enable low
	cbi(RTC_CE_PORT, RTC_CE_PIN);
	_delay_us(US_DELAY);
}	

// Send a command and receive a byte long response from the RTC
unsigned int receiveCommandRTC(unsigned int command)
{
	// Chip enable high
	sbi(RTC_CE_PORT, RTC_CE_PIN);
	_delay_us(US_DELAY);
	
	sendByteRTC(command + RTC_READ);
	unsigned int result = receiveByteRTC();
	
	// Chip enable low
	cbi(RTC_CE_PORT, RTC_CE_PIN);
	_delay_us(US_DELAY);
	
	return result;
}	

void initialiseRTC(void)
{
	// Ensure all the required PINs are in the correct state
	cbi(RTC_CE_PORT, RTC_CE_PIN);
	cbi(RTC_SCLK_PORT, RTC_SCLK_PIN);
	cbi(RTC_IO_PORT, RTC_IO_PIN);
	
	// Power up delay of 20 uS (as per the datasheet)
	_delay_us(20);
	
	// Disable the write protection
	sendCommandRTC(WP, 0x00);
	
	// Setting trickle charge to 2.40mA@5.5V (2Kohms, 1 diode)
	// This gives a charge time of 4.7 minutes (with a 0.047F cap)
	// and a discharge to 2Vs time of 95.2 hours
	// sendCommandRTC(TCS, 0xA5);
	
	// If the clock is not running (i.e. this is the first start
	// or the battery backup has run out) set the clock halt
	// flag to FALSE to start the RTC
	unsigned int x = receiveCommandRTC(SECONDS);
	if ((x & SECONDS) != 0)
	{
		sendCommandRTC(SECONDS, 0x00);
		clockStatus = CLOCK_UNSET;
	}
	else clockStatus = CLOCK_SET;
}

// Use a write burst to set the time and date
void setRTC(void)
{
	// Chip enable high
	sbi(RTC_CE_PORT, RTC_CE_PIN);
	_delay_us(US_DELAY);
	
	// Request a clock write burst
	sendByteRTC(CLOCK_BURST + RTC_WRITE);

	// Write the seconds (converting decimal to BCD)
	sendByteRTC(((datetime.seconds/10)<<4)+(datetime.seconds%10));

	// Write the minutes (converting decimal to BCD)
	sendByteRTC(((datetime.minutes/10)<<4)+(datetime.minutes%10));

	// Write the hours (converting decimal to BCD)
	sendByteRTC(((datetime.hours/10)<<4)+(datetime.hours%10));

	// Write the day/date (converting decimal to BCD)
	sendByteRTC(((datetime.day/10)<<4)+(datetime.day%10));

	// Write the month (converting decimal to BCD)
	sendByteRTC(((datetime.month/10)<<4)+(datetime.month%10));

	// Write the day number (converting decimal to BCD)
	sendByteRTC(((datetime.dayNo/10)<<4)+(datetime.dayNo%10));

	// Write the year (converting decimal to BCD)
	sendByteRTC(((datetime.year/10)<<4)+(datetime.year%10));

	// Write the write protect (set to off)
	sendByteRTC(0b00000000);

	// Chip enable low
	cbi(RTC_CE_PORT, RTC_CE_PIN);
	_delay_us(US_DELAY);
}

// Use a read burst to get the time and date
void readRTC(void)
{
	unsigned int temp;

	// Chip enable high
	sbi(RTC_CE_PORT, RTC_CE_PIN);
	_delay_us(US_DELAY);

	// Request a clock burst (returns 8 bytes)
	sendByteRTC(CLOCK_BURST + RTC_READ);

	// Read the seconds (in BCD and convert to decimal)
	temp = receiveByteRTC();
	datetime.seconds = ((temp >> 4)*10+(temp & 0x0F));

	// Read the minutes (in BCD and convert to decimal)
	temp = receiveByteRTC();
	datetime.minutes = ((temp >> 4)*10+(temp & 0x0F));

	// Read the hours  (in BCD and convert to decimal - 24hr format only)
	temp = receiveByteRTC();
	datetime.hours = ((temp >> 4)*10+(temp & 0x0F));

	// Read the day/date (in BCD and convert to decimal)
	temp = receiveByteRTC();
	datetime.day = ((temp >> 4)*10+(temp & 0x0F));

	// Read the month (in BCD and convert to decimal)
	temp = receiveByteRTC();
	datetime.month = ((temp >> 4)*10+(temp & 0x0F));

	// Read the day of the week number (in BCD and convert to decimal)
	temp = receiveByteRTC();
	datetime.dayNo = ((temp >> 4)*10+(temp & 0x0F));

	// Read the year (in BCD and convert to decimal)
	temp = receiveByteRTC();
	datetime.year = ((temp >> 4)*10+(temp & 0x0F));

	// Read the write protect (and discard as we don't need it)
	temp = receiveByteRTC();

	// Chip Enable low
	cbi(RTC_CE_PORT, RTC_CE_PIN);
	_delay_us(US_DELAY);
}

// Read the clock status
unsigned int readClockStatus(void)
{
	return clockStatus;
}

// Write the clock status
void writeClockStatus(unsigned int status)
{
	clockStatus = status;
}