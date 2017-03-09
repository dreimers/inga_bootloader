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

#include "uart.h"
#include "util/delay.h"
#include "avr/io.h"
#define LED_1_ON()		PORTD &=~(1 << PD5)
#define LED_2_ON()		PORTD &=~(1 << PD7)
#define LED_1_OFF()		PORTD |= (1 << PD5)
#define LED_2_OFF()		PORTD |= (1 << PD7)
#define LED_1_TOGGLE()		PORTD ^= (1 << PD5);
#define LED_2_TOGGLE()		PORTD ^= (1 << PD7);
uint8_t channel = 2;
uint8_t h,l,b,c;

void uart_init(void) {

	//DDRA|=0xe0;
	//PORTA= 0;
	h=UBRR0H;
	l=UBRR0L;
	b=UCSR0B;
	c=UCSR0C;
	UBRR0H = (uint8_t) (UBRR_UART >> 8);
	UBRR0L = (uint8_t) UBRR_UART;
	
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	do {
		UDR0;
	} while (UCSR0A & (1 << RXC0));
	
	
	
	UBRR1H = (uint8_t) (UBRR_UART >> 8);
	UBRR1L = (uint8_t) UBRR_UART;

	UCSR1B = (1 << RXEN1) | (1 << TXEN1);
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);

	do {
		UDR1;
	} while (UCSR1A & (1 << RXC1));
}

void uart_deinit(void)
{
	UBRR1H =h;
	UBRR1L =l;
	UCSR1B =b;
	UCSR1C =c;
	do {
		UDR1;
	} while (UCSR1A & (1 << RXC1));
	do {
		UDR0;
	} while (UCSR0A & (1 << RXC0));
}
void uart_TXchar(uint8_t c) {
	/*if(channel==0){
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = c;
	/}else if(channel==1){
		while (!(UCSR1A & (1 << UDRE1)));
		UDR1 = c;
	}else{*/
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = c;
		while (!(UCSR1A & (1 << UDRE1)));
		UDR1 = c;
	//}
}

uint8_t uart_RXchar(void) {
	
	/*if(channel==0){
		while (!(UCSR0A & (1 << RXC0)));
		return UDR0;
	}else if(channel == 1){
		while (!(UCSR1A & (1 << RXC1))) {
		}
		return UDR1;
	}else{*/
		while ((!(UCSR1A & (1 << RXC1))) && (!(UCSR0A & (1 << RXC0))));
		if((UCSR1A & (1 << RXC1))){
			channel=1;
			return UDR1;
		}else{
			channel=0;
			return UDR0;
		}
	//}
}

