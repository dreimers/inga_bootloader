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

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

#include "uart.h"
#include "frq-calib.h"
#include "flash-mgr.h"
#include "flash-microSD.h"
#include "flash-at45db.h"
#include "update.h"
#include "update_SD.h"
#include "diskio.h"
#include "../../contiki/cpu/stm32w108/e_stdio/src/small_mprec.h"

#define LED_INIT()		DDRD |= (1 << PD5)|(1 << PD7)
#define BUTTON_INIT()		DDRB  &= ~(1<<DDB2);PORTB |= (1 << PB2)

#define LED_1_ON()		PORTD &=~(1 << PD5)
#define LED_2_ON()		PORTD &=~(1 << PD7)
#define LED_1_OFF()		PORTD |= (1 << PD5)
#define LED_2_OFF()		PORTD |= (1 << PD7)
#define LED_1_TOGGLE()		PORTD ^= (1 << PD5);
#define LED_2_TOGGLE()		PORTD ^= (1 << PD7);

#define BUTTON_PRESSED()	!(PINB & (1<<PINB2))

/*
 * 	main
 *
 *
 *
 */
/*function pointer to address 0x0000 to start user application*/
void ( *start_app ) ( void ) = 0x0000;

int main ( void )
{
	static uint32_t address;
	uint8_t tmp_MCUCR, tmp_SREG;
	uint8_t start_bootloader = 0;;
	LED_INIT();
	BUTTON_INIT();
	LED_1_OFF();
	LED_2_OFF();
	at45db_init();

#ifdef BL
	
	if ( ( MCUSR & _BV ( PORF ) ) ) {
		MCUSR &=~ ( 1 << EXTRF );
		MCUSR &=~s ( 1 << PORF );
	}
	//stay in bootloader conditions
	if ( BUTTON_PRESSED() ) {
		start_bootloader = 1;
	} else if (microSD_init() == 0) { // SD-card found
#if UPDATE_EVERYTIME
		start_bootloader = 2;
#else
		uint8_t update_flag=0;
		at45db_read_page_bypassed(AT45DB_PAGES-1,0,&update_flag,1);
		if(update_flag){
			start_bootloader=2;
		}
#endif
	} else if ( MCUSR & _BV ( EXTRF ) ) {
		LED_2_ON();
		tmp_SREG = SREG;
		cli();
		tmp_MCUCR = MCUCR;
		MCUCR = tmp_MCUCR | ( 1<<IVCE );
		MCUCR = tmp_MCUCR | ( 1<<IVSEL );
		SREG = tmp_SREG;
		TCCR0B |= ( ( 1 << CS02 ) | ( 1 << CS00 ) );
		TIMSK0 |= ( 1 << OCIE0A );
		OCR0A  |= 100;
		start_bootloader = 3;
	}

	if ( !start_bootloader ) {
		start_app();
	}
#else 
	start_bootloader = 2;
#endif

	//LED_1_ON();
	uart_init();
	clear_local_buffer();
	//LED_2_ON();
	frq_calib();
	//LED_2_OFF();
#ifndef BL
	uint8_t test=microSD_init();
#endif
	sei();
	//check for other firmware sources than uart
	//TODO check SD-card first block imagelen + magic code and (len/512)+1.block magic code
	if( start_bootloader == 2){
		uint8_t val_error =update_sd_validate(0);
		if( val_error == 0){
			update_sd_install(0);
		}
	} else if ( start_bootloader == 3 ){
		while ( 1 ) {
			//uart protocol
			uint16_t temp_int;
			uint8_t val;
			val = uart_RXchar();
			
			switch ( val ) {
			case 'a':
				uart_TXchar ( 'Y' );
				break;
			case 'A':
				address = 0;
				address = ( uart_RXchar() << 8 ) | uart_RXchar();
				boot_rww_enable_safe();
				uart_TXchar ( '\r' ); // Send OK back.

				break;
			case 'e':
				LED_2_ON();
				erase_flash();
				LED_2_OFF();
				uart_TXchar ( '\r' ); // Send OK back.
				break;
			case 'b':
				uart_TXchar ( 'Y' ); // Report block load supported.
				uart_TXchar ( ( PAGESIZE >> 8 ) & 0xFF ); // MSB first.
				uart_TXchar ( PAGESIZE & 0xFF );
				break;
			case 'x':
				uart_RXchar(); 		//ignore
				uart_TXchar ( '\r' );
				break;
			case 'y':
				uart_RXchar(); 		//ignore
				uart_TXchar ( '\r' );
				break;
			case 'T':
				uart_RXchar(); 		//ignore
				uart_TXchar ( '\r' );
				break;
			case 'S':
				cli();
				TCCR0B &= ~ ( ( 1 << CS00 ) | ( 1 << CS02 ) );
				TIMSK0 &= ~ ( 1 << OCIE0A );

				tmp_MCUCR = MCUCR;
				cli();
				MCUCR = tmp_MCUCR | ( 1<<IVCE );
				MCUCR = tmp_MCUCR & ~ ( 1<<IVSEL );

				uart_TXchar ( 'I' );
				uart_TXchar ( 'N' );
				uart_TXchar ( 'G' );
				uart_TXchar ( 'A' );
				uart_TXchar ( ' ' );
				uart_TXchar ( ' ' );
				uart_TXchar ( ' ' );
				break;
			case 'V':
				uart_TXchar ( '1' );
				uart_TXchar ( '2' );
				break;
			case 'v':
				uart_TXchar ( '1' );
				uart_TXchar ( '2' );
				break;
			case 's':
				uart_TXchar ( 0x05 );
				uart_TXchar ( 0x97 );
				uart_TXchar ( 0x1E );
				break;
			case 't':
				uart_TXchar ( 0 );
				break;
			case 'g':
				temp_int = ( uart_RXchar() << 8 ) | uart_RXchar(); // Get block size.
				val = uart_RXchar(); // Get memtype
				page_read_uart ( temp_int, val, &address ); // Block read
				break;
			case 'p':
				uart_TXchar ( 'S' );
				break;
			case 'B':

				temp_int = ( uart_RXchar() << 8 ) | uart_RXchar(); // Get block size.
				val = uart_RXchar(); //mem type
				uart_TXchar ( page_write_uart ( temp_int, val, &address ) ); // Block load.

				break;
			case 'P':
				uart_TXchar ( '\r' );
				break;
			case 'L':
				uart_TXchar ( '\r' ); // Nothing special to do, just answer OK.
				break;
			case 'E':
				uart_TXchar ( '\r' );
				_delay_ms ( 1 );
				boot_rww_enable_safe();

				frq_calib_restore_osccl();
				CALIB_FRQ_WAIT()
				LED_1_OFF();
				start_app();
				break;
			default:
				uart_TXchar ( '?' );
				break;
			}
		}
	}

	return 0;
}

#ifdef BL
ISR ( TIMER0_COMPA_vect )
{
	static uint16_t count = 0;
	count++;
	if ( count % 30 == 0 ) {
		LED_2_TOGGLE();
	}
	if ( count > 500 ) {
		uint8_t tmp_MCUCR;

		tmp_MCUCR = MCUCR;
		cli();
		TCCR0B &= ~ ( ( 1 << CS00 ) | ( 1 << CS02 ) );
		TIMSK0 &= ~ ( 1 << OCIE0A );

		MCUCR = tmp_MCUCR | ( 1<<IVCE );
		MCUCR = tmp_MCUCR & ~ ( 1<<IVSEL );

		frq_calib_restore_osccl();
		LED_1_OFF();
		LED_2_OFF();
		start_app();
	}
}
#endif
// kate: indent-mode cstyle; replace-tabs off; tab-width 8; 
