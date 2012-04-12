#include "flash-microSD.h"
#include "update_SD.h"
#include "update.h"
#include <stdint.h>
#include "flash-mgr.h"
#include <string.h>

update_t update;

#if FORMAT
void update_sd_format (void)
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
	microSD_write_block (0, buff);
	microSD_write_block ( ( (uint32_t) size * 512) + addr, buff);

}
#endif
uint8_t update_sd_validate (uint32_t header_addr)
{
	uint8_t buff[512];
	if (microSD_read_block (header_addr, buff) == 0) {
		if (buff[0] == MAGIC_NUM) {
			update.size = ( (uint16_t) buff[1] << 8) + buff[2];
			update.addr = ( (uint32_t) buff[3] << 24) + ( (uint32_t) buff[4] << 16) + ( (uint32_t) buff[5] << 8) + buff[6];
			update.flags = buff[7];
			update.success_count = ((uint16_t)buff[8]<<8) + buff[9];
			microSD_read_block ( ( (uint32_t) update.size * 512) + update.addr , buff);
			if ( (buff[0] == MAGIC_NUM) && \
			                (update.size == ( (uint16_t) buff[1] << 8) + buff[2]) && \
			                (update.addr == ( (uint32_t) buff[3] << 24) + ( (uint32_t) buff[4] << 16) + ( (uint32_t) buff[5] << 8) + buff[6])) {
				return 0; //success
			} else {
				return 3;
			}


		} else {
			return 2;
		}
	}
	return 1; //failure
}

uint8_t update_sd_install (uint32_t header_addr)
{
	uint32_t flash_addr = 0;
	uint8_t buff[512];
	uint16_t i = 0;
	for (; i < update.size; i+=512) {
		if (microSD_read_block (update.addr + i, buff) == 0) {
			page_write (PAGESIZE, buff, 'F', &flash_addr);
			page_write (PAGESIZE, buff + PAGESIZE, 'F', &flash_addr);
		} else {
			break;
		}
	}
	update.success_count++;
	memset(buff,0,512);
	buff[0] = MAGIC_NUM;
	//size
	buff[1] = update.size>>8;
	buff[2] = update.size&0xff;
	//addr
	buff[3] = (update.addr>>24)&0xff;
	buff[4] = (update.addr>>16)&0xff;
	buff[5] = (update.addr>>8)&0xff;
	buff[6] = (update.addr)&0xff;
	//flags
	buff[7] = update.flags;
	//success_count
	buff[8] = (update.success_count>>8)&0xff;
	buff[9] = (update.success_count)&0xff;
	microSD_write_block (header_addr, buff);
	microSD_write_block ( ( (uint32_t) update.size * 512) + update.addr, buff);
	
	
	
}
