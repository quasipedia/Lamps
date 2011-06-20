/************************************************************************
	buttons.h

    Word Clock Firmware - Hardware buttons
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
#include "buttons.h"

// Initialise the button states
void initialiseButtons(void)
{
	// Initialise the button states
	for (unsigned char buttonNumber = 0; buttonNumber < NUMBEROFBUTTONS; buttonNumber++)
	{
		button[buttonNumber].buttonState = 0; // Button is off
		button[buttonNumber].debounceCounter = 0; // Reset debounce counter
	}
	
	// Set the button ports to input and turn on the internal pull-ups
	cbi(BUTTON1_DIR_PORT, BUTTON1_PIN); // Set port as input
	sbi(BUTTON1_PORT, BUTTON1_PIN); // Pull up on
	
	cbi(BUTTON2_DIR_PORT, BUTTON2_PIN); // Set port as input
	sbi(BUTTON2_PORT, BUTTON2_PIN); // Pull up on
	
	cbi(BUTTON3_DIR_PORT, BUTTON3_PIN); // Set port as input
	sbi(BUTTON3_PORT, BUTTON3_PIN); // Pull up on
	
	cbi(BUTTON4_DIR_PORT, BUTTON4_PIN); // Set port as input
	sbi(BUTTON4_PORT, BUTTON4_PIN); // Pull up on
}

// Poll the hardware buttons
void pollButtons(void)
{
	unsigned char buttonState;
	
	for (unsigned char buttonNumber = 0; buttonNumber < NUMBEROFBUTTONS; buttonNumber++)
	{
		// Get the physical button state
		switch (buttonNumber)
		{
			case 0:	if (BUTTON1_INP & (1 << BUTTON1_PIN)) buttonState = 0; else buttonState = 1;
					break;
					
			case 1:	if (BUTTON2_INP & (1 << BUTTON2_PIN)) buttonState = 0; else buttonState = 1;
					break;
					
			case 2:	if (BUTTON3_INP & (1 << BUTTON3_PIN)) buttonState = 0; else buttonState = 1;
					break;
					
			case 3:	if (BUTTON4_INP & (1 << BUTTON4_PIN)) buttonState = 0; else buttonState = 1;
					break;
		}
		

		
		// Note: buttons are active low so on is 0 and off is 1
		
		// Debounce ON
		if (buttonState == 1 && button[buttonNumber].buttonState == 0)
		{
			// If the debounce tolerance is met change state otherwise
			// increment the debounce counter
			if (button[buttonNumber].debounceCounter > BUTTONDEBOUNCE)
				button[buttonNumber].buttonState = 1;		
			else button[buttonNumber].debounceCounter++;
		}
	
		// Debounce OFF
		if (buttonState == 0 && button[buttonNumber].buttonState == 1)
		{
			// If the debounce tolerance is met change state otherwise
			// increment the debounce counter
			if (button[buttonNumber].debounceCounter > BUTTONDEBOUNCE)
				button[buttonNumber].buttonState = 0;			
			else button[buttonNumber].debounceCounter++;
		}
		
		// Reset debounce counter
		if (buttonState == button[buttonNumber].buttonState) button[buttonNumber].debounceCounter = 0;
	} // end for
}