#include "flash-microSD.h"
#include "update_SD.h"
#include "update.h"

update_t update;

uint8_t update_sd_validate (void)
{
	uint8_t buff[512];
	if (microSD_read_block(0,buff) == 0) {
		if ( buff[0] == MAGIC_NUM){
			update.size = (buff[1]<<1) + buff[2];
			update.addr = (buff[3]<<3) + (buff[4]<<2) + (buff[5]<<1) +buff[6];
			update.flags = buff[7];
			microSD_read_block(((uint32_t)update.size*512)+update.addr, buff);
			if ( (buff[0] == MAGIC_NUM) &&\
				(update.size == (buff[1]<<1) + buff[2]) &&\
				(update.addr == (buff[3]<<3) + (buff[4]<<2) + (buff[5]<<1) +buff[6])){
				return 0;
			}
				
				
		}
	}
	return 1;
}