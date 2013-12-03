#ifndef __UPDATE_H__
#define __UPDATE_H__

#define INTERNAL_FLASH_SIZE 1000

#define SD_UPDATE 0
#define FLASH_UPDATE 0
#define UPDATE_EVERYTIME 0
#define BACKUP 0
#define DBG 0

#define BOOTLOADER_STORAGE_HEADER_ADDR 4096-2
#define BOOTLOADER_STORAGE_INFO_ADDR 4096-1

/*FLAGS*/
#define NEW_FLAG 0
#define SUCCESS_FLAG 1
#define PROCESS_FLAG 2



// // //

struct update_s {
	uint16_t size; // size in blocks
	uint32_t addr; // address in byte
	uint8_t flags; 
	uint8_t fail_count;
	uint8_t last;
	uint16_t crc_sum;
} __attribute__((__packed__));

typedef struct update_s update_t;

#endif
