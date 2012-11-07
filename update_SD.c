#include "flash-microSD.h"
#include "update_SD.h"
#include "update.h"
#include <stdint.h>
#include "flash-mgr.h"
#include "flash-at45db.h"
#include <string.h>

#include "uart.h"
#include "crc16.h"

#define LED_1_ON()		PORTD &=~(1 << PD5)
#define LED_2_ON()		PORTD &=~(1 << PD7)
#define LED_1_OFF()		PORTD |= (1 << PD5)
#define LED_2_OFF()		PORTD |= (1 << PD7)
#define LED_1_TOGGLE()		PORTD ^= (1 << PD5)
#define LED_2_TOGGLE()		PORTD ^= (1 << PD7)
update_t update;


void (*update_read_block[2]) (uint32_t , uint8_t *) = {at45db_read_page_bypassed, microSD_read_block};
void (*update_write_block[2]) (uint32_t , uint8_t *) = {at45db_write_page, microSD_write_block};

uint8_t g_pos;

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
	update_write_block[1] ( ( (uint32_t) size + addr) * 512, buff);

}
#endif
#if BACKUP
uint16_t update_backup (uint8_t method, uint32_t header_addr, uint32_t backup_addr)
{
	update_t bk;
	uint32_t i = 0;
	uint8_t buff[512];
	update_read_block[method] (header_addr, buff);
	bk.addr = backup_addr;
	uint32_t addr = 0;
	bk.size = 0;
	for (; i < INTERNAL_FLASH_SIZE; i += 512) {
		bk.size++;
		page_read (512, 'F', &addr, buff);
		update_write_block[method] (backup_addr + (bk.size * 512), buff);
	}
	bk.flags = 1;
	bk.success_count = 0;
	uint16_t pos=9;
	while(buff[pos]!=1){
		pos+=9
	}
	buff[pos]=0;
	
	
	//size
	* ( (uint16_t *) &buff[pos+1]) = bk.size;
	//addr
	* ( (uint32_t *) &buff[pos+3]) = bk.addr;
	//flags
	buff[pos+7] = bk.flags;
	//success_count
	buff[pos+8] = (bk.success_count) & 0xff;
	buff[pos+9] = 1;

	//memcpy (&buff[1], &bk, sizeof (update_t));
	update_write_block[method] (header_addr, buff);
	update_write_block[method] ( ( (uint32_t) bk.size bk.addr) * 512, buff);

}
#endif

uint8_t update_validate (uint8_t method, uint32_t header_addr, uint8_t pos)
{
	uint8_t buff[512];
	update_read_block[method] (header_addr, buff);
	uint16_t d_count=0;
	if (buff[0] == MAGIC_NUM) {

		pos*=sizeof(update_t);
		g_pos=pos;
		update.size = * ( (uint16_t *) &buff[pos+1]);
		
		update.addr = * ( (uint32_t *) &buff[pos+3]);
		update.flags = buff[pos+7];
		update.fail_count = buff[pos+8]+1;
		update.last = buff[pos+9];
		update.crc_sum = * ( (uint16_t *) &buff[pos+10]);
		if(update.flags & (1<<NEW_FLAG)){
			uint16_t i=0;
			uint16_t  crc=0;
			for (; i < update.size; i++) {
				//LED_1_TOGGLE();
				update_read_block[method] (update.addr + i, buff);
			/*	d_count=0;
				for(;d_count<512;d_count++){
					uart_TXchar (buff[d_count]);
				}
			*/
				crc=crc16_calc(buff,511,crc);
			}
			crc=crc16_calc((uint8_t*)&update.crc_sum,1,crc);
			return crc;
		}else if(!(update.flags & (1<<SUCCESS_FLAG)) && !method){
			//update may failed 
			return 4;
		}
		return 3;
		

	} else {
		return 2;
	}

	return 1; //failure
}

uint8_t update_install (uint8_t method, uint32_t header_addr)
{
	uint32_t flash_addr = 0;
	uint16_t buff[256];
	if(!method){
		at45db_read_page_bypassed(header_addr,(uint8_t *) buff);
		update_t *tmp;
		tmp=(update_t*)(((uint8_t*)buff)+g_pos+1);
		tmp->flags|=PROCESS_FLAG;
		tmp->fail_count=update.fail_count;
		at45db_erase_page(header_addr);
		at45db_write_page(header_addr,(uint8_t *) buff);
	}
	uint16_t i = 0;
	//_delay_ms(500);
	erase_flash();
#if BACKUP
	update_backup (method, 512, (update.addr + update.size) * 512);
#endif
	//uart_TXchar (update.size >> 8);
	//uart_TXchar (update.size);
	for (; i < update.size; i++) {
		LED_2_TOGGLE();
		//uart_TXchar('A');
		update_read_block[method] (update.addr + i, (uint8_t *) buff);
		//uart_TXchar('M');
		page_write (PAGESIZE, buff, 'F', &flash_addr);
		//uart_TXchar('W');
		page_write (PAGESIZE, buff + (PAGESIZE/2), 'F', &flash_addr);
		//uart_TXchar('E');
	}
	if(!method){
		at45db_read_page_bypassed(header_addr,(uint8_t *) buff);
		update_t *tmp;
		tmp = (update_t *)(((uint8_t *)buff)+1+g_pos);
		tmp->flags &= ~(1<<PROCESS_FLAG);
		at45db_write_page(header_addr,(uint8_t *) buff);
	}
	
	//update.success_count++;
	//memset (buff, 0, 512);
	//buff[0] = MAGIC_NUM;
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
	//memcpy (&buff[1], &update, sizeof (update_t));
//	update_write_block[method] (header_addr, buff);
//	update_write_block[method] ( ( (uint32_t) update.size * 512) + update.addr, buff);
}
