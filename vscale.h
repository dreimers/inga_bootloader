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
 *
 * \addtogroup Drivers
 *
 * \section about About
 *
 * These are the low level hardware drivers for the Energy Reducing
 * INGA Kernel, which are needed to get access to the subjacent hardware
 * modules.
 *
 * \section usage Usage
 *
 * The hardware drivers are used to interface the I2C Register of
 * the ATmega1284p. Furthermore they control the digital potentiometer of
 * the I2C voltage Scaling module and implement methods to set an RC-oscillator
 * frequency of the CPU.
 *
 * \section lic License
 *
 * <pre>Copyright (c) 2012, Ulf Kulau <kulau@ibr.cs.tu-bs.de>
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
 * OTHER DEALINGS IN THE SOFTWARE.</pre>
 */

/**
 * \addtogroup Drivers
 * @{
 * \defgroup poti_drv 256-Position Digital Potentiometer
 *
 * <p>
 * This driver controls the basic functionality of the AD5242
 * digital potentiometer. You can init the device and read or write
 * the register.
 * </p>
 *
 * @{
 *
 */

/**
 * \file
 *		AD5242 256-Position Digital Potentiometer interface definitions
 * \author
 *      Ulf Kulau <kulau@ibr.cs.tu-bs.de>
 */


#ifndef AD5242_H_
#define AD5242_H_

#include "i2c.h"

/*!
 * AD5242 read regsiter address
 */
#define AD5242_DEV_ADDR_R		0x59
/*!
 * AD5242 write regsiter address
 */
#define AD5242_DEV_ADDR_W		0x58

/*\cond*/
#define AD5242_CHN_A			0x00
#define AD5242_CHN_B			(1 << 7)
/*\endcond*/

/*!
 * AD5242 mid position reset bit
 */
#define AD5242_MID_RESET		(1 << 6)

/*!
 * AD5242 shutdown bit
 */
#define AD5242_SHDN_ON			(1 << 5)
/*!
 * AD5242 wakeup bit
 */
#define AD5242_SHDN_OFF			(0 << 5)

/*!
 * AD5242 I/O Pin 1 bit
 */
#define	AD5242_OUT_1			(1 << 4)
/*!
 * AD5242 I/O Pin 2 bit
 */
#define	AD5242_OUT_2			(1 << 3)
/*!
 * RADC Resistor Channel
 */
#define RADC_CHANNEL			AD5242_CHN_A

#define AD5242_TEST_VALUE		130
#define AD5242_RESET_VALUE		128

/**
 * \brief Initialize the AD5242 digital potentiometer
 *
 * \return <ul>
 * 		   <li> 0: i2c-bus driver was initiated
 * 		   </ul>
 */
uint8_t vscale_init(void);

 /**
  * \brief Write register value to given register address
  *
  * \param instr_reg <table border="1">
  * <tr>
  * <th> 0 </th>
  * <th> 1 </th>
  * <th> 2 </th>
  * <th> 3 </th>
  * <th> 4 </th>
  * <th> 5 </th>
  * <th> 6 </th>
  * <th> 7 </th>
  * </tr>
  * <tr>
  * <td> Channel A = 0, Channel B = 1 </td>
  * <td> Midscale Reset  </td>
  * <td> Shutdown  </td>
  * <td> Output logic pin 1 latched values </td>
  * <td> Output logic pin 2 latched values </td>
  * <td> x </td>
  * <td> x </td>
  * <td> x </td>
  * </tr>
  * </table>
  *
  * \param data RADC channel value
  *
  */
void vscale_write_radc(uint8_t instr_reg, uint8_t data);

/**
  * \brief Write register value to given register address
  *
  * \return Byte from previously selected master RADC register
  *
  */
uint8_t vscale_get_voltreg(void);

/**
 * \brief Method to set the voltage register. Raw values are absolute sufficient
 */
void vscale_set_voltreg(uint8_t data);
#endif
