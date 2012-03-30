#ifndef __UPDATE_H__
#define __UPDATE_H__

// // //
typedef struct {
	uint16_t size; // size in blocks
	uint32_t addr; // address in byte
	uint8_t flags;
} update_t;

extern update_t update;
#endif