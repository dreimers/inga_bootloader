/* CITT CRC16 polynomial ^16 + ^12 + ^5 + 1 */
#include <avr/io.h>
uint16_t crc16_calc(uint8_t *data, uint16_t length, uint16_t acc)
{
	do{
		acc ^= *data;
		acc  = (acc >> 8) | (acc << 8);
		acc ^= (acc & 0xff00) << 4;
		acc ^= (acc >> 8) >> 4;
		acc ^= (acc & 0xff00) >> 5;
		data++;
	}while (length--);
	return acc;
}