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

/**
 * \addtogroup Drivers
 * @{
 * \addtogroup poti_drv
 *
 * @{
 */

/**
 * \file
 *     AD5242 256-Position Digital Potentiometer interface implementation
 * \author
 *      Ulf Kulau <kulau@ibr.cs.tu-bs.de>
 */

#include "vscale.h"

uint8_t vscale_init(void) {
	uint8_t test_vec;
	i2c_init();
	vscale_set_voltreg(AD5242_TEST_VALUE);
	test_vec = vscale_get_voltreg();
	vscale_set_voltreg(AD5242_RESET_VALUE);	
	if(test_vec == AD5242_TEST_VALUE){
		return 1;
	}else{
		return 0;
	}
}

void vscale_set_voltreg(uint8_t data){
	 vscale_write_radc(RADC_CHANNEL, data);
}

uint8_t vscale_get_voltreg(void){
	uint8_t data;
	i2c_start(AD5242_DEV_ADDR_R);
	i2c_read_nack(&data);
	i2c_stop();
	return data;
}


void vscale_write_radc(uint8_t instr_reg, uint8_t data){
	i2c_start(AD5242_DEV_ADDR_W);
	i2c_write(instr_reg);
	i2c_write(data);
	i2c_stop();
}






