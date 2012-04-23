#ifndef __UPDATE_H__
#define __UPDATE_H__

#define INTERNAL_FLASH_SIZE 1000

#define UPDATE_EVERYTIME 1
#define BACKUP 1
#define DBG 0

// // //

#if DBG
typedef struct {
	uint16_t size; // size in blocks
	uint32_t addr; // address in byte
	uint8_t flags;
	uint16_t success_count;
} update_t;
#endif

#endif