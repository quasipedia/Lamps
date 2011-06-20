/************************************************************************
	channelmap.c

    Word Clock Firmware - Mac's channel mapping
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
#include "tlc5940.h"
#include "channelmap.h"
#include <util/delay.h>

unsigned char channelMap(unsigned char channel)
{
	// Range check the channel
	if (channel > 106)
		channel = 106;

	// Correct mis-wired channels using hard-coded corrections ---------------------------------------------------------
	if (channel == 11) channel = 12;
	else if (channel == 12) channel = 11;
	
	else if (channel == 13) channel = 14;
	else if (channel == 14) channel = 13;
	
	else if (channel == 52) channel = 56;
	else if (channel == 56) channel = 52;
	
	else if (channel == 90) channel = 93;
	else if (channel == 93) channel = 90;
	
	// Remap the channels according to the physical channel map of the TLC --------------------------------------------
	int tlcNumber, localChannel, mappedChannel = 0;
	
	// Find the number of the target TLC
	if (channel != 0) tlcNumber = channel / 16; // Since we are using an int this will round down
	else tlcNumber = 0;  // Avoid divide by zero
	
	// Find the 'local' channel number for the TLC
	localChannel = channel - (tlcNumber * 16);

	// Map the pin
	switch(localChannel)
	{
		case 0 :	mappedChannel = TLC_CHANNEL0;
					break;
		case 1 :	mappedChannel = TLC_CHANNEL1;
					break;
		case 2 :	mappedChannel = TLC_CHANNEL2;
					break;
		case 3 :	mappedChannel = TLC_CHANNEL3;
					break;
		case 4 :	mappedChannel = TLC_CHANNEL4;
					break;
		case 5 :	mappedChannel = TLC_CHANNEL5;
					break;
		case 6 :	mappedChannel = TLC_CHANNEL6;
					break;
		case 7 :	mappedChannel = TLC_CHANNEL7;
					break;
		case 8 :	mappedChannel = TLC_CHANNEL8;
					break;
		case 9 :	mappedChannel = TLC_CHANNEL9;
					break;
		case 10 :	mappedChannel = TLC_CHANNEL10;
					break;
		case 11 :	mappedChannel = TLC_CHANNEL11;
					break;
		case 12 :	mappedChannel = TLC_CHANNEL12;
					break;
		case 13 :	mappedChannel = TLC_CHANNEL13;
					break;
		case 14 :	mappedChannel = TLC_CHANNEL14;
					break;
		case 15 :	mappedChannel = TLC_CHANNEL15;
					break;
	}
	
	// Now readjust the channel number against the mapped channel
	channel = mappedChannel + (tlcNumber * 16);
	
	// Reverse the channel order --------------------------------------------------------------------------------------
	channel = (NUMBEROF5940 * 16 - 1) - channel;
	
	return channel;
}