#ifndef __UPDATE_SD_H__
#define __UPDATE_SD_H__
#define FORMAT 1

#define MAGIC_NUM 5
uint8_t update_sd_validate (uint32_t header_addr);

#if FORMAT
void update_sd_format(void);
#endif

#endif