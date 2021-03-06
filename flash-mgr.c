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
#include <avr/io.h>
#include "flash-mgr.h"
#include "update.h"
static uint8_t local_buffer[PAGESIZE];
/*
void page_read_uart ( uint16_t size, uint8_t mem_type, uint32_t *address )
{
	uint16_t addr= ( *address ) &0xFFFF;
	uint8_t *a_ptr = ( uint8_t * ) addr;
	boot_spm_busy_wait();
	switch ( mem_type ) {
	case 'E':
		do {
			uart_TXchar ( eeprom_read_byte ( a_ptr++ ) );
			( *address ) ++;
			size--; // Decrease number of bytes to read
		} while ( size ); // Repeat until all block has been read
		break;
	case 'F':
		( *address ) <<= 1; // Convert address to bytes temporarily.

		do {

			uint32_t add_t;
			add_t = ( *address );

			uart_TXchar ( pgm_read_byte_far ( add_t ) );
			uart_TXchar ( pgm_read_byte_far ( ( add_t ) + 1 ) );

			( *address ) += 2; // Select next word in memory.
			size -= 2; // Subtract two bytes from number of bytes to read
		} while ( size ); // Repeat until all block has been read

		( *address ) >>= 1; // Convert address back to Flash words again.
		break;
	default:
		uart_TXchar ( '?' );
	}
}
*/
void page_read ( uint16_t size, uint8_t mem_type, uint32_t *address, uint8_t *buff )
{
	uint16_t addr= ( *address ) &0xFFFF;
	uint8_t *a_ptr = ( uint8_t * ) addr;
	boot_spm_busy_wait();
	switch ( mem_type ) {
	case 'E':
		do {
			if(buff>0){
				*buff= eeprom_read_byte ( a_ptr++ );
				buff++;
			}else{
				uart_TXchar ( eeprom_read_byte ( a_ptr++ ) );
			}
			( *address ) ++;
			size--; // Decrease number of bytes to read
		} while ( size ); // Repeat until all block has been read
		break;
	case 'F':
		( *address ) <<= 1; // Convert address to bytes temporarily.
		do {
			uint32_t add_t;
			add_t = ( *address );
			if(buff>0){
				*buff= pgm_read_byte_far ( add_t );
				buff++;
				*buff= pgm_read_byte_far ( ( add_t ) + 1 );
				buff++;
			}else{
				uart_TXchar ( pgm_read_byte_far ( add_t ) );
				uart_TXchar ( pgm_read_byte_far ( ( add_t ) + 1 ) );
			}
			( *address ) += 2; // Select next word in memory.
			size -= 2; // Subtract two bytes from number of bytes to read
		} while ( size ); // Repeat until all block has been read

		( *address ) >>= 1; // Convert address back to Flash words again.
		break;
	}
}
/*
uint8_t page_write_uart( uint16_t size, uint8_t mem_type, uint32_t *address )
{
	uint8_t rx_data;
	uint32_t tmp_address;
	uint16_t addr= ( *address ) &0xFFFF;
	uint8_t *a_ptr = ( uint8_t * ) addr;

	switch ( mem_type ) {
	case 'E':

		for ( tmp_address = 0; tmp_address < size; tmp_address++ ) {
			local_buffer[tmp_address] = uart_RXchar();
		}
		boot_spm_busy_wait();

		for ( tmp_address = 0; tmp_address < size; tmp_address++ ) {
			eeprom_write_byte ( a_ptr++, local_buffer[tmp_address] );
		}
		( *address ) += size;

		break;

	case 'F':

		( *address ) <<= 1; // Convert address to bytes temporarily.
		tmp_address = ( *address ); // Store address in page.
		uint16_t i = 0;
		uint16_t block_size = size;

		do {
			local_buffer[i++] = uart_RXchar();
			size -=1;
		} while ( size ); // Loop until all bytes written.
		( *address ) += block_size;

		fill_page ( tmp_address );
		( *address ) >>= 1;
		break;
	default:
		uart_TXchar ( '?' );
	}
	return '\r';
}
*/
	uint32_t tmp_address;
uint16_t page_write( uint16_t b_size, uint16_t *data_ptr, uint8_t mem_type, uint32_t *address )
{
	uint8_t rx_data;
	uint16_t addr= ( *address ) &0xFFFF;
	uint8_t *a_ptr = ( uint8_t * ) addr;

	switch ( mem_type ) {
	case 'E':
		if(data_ptr==0){
			for ( tmp_address = 0; tmp_address < b_size; tmp_address++ ) {
				local_buffer[tmp_address] = uart_RXchar();
			}
			data_ptr=(uint16_t *)local_buffer;
		}
		boot_spm_busy_wait();
		for ( tmp_address = 0; tmp_address < b_size; tmp_address++ ) {
			eeprom_write_byte ( a_ptr++, data_ptr[tmp_address]>>8 );
			eeprom_write_byte ( a_ptr++, data_ptr[tmp_address] );
		}
		( *address ) += b_size;

		break;

	case 'F':
		( *address ) <<= 1; // Convert address to bytes temporarily.
		tmp_address = ( *address ); // Store address in page.
		if(((uint32_t)tmp_address + (uint32_t)b_size) > 0x1e000){ //prevent bootloader from over writing it self
			return 0;
		}
		uint16_t i = 0;
		uint16_t block_size = b_size;
		uint16_t data;

		if (data_ptr==0){
			//uart_TXchar(b_size>>8);
			do {
				local_buffer[i++] = uart_RXchar();
				b_size -=1;
			} while ( b_size ); // Loop until all bytes written.
		//	uart_TXchar(size);
		//	fill_page ( tmp_address );
			data_ptr=local_buffer;
		}
		i=0;
		for ( i = 0; i < PAGESIZE; i+=2 ) {
			boot_page_fill_safe(tmp_address + i, data_ptr[i/2]);
		}
		clear_local_buffer();
		boot_page_write_safe ( tmp_address );
		boot_spm_busy_wait();
		( *address ) += block_size;
		( *address ) >>= 1;
		b_size = block_size;
		
		break;
	}
	return b_size;
}

void clear_local_buffer ( void )
{
	uint16_t i;
	for ( i = 0; i < 256; i++ ) {
		local_buffer[i] = 0xFF;
	}

}
/*
void fill_page ( uint32_t address )
{
	uint16_t i;
	uint16_t data;

	for ( i = 0; i < PAGESIZE; i += 2 ) {

		data = local_buffer[i];
		data |= ( local_buffer[i + 1] << 8 );

		boot_page_fill_safe ( address + i, data );
	}
	clear_local_buffer();

	boot_page_write_safe ( address );
}
*/
void erase_flash ( void )
{
	uint32_t i;
	for ( i = 0; i < APPLICATION_END; i += PAGESIZE ) {
		boot_page_erase_safe ( i );
	}
}



