#ifndef __UPDATE_SD_H__
#define __UPDATE_SD_H__
#define FORMAT 1

#define MAGIC_NUM 5
#include <stdint.h>
uint8_t update_validate (uint8_t method, uint32_t header_addr);
uint16_t update_backup (uint8_t method, uint32_t header_addr, uint32_t backup_addr);
uint8_t update_install (uint8_t method, uint32_t header_addr);

#if FORMAT
void update_sd_format(void);
#endif

#endif