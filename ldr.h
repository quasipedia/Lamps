/************************************************************************
	ldr.h

    Word Clock Firmware - Light Dependent Resistor
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

#ifndef LDR_H_
#define LDR_H_

// Hardware mapping
#define LDR_PORT		PORTC
#define LDR_PIN			0
#define LDR_INP			PINC
#define LDR_DIR_PORT	DDRC

// Function prototypes
void initialiseLdr(void);
unsigned int readLdrValue(void);

#endif /* LDR_H_ */