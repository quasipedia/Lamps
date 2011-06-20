/************************************************************************
	channelmap.h

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

#ifndef CHANNELMAP_H_
#define CHANNELMAP_H_

// Channel mapping: here you can set the channel order to the physical
// order if your circuit is not routed in the expected order of 0-16
#define TLC_CHANNEL0	1
#define TLC_CHANNEL1	3
#define TLC_CHANNEL2	5
#define TLC_CHANNEL3	7
#define TLC_CHANNEL4	9
#define TLC_CHANNEL5	11
#define TLC_CHANNEL6	13
#define TLC_CHANNEL7	15
#define TLC_CHANNEL8	0
#define TLC_CHANNEL9	2
#define TLC_CHANNEL10	4
#define TLC_CHANNEL11	6
#define TLC_CHANNEL12	8
#define TLC_CHANNEL13	10
#define TLC_CHANNEL14	12
#define TLC_CHANNEL15	14

// Function prototypes
unsigned char channelMap(unsigned char channel);

#endif /* CHANNELMAP_H_ */