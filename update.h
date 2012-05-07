#ifndef __UPDATE_H__
#define __UPDATE_H__

#define INTERNAL_FLASH_SIZE 1000

#define UPDATE_EVERYTIME 1
#define BACKUP 0
#define DBG 0

// // //

struct update_s {
	uint16_t size; // size in blocks
	uint32_t addr; // address in byte
	uint8_t flags;
	uint16_t success_count;
} __attribute__((__packed__));

typedef struct update_s update_t;

#endif