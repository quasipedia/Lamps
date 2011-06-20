/************************************************************************
	Chasy Clock.c

    Word Clock Firmware
    Copyright (C) 2011 Simon Inns, Mac Ryan

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

	Email: simon.inns@gmail.com, quasipedia@gmail.com

************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "hardware.h"
#include "tlc5940.h"
#include "ds1302.h"
#include "buttons.h"
#include "ldr.h"
#include "channelmap.h"
#include "clockmap.h"
#include "tests.h"
#include <util/delay.h>

// Note: Target is ATmega168-20

// Test mode definitions
#define MODE_DISPLAYRTC		0
#define MODE_DISPLAYLDR		1
#define MODE_LEDCHASETEST	2
#define MODE_ALLON			3

// Clock state-machine states
#define STATE_CLOCKRUNNING	0
#define STATE_CHASETEST		1
#define STATE_TIMETEST		2

// Button definitions
#define BUTTON_MINUTE		0
#define BUTTON_HOUR			1
#define BUTTON_LDR			2
#define BUTTON_TEST			3

#define RELEASED			0
#define PRESSED				1

int main(void)
{
	// Configure port directions
	DDRB = 0xFF;
	DDRC = 0xFF;
	DDRD = 0xFF;
	
	// Clear all ports
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	
	// Power up delay of 1 second
	// This is required since we don't have a capacitor on the reset line
	// which can cause the reset to bounce as power ramps up after being
	// turned on
	for (unsigned int delay = 0; delay < 100; delay++) _delay_ms(10);
	
	// Initialise the TLC5940s
	initialiseTlc5940();

	// Initialise the LED fading control
	initialiseFadingLeds();
	
	// Enable interrupts globally
	sei();
	
	// Initialise the DS1302 RTC
	initialiseRTC();
	
	// Check if the RTC is set or unset (first run)
	if (readClockStatus() == CLOCK_UNSET)
	{
		// Clock is unset, so we set it to 00:00
		datetime.hours = 0;
		datetime.minutes = 0;
		datetime.seconds = 0;
		datetime.dayNo = 0;
		datetime.day = 12;
		datetime.month = 6;
		datetime.year = 11;
		
		// Set the clock
		setRTC();
	}
	
	// Initialise the LDR
	initialiseLdr();
	
	// Initialise the buttons
	initialiseButtons();
	
	// Initialise state-machine to run power-up test
	unsigned char clockState = STATE_CHASETEST;
	//unsigned char clockState = STATE_CLOCKRUNNING;
	
	// Set the start brightness
	int displayBrightness = 4095;
	
	// State-machine delay counter
	unsigned int delayCounter1 = 0;
	
	// Button functions are hours, minutes, LDR on/off and test
	unsigned char minuteButtonDownFlag = 0;
	unsigned char hourButtonDownFlag = 0;
	
	unsigned char ldrActiveFlag = 1; // LDR is active
	
	// Set the led fading speed
	setLedFadeSpeed(30, 100);
	
	while(1)
	{
		// Update the delay counter
		delayCounter1++;
		
		// Poll the button states
		pollButtons();
		
		// Clock running state
		if (clockState == STATE_CLOCKRUNNING)
		{
			// Update the clock display ------------------------------------------------------------------
			if (delayCounter1 > 30000)
			{	
				int minuteOfDay = 0;
			
				// Read the real-time clock
				readRTC();
			
				// Calculate the minute of the day
				minuteOfDay = (datetime.hours * 60) + datetime.minutes;
			
				// Update the display
				displayMinute(minuteOfDay, displayBrightness);
				
				// Reset the delay counter
				delayCounter1 = 0;
			}
			
			// Button handling ---------------------------------------------------------------------------
			
			// Minute button pressed?
			if (button[BUTTON_MINUTE].buttonState == PRESSED && minuteButtonDownFlag == 0)
			{
				// Read the current time
				readRTC();
				
				// Advance one minute and reset seconds
				if (datetime.minutes == 59)
				{
					datetime.minutes = 0;
				}
				else datetime.minutes++;
				datetime.seconds = 0;
				
				// Set the RTC
				setRTC();
				
				minuteButtonDownFlag = 1;
			}
			
			// Minute button released?
			if (button[BUTTON_MINUTE].buttonState == RELEASED && minuteButtonDownFlag == 1)
			{
				minuteButtonDownFlag = 0;
			}
			
			// Hour button pressed?
			if (button[BUTTON_HOUR].buttonState == PRESSED && hourButtonDownFlag == 0)
			{
				// Read the current time
				readRTC();
				
				// Advance one hour and reset seconds
				if (datetime.hours == 23)
				{
					datetime.hours = 0;
				}
				else datetime.hours++;
				datetime.seconds = 0;
				
				// Set the RTC
				setRTC();
				
				hourButtonDownFlag = 1;
			}
			
			// Minute button released?
			if (button[BUTTON_HOUR].buttonState == RELEASED && hourButtonDownFlag == 1)
			{
				hourButtonDownFlag = 0;
			}
			
			// Test button pressed? If so change state
			if (button[BUTTON_TEST].buttonState == 1) clockState = STATE_CHASETEST;
			
			// LDR brightness control --------------------------------------------------------------------
			if (ldrActiveFlag == 1)
			{
				unsigned int ldrValue = 0;
				
				// Read the LDR brightness level (0-15)
				ldrValue = readLdrValue();
				
				// Here we use a switch statement to translate the LDR level into the brightness
				// level for the PWM (which allows us to choose a logarithmic or linear scale)
				switch(ldrValue)
				{
					case 0 :	displayBrightness = 100;
								break;
					
					case 1 :	displayBrightness = (4095/16) * 1;
								break;
					
					case 2 :	displayBrightness = (4095/16) * 2;
								break;
					
					case 3 :	displayBrightness = (4095/16) * 3;
								break;
					
					case 4 :	displayBrightness = (4095/16) * 4;
								break;
					
					case 5 :	displayBrightness = (4095/16) * 5;
								break;
					
					case 6 :	displayBrightness = (4095/16) * 6;
								break;
					
					case 7 :	displayBrightness = (4095/16) * 7;
								break;
					
					case 8 :	displayBrightness = (4095/16) * 8;
								break;
					
					case 9 :	displayBrightness = (4095/16) * 9;
								break;
					
					case 10 :	displayBrightness = (4095/16) * 10;
								break;
					
					case 11 :	displayBrightness = (4095/16) * 11;
								break;
					
					case 12 :	displayBrightness = (4095/16) * 12;
								break;
					
					case 13 :	displayBrightness = (4095/16) * 13;
								break;
					
					case 14 :	displayBrightness = (4095/16) * 14;
								break;
					
					case 15 :	displayBrightness = 4095;
								break;
				}
			}
		}
		
		// Clock test state
		if (clockState == STATE_CHASETEST)
		{
			// Perform the test
			chaseTest();
			emrTest();

			// Set the led fading speed
			setLedFadeSpeed(30, 100);

			// Go back to the clock running state
			clockState = STATE_CLOCKRUNNING;
		}
	}
}
