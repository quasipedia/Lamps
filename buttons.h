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

#ifndef BUTTONS_H_
#define BUTTONS_H_

// Define the number of buttons
#define NUMBEROFBUTTONS	4

// Define the number of debounce checks before switching state
#define BUTTONDEBOUNCE 10

// Button hardware mapping definitions
#define BUTTON1_PORT		PORTC
#define BUTTON1_PIN			1
#define BUTTON1_INP			PINC
#define BUTTON1_DIR_PORT	DDRC

#define BUTTON2_PORT		PORTC
#define BUTTON2_PIN			2
#define BUTTON2_INP			PINC
#define BUTTON2_DIR_PORT	DDRC

#define BUTTON3_PORT		PORTC
#define BUTTON3_PIN			3
#define BUTTON3_INP			PINC
#define BUTTON3_DIR_PORT	DDRC

#define BUTTON4_PORT		PORTC
#define BUTTON4_PIN			4
#define BUTTON4_INP			PINC
#define BUTTON4_DIR_PORT	DDRC

// Define a global structure for storing the button states
struct buttonStruct {
	unsigned char buttonState;
	unsigned char debounceCounter;
};

struct buttonStruct button[NUMBEROFBUTTONS];

// Function prototypes
void initialiseButtons(void);
void pollButtons(void);

#endif /* BUTTONS_H_ */