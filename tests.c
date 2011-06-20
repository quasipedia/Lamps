/************************************************************************
	tests.c

    Word Clock Firmware - Hardware buttons
    Copyright (C) 2011 Mac Ryan

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

	Email: quasipedia@gmail.com

************************************************************************/

// Includes
#include <avr/io.h>
#include "hardware.h"
#include "tests.h"
#include "ldr.h"
#include "channelmap.h"
#include "buttons.h"
#include "tlc5940.h"
#include <util/delay.h>

void chaseTest(void)
{
	int channel;

	// Set the led fading speed
	setLedFadeSpeed(4095, 400);

	// Turn all channels off
	for (channel = 0; channel <= 106; channel++)
	{
		setLedBrightness(channelMap(channel), 0);
	}

	// Run the chase pattern
	for (channel = 0; channel <= 106; channel++)
	{
		// We have to poll the buttons here since we have suspended the state machine
		pollButtons();

		if (channel != 15 && channel != 31 && channel != 47 && channel != 63 && channel != 79 && channel != 95)
		{
			// Current channel on
			setLedBrightness(channelMap(channel), 4095);

			// Update the TLC
			while(updateTlc5940() == 1);

			// Wait
			for (unsigned int delay = 0; delay < 2; delay++) _delay_ms(10);

			// Channel off
			setLedBrightness(channelMap(channel), 0);
		}
	}
}

void emrTest(void)
{
	// This test is designed to run the clock in such a configuration that the Electro-Magnetic Radiation
	// (EMR) will be maximum. This is a stress-test for the hardware design.
	//
	// Power dissipation rating for HTSSOP (PWM) package without thermal pad is 1053mW
	// Device basic dissipation (maximum) = 60mW
	// Each channel can have between 3 and 7 LED's, hence dissipation can vary. Assuming
	// dot correction is unset (63/63) and Duty cycle is 100%, the values are:
	// 3 --> 299mW   4 --> 231mW   5 --> 163mW   6 --> 95mW   7 --> 26mW
	//
	// The maximum energy swing across the chip is obtained by setting PWM all the channels
	// on the chip to a GS value that cause the chip to reach its maximum dissipation rating.

	// The below numbers are slightly conservative figures, they run the chip within ~50mW
	// of its maximum rating.
	const int maxGSvalues[] = {1818, 1875, 1393, 1875, 1498, 1666, 2110};
	const int maxSafeGSforAll = 1393;

	// Turn all channels off
	char channel;
	for (channel = 0; channel <= 106; channel++)
	{
		setLedBrightness(channelMap(channel), 0);
	}

	char chip;
	char counter;

	// TODO: advancement by pressing button instead of automatic
	pollButtons();
	//	if (button[BUTTON_TEST].buttonState == PRESSED) {};

	// Set slow fade-in
	setLedFadeSpeed(15, 4095);

	// First pass: all channels get lit ON progressively
	for (chip = 0; chip < 7; chip++)
	{
		for (counter = 0; counter < 15; counter++)
		{
			// Calculate channel number
			channel = chip*16+counter;

			// Current channel on
			setLedBrightness(channelMap(channel), maxGSvalues[chip]);

			// Update the TLC
			while(updateTlc5940() == 1);

			// Wait
			_delay_ms(300);
		}
	}

	// Switch off fading
	setLedFadeSpeed(4095, 4095);

	// Second pass: all channels get lit ON progressively but they blink on and off
	for (chip = 0; chip < 7; chip++)
	{
		for (counter = 0; counter < 15; counter++)
		{
			// Calculate channel number
			channel = chip*16+counter;

			// Turn ON all channels up to and including the present one
			char i;
			for (i = 0; i <= channel; i++)
			{
				setLedBrightness(channelMap(i), maxSafeGSforAll);
			}

			// Update the TLC
			while(updateTlc5940() == 1);

			// Wait
			_delay_ms(150);

			// Turn off all channels
			for (channel = 0; channel <= 106; channel++)
			{
				setLedBrightness(channelMap(channel), 0);
			}

			// Update the TLC
			while(updateTlc5940() == 1);

			// Wait
			_delay_ms(150);
		}
	}
}
