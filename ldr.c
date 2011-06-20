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

// Includes
#include <avr/io.h>
#include "hardware.h"
#include "ldr.h"

// LDR Notes: Intense light generates around 550mV.  Darkness generates around 50mV

// Initialise the LDR read for reading
void initialiseLdr(void)
{
	// Set the LDR port to input and turn off the internal pull-up
	cbi(LDR_DIR_PORT, LDR_PIN); // Set port as input
	cbi(LDR_PORT, LDR_PIN); // Pull up off
	
	// Configure ADMUX for 5V Vref - Left justified - ADC channel 0
	ADMUX = 0x40; // 01000000
	
	// Configure ADSC for ADC Enabled - /128 pre-scaler
	ADCSRA = 0x87; // 10000111
}

// Read the current value of the LDR (returns 0-15)
unsigned int readLdrValue(void)
{
	unsigned int ldrValue = 0;
	
	 // Clear the previous result
	ADCH = 0x00;
	
	// Set ADSC to start an ADC conversion
	sbi(ADCSRA, 6);

	// Wait for the ADC conversion to complete
	while(ADCSRA & (1 << ADSC));
	
	// Store the result
	ldrValue = ADC;
	
	// Vref is 5000 millivolts and we have 10-bits of resolution meaning
	// 1 unit of the ADC is around 4.88 mV
	//
	// The LDR returns 5V at full brightness and 0Vs in darkness
	
	// Return 0 to 15 with x being dark and y being light
	
	ldrValue /= 64;
	
	return ldrValue;
}