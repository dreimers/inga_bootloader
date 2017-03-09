/* Copyright (c) 2017, Dennis Reimers <dreimers@ibr.cs.tu-bs.de>
 * based on inga_bootloader by Ulf Kulau et al.
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
#include <avr/wdt.h>
#include <stdlib.h>

#include "vscale.h"
#include "uart.h"
#include "update.h"
#include "flash-mgr.h"

#define LED_PORT PORTD
#define LED_DDR DDRD
#define LED_1_PIN PD5
#define LED_2_PIN PD7


#define LED_INIT()		LED_DDR |= (1 << LED_1_PIN)|(1 << LED_2_PIN)
#define LED_1_ON()		LED_PORT &=~(1 << LED_1_PIN)
#define LED_2_ON()		LED_PORT &=~(1 << LED_2_PIN)
#define LED_1_OFF()		LED_PORT |= (1 << LED_1_PIN)
#define LED_2_OFF()		LED_PORT |= (1 << LED_2_PIN)
#define LED_1_TOGGLE()		LED_PORT ^= (1 << LED_1_PIN);
#define LED_2_TOGGLE()		LED_PORT ^= (1 << LED_2_PIN);

/* store timer register content in memory for restore on exit */
uint8_t backup_timer_cfg = 0;
uint8_t backup_timer_mask = 0;
uint8_t backup_timer_compare= 0;

/******************VERY IMPORTANT********************************************
 * this code block is necessary to prevent the processor to run into watchdog-reset-lifelock 
 * do NOT delete this block
 ****************************************************************************/
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

void wdt_init(void)
{
	MCUSR &= ~(1<<WDRF);
	wdt_disable();

	return;
}
/*******************end of very important block***************************/

void start_app(void){

	use_main_iv_table();

	LED_1_OFF();
	LED_2_OFF();
	asm volatile("jmp 0000");
}

void wdt_reboot(void){
	wdt_enable(0);
	while(1){
		//	LED_1_TOGGLE();
	}
}

void use_main_iv_table(void)	{
	uint8_t tmp_MCUCR;

	cli();
	tmp_MCUCR = MCUCR;

	MCUCR = tmp_MCUCR | ( 1<<IVCE );
	MCUCR = tmp_MCUCR & ~ ( 1<<IVSEL );
}

void use_bootloader_iv_table(void) {
	uint8_t tmp_MCUCR, tmp_SREG;

	tmp_SREG = SREG;
	cli();

	tmp_MCUCR = MCUCR;
	MCUCR = tmp_MCUCR | ( 1<<IVCE );
	MCUCR = tmp_MCUCR | ( 1<<IVSEL );
	SREG = tmp_SREG;
}

void start_exit_timer(void) {
	/*save register content*/
	backup_timer_cfg = TCCR0B;
	backup_timer_mask = TIMSK0;
	backup_timer_compare = OCR0A;

	restart_exit_timer();
}

void restart_exit_timer(void) {

	TCCR0B = ( ( 1 << CS02 ) | ( 1 << CS00 ) );
	TIMSK0 = ( 1 << OCIE0A );
	OCR0A  = 100;
}

void stop_exit_timer(void) {
	/*restore register content*/
	TCCR0B = backup_timer_cfg;
	TIMSK0 = backup_timer_mask;
	OCR0A = backup_timer_compare;
}

int main ( void )
{
	LED_INIT();
	LED_2_ON();

	static uint32_t address;

	LED_1_OFF();
	uart_init();

	vscale_init();
	vscale_set_voltreg(0);

	if ( ( MCUSR & _BV ( PORF ) ) ) {
		MCUSR &= ~ ( 1 << EXTRF );
		MCUSR &= ~ ( 1 << PORF );
	}

	use_bootloader_iv_table();

	start_exit_timer();

	LED_1_ON();
	LED_2_ON();

	clear_local_buffer();

	/* turn on global interrupts*/
	sei();

	while ( 1 ) {
		uint16_t temp_int;
		uint8_t val;

		uint8_t enable_exit_timer = 1;

		/* AVR109 protocol*/
		/* receive char */
		val = uart_RXchar();

		enable_exit_timer = 0;

		switch ( val ) {
			case 'a':
				uart_TXchar ( 'Y' );
				break;
			case 'A':
				address = 0;
				address = (uint32_t)( (uint16_t)uart_RXchar() << 8 ) + uart_RXchar();
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
				uart_TXchar ( 'I' );
				uart_TXchar ( 'N' );
				uart_TXchar ( 'G' );
				uart_TXchar ( 'A' );
				uart_TXchar ( ' ' );
				uart_TXchar ( 'B' );
				uart_TXchar ( 'L' );
				break;
			case 'V':
				uart_TXchar ( '1' );
				uart_TXchar ( '4' );
				break;
			case 'v':
				uart_TXchar ( '1' );
				uart_TXchar ( '4' );
				break;
			case 's':
				uart_TXchar ( SIGNATURE_2 );
				uart_TXchar ( SIGNATURE_1 );
				uart_TXchar ( SIGNATURE_0 );
				break;
			case 't':
				uart_TXchar ( 0 );
				break;
			case 'g':
				temp_int = (uint16_t)( (uint16_t)uart_RXchar() << 8 ) + uart_RXchar(); // Get block size.
				val = uart_RXchar(); // Get memtype
				page_read ( temp_int, val, &address ,0); // Block read
				break;
			case 'p':
				uart_TXchar ( 'S' );
				break;
			case 'B':

				temp_int = (uint16_t)( (uint16_t)uart_RXchar() << 8 ) + uart_RXchar(); // Get block size.
				val = uart_RXchar(); //mem type

				//uart_TXchar(update_b_size>>8);
				uint16_t r = page_write ( temp_int,0, val, &address );
				if( r>0 ) {
					uart_TXchar ( '\r' );
				} else {
					uart_TXchar ( '?' );
				}
				break;
			case 'P':
				uart_TXchar ( '\r' );
				break;
			case 'L':
				LED_2_ON();
				uart_TXchar ( '\r' ); // Nothing special to do, just answer OK.
				break;
			case 'E':
				cli();
				uart_TXchar ( '\r' );
				_delay_ms ( 1 );
				boot_rww_enable_safe();
				uart_deinit();

				stop_exit_timer();
				start_app();
				break;
			default:
				enable_exit_timer = 1;
				uart_TXchar ( '?' );
				break;
		}

		if (enable_exit_timer != 0)	{
			/* start timer */
			restart_exit_timer();
		} else {
			/* stop timer */
			stop_exit_timer();
		}
	}

	return 0;
}

ISR ( TIMER0_COMPA_vect )
{
	static uint16_t count = 0;
	count++;
	if ( count % 30 == 0 ) {
		LED_2_TOGGLE();
	}
	if ( count > 180 ) {
		stop_exit_timer();

		start_app();
	}
}
// kate: indent-mode cstyle; replace-tabs off; tab-width 8; 
