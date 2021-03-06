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


#ifndef FRQCALIB_H_
#define FRQCALIB_H_

#include <avr/io.h>

#define CALIB_FRQ_CLAIMED   7372800UL
#define CALIB_FRQ_XTAL		32768UL
#define CALIB_FRQ_REF_CNT	(uint16_t)((CALIB_FRQ_CLAIMED / CALIB_FRQ_XTAL) * 100)

#define CALIB_FRQ_WAIT()	asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");\
							asm("nop");asm("nop");asm("nop");asm("nop");

void frq_calib(void);

uint16_t frq_calib_count(void);

void frq_calib_restore_osccl(void);

#endif /* FRQCALIB_H_ */
