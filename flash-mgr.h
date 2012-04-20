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

#ifndef FLASHMGR_H_
#define FLASHMGR_H_

#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "uart.h"

#define PAGESIZE 			256
#define APPLICATION_END		0x1E000

static uint8_t local_buffer[PAGESIZE];

//void page_read_uart ( uint16_t size, uint8_t mem_type, uint32_t *address );

void page_read ( uint16_t *size, uint8_t mem_type, uint32_t *address, uint8_t *buff );

//uint8_t page_write_uart ( uint16_t size, uint8_t mem_type, uint32_t *address );

uint16_t page_write( uint16_t *b_size, uint16_t *data_ptr, uint8_t mem_type, uint32_t *address );

void clear_local_buffer ( void );

//void fill_page ( uint32_t address );

void erase_flash ( void );

#endif /* FLASHMGR_H_ */
