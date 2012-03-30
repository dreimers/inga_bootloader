/* Copyright (c) 2010, Ulf Kulau
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "frq-calib.h"
#include "uart.h"

#include <util/delay.h>
#include <stdlib.h>

static uint8_t osccal_backup;

void frq_calib(void) {
	
	uint8_t tries = 100;
	uint16_t ticks;
	uint16_t min = 60000;

	osccal_backup = OSCCAL;

	
	TIMSK2 = 0x00;
	
	ASSR |= (1 << AS2);
	while (ASSR & ((1 << TCN2UB) | (1 << OCR2AUB) | (1 << TCR2AUB) | (1
			<< TCR2BUB))) {
		;
	}
	TCCR2B = (1 << CS20);
	TCCR1B = (1 << CS10);

	do {
		ticks = frq_calib_count();
		if (ticks > CALIB_FRQ_REF_CNT) {
			if((ticks - CALIB_FRQ_REF_CNT) < min){
			min = OSCCAL;
			}
			OSCCAL--;
		}
		CALIB_FRQ_WAIT();
		if (ticks < CALIB_FRQ_REF_CNT) {
			if((CALIB_FRQ_REF_CNT - ticks) < min){
			min = OSCCAL;
			}
			OSCCAL++;
		}
		CALIB_FRQ_WAIT();
		if (ticks == CALIB_FRQ_REF_CNT) {
			tries = 1;
		}
	} while (--tries);
TCCR1B = 0x00;
TCCR2B = 0x00;
CALIB_FRQ_WAIT();
}

uint16_t frq_calib_count(void) {
	uint16_t cnt_value;
//optimieren -> in while schelief verlegen

	TCNT2 = 0;
	TCNT1 = 0;

	while (ASSR & ((1 << TCN2UB) | (1 << OCR2AUB) | (1 << TCR2AUB) | (1
			<< TCR2BUB))) {
	}
	TCNT1 = 0;

	while (TCNT2 < 100) {
	}
	cnt_value = TCNT1;
	return cnt_value;
}

void frq_calib_restore_osccl(void){
OSCCAL = osccal_backup;
}


