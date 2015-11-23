/*
 * asf.h
 *
 * Created: 12/6/2012 5:00:14 PM
 *  Author: Caleb
 */ 


#ifndef ASF_H_
#define ASF_H_

#ifdef F_CPU
#undef F_CPU
#endif
#define F_CPU 8000000UL

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <util/twi.h>

#endif /* ASF_H_ */
