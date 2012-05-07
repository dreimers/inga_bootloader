#include "flash-microSD.h"
#include "update_SD.h"
#include "update.h"
#include <stdint.h>
#include "flash-mgr.h"
#include "flash-at45db.h"
#include <string.h>

#include "uart.h"



#define LED_2_TOGGLE()		PORTD ^= (1 << PD7)
update_t update;


void (*update_read_block[2])(uint32_t , uint8_t*)={at45db_read_page_bypassed,microSD_read_block};
void (*update_write_block[2])(uint32_t , uint8_t*)={at45db_write_page,microSD_write_block};



#if FORMAT
void update_format (void)
{
	uint8_t buff[512];
	buff[0] = MAGIC_NUM;
	//size
	buff[1] = 0;
	buff[2] = 0;
	uint32_t size = 0;
	//addr
	buff[3] = 0;
	buff[4] = 0;
	buff[5] = 0;
	buff[6] = 1;
	uint32_t addr = 1;
	//flags
	buff[7] = 0;
	update_write_block[1] (0, buff);
	update_write_block[1] ( ( (uint32_t) size +addr)*512, buff);

}
#endif
#if BACKUP
uint16_t update_backup (uint8_t method, uint32_t header_addr, uint32_t backup_addr)
{
	update_t bk;
	uint32_t i = 0;
	uint8_t buff[512];
	bk.addr = backup_addr;
	uint32_t addr = 0;
	bk.size = 0;
	for (; i < INTERNAL_FLASH_SIZE; i += 512) {
		bk.size++;
		page_read (512, 'F', &addr, buff);
		update_write_block[method] (backup_addr + (bk.size * 512), buff);
	}
	memset (buff, 0, 512);
	buff[0] = MAGIC_NUM;
	bk.flags = 1;
	bk.success_count = 0;
	
	//size
	*((uint16_t*)&buff[1]) = bk.size;
	//addr
	*((uint32_t*)&buff[3]) = bk.addr;
	//flags
	buff[7] = bk.flags;
	//success_count
	buff[8] = (bk.success_count>>8)&0xff;
	buff[9] = (bk.success_count)&0xff;
	
	//memcpy (&buff[1], &bk, sizeof (update_t));
	update_write_block[method] (header_addr, buff);
	update_write_block[method] ( ( (uint32_t) bk.size bk.addr)*512, buff);

}
#endif
uint8_t update_validate (uint8_t method, uint32_t header_addr)
{
	uint8_t buff[512];
	update_read_block[method] (header_addr, buff);
	if (buff[0] == MAGIC_NUM) {
		
		
		update.size = *((uint16_t*)&buff[1]);
		update.addr = *((uint32_t*)&buff[3]);
		update.flags = buff[7];
		update.success_count = ((uint16_t)buff[8]<<8) | buff[9];
		
		
		//memcpy (&buff[1], &update, sizeof (update_t));
		uart_TXchar(update.size);
		update_read_block[method] ( ( (uint32_t) update.size + update.addr+1) , buff);
		uart_TXchar('A');
		uart_TXchar(((update.size + update.addr+1)*512)>>8);
		uart_TXchar(((update.size + update.addr+1)*512));
		if ( (buff[0] == MAGIC_NUM) ){
			if (update.size == *((uint16_t*)&buff[1])){
				uart_TXchar(1);
				if(update.addr == *((uint32_t*)&buff[3])) {
					uart_TXchar(2);
					return 0; //success
				}
			}
				
		} else {
			return 3;
		}


	} else {
		return 2;
		}
	
	return 1; //failure
}

uint8_t update_install (uint8_t method, uint32_t header_addr)
{
	uint32_t flash_addr = 0;
	uint16_t buff[256];
	uint16_t i = 0;
#if BACKUP
	update_backup (method, 512, (update.addr + update.size) * 512);
#endif
	uart_TXchar(update.size>>8);
	uart_TXchar(update.size);
	for (; i < update.size; i += 512) {
		LED_2_TOGGLE();
		update_read_block[method] (update.addr + i, (uint8_t*)buff);
		page_write (PAGESIZE, buff, 'F', &flash_addr);
		page_write (PAGESIZE, buff + PAGESIZE * 2, 'F', &flash_addr);
	}
	update.success_count++;
	memset (buff, 0, 512);
	buff[0] = MAGIC_NUM;
#if 0
	//size
	* ( (uint16_t *) &buff[1]) = update.size;
	//addr
	* ( (uint16_t *) &buff[3]) = update.addr;
	//flags
	buff[7] = update.flags;
	//success_count
	* ( (uint16_t *) &buff[8]) = update.success_count;
#endif
	memcpy (&buff[1], &update, sizeof (update_t));
//	update_write_block[method] (header_addr, buff);
//	update_write_block[method] ( ( (uint32_t) update.size * 512) + update.addr, buff);
}
