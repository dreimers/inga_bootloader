#ifndef __UPDATE_SD_H__
#define __UPDATE_SD_H__
#define FORMAT 1

#define MAGIC_NUM 5
uint8_t update_sd_validate (uint32_t header_addr);
uint16_t udate_sd_backup (uint32_t header_addr, uint32_t backup_addr);
uint8_t update_sd_install (uint32_t header_addr);

#if FORMAT
void update_sd_format(void);
#endif

#endif