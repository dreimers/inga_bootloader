﻿/* Copyright (c) 2012, Christoph Peltz
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \addtogroup Device Interfaces
 * @{
 *
 * \addtogroup diskio_layer
 * @{
 */

/**
 * \file
 *      DiskIO Abstraction Layer Implementation
 * \author
 *      Christoph Peltz <peltz@ibr.cs.tu-bs.de>
 */
 
#include "diskio.h"
#include "mbr.h"
#include <string.h>
#include "../flash-microSD.h"
#include "../flash-at45db.h"

static struct diskio_device_info *default_device = 0;
static struct diskio_device_info devices[DISKIO_MAX_DEVICES];

#define DISKIO_OP_WRITE_BLOCK  1
#define DISKIO_OP_READ_BLOCK   2
#define DISKIO_OP_WRITE_BLOCKS 3
#define DISKIO_OP_READ_BLOCKS  4

void print_device_info( struct diskio_device_info *dev ) {
	printf("\nDiskIO Device Info\n");
	printf("\ttype = ");
	switch(dev->type & 0x7F) {
		case DISKIO_DEVICE_TYPE_SD_CARD:
			printf("SD_CARD");
			break;
		case DISKIO_DEVICE_TYPE_GENERIC_FLASH:
			printf("Generic_Flash");
			break;
		default:
			printf("Unknown");
	}
	if(dev->type & DISKIO_DEVICE_TYPE_PARTITION)
		printf(" (Partition)");
	printf("\n");
	printf("\tnumber = %d\n", dev->number);
	printf("\tpartition = %d\n", dev->partition);
	printf("\tnum_sectors = %ld\n", dev->num_sectors);
	printf("\tsector_size = %d\n", dev->sector_size);
	printf("\tfirst_sector = %ld\n", dev->first_sector);
}

int diskio_rw_op( struct diskio_device_info *dev, uint32_t block_start_address, uint8_t num_blocks, uint8_t *buffer, uint8_t op );

int diskio_read_block( struct diskio_device_info *dev, uint32_t block_address, uint8_t *buffer ) {
	return diskio_rw_op( dev, block_address, 1, buffer, DISKIO_OP_READ_BLOCK );
}

int diskio_read_blocks( struct diskio_device_info *dev, uint32_t block_start_address, uint8_t num_blocks, uint8_t *buffer ) {
	return diskio_rw_op( dev, block_start_address, num_blocks, buffer, DISKIO_OP_READ_BLOCKS );
}

int diskio_write_block( struct diskio_device_info *dev, uint32_t block_address, uint8_t *buffer ) {
	return diskio_rw_op( dev, block_address, 1, buffer, DISKIO_OP_WRITE_BLOCK );
}

int diskio_write_blocks( struct diskio_device_info *dev, uint32_t block_start_address, uint8_t num_blocks, uint8_t *buffer ) {
	return diskio_rw_op( dev, block_start_address, num_blocks, buffer, DISKIO_OP_WRITE_BLOCKS );
}

int diskio_rw_op( struct diskio_device_info *dev, uint32_t block_start_address, uint8_t num_blocks, uint8_t *buffer, uint8_t op ) {
	if( dev == NULL ) {
		if( default_device == 0 )
			return DISKIO_ERROR_NO_DEVICE_SELECTED;
		dev = default_device;
	}
	block_start_address += dev->first_sector;
	switch( dev->type & DISKIO_DEVICE_TYPE_MASK ) {
		uint8_t ret_code = 0;
		uint8_t tries = 0, reinit = 0;
		case DISKIO_DEVICE_TYPE_SD_CARD:
			switch( op ) {
				case DISKIO_OP_READ_BLOCK:
					#ifndef DISKIO_OLD_STYLE
					for(tries = 0; tries < 50; tries++) {
						ret_code = microSD_read_block( block_start_address, buffer );
						if( ret_code == 0 ) {
							return DISKIO_SUCCESS;
						}
						_delay_ms(1);
						if( reinit == 0 && tries == 49 ) {
							tries = 0;
							reinit = 1;
							microSD_init();
						}
					}
					#ifdef DEBUG
					printf("\ndiskion_rw_op(): Unrecoverable Error!");
					#endif
					return DISKIO_ERROR_INTERNAL_ERROR;
					#else
					if( microSD_read_block( block_start_address, buffer ) == 0) {
						return DISKIO_SUCCESS;
					}
					return DISKIO_ERROR_TRY_AGAIN;
					#endif
					break;
				case DISKIO_OP_READ_BLOCKS:
					return DISKIO_ERROR_TO_BE_IMPLEMENTED;
					break;
				case DISKIO_OP_WRITE_BLOCK:
					#ifndef DISKIO_OLD_STYLE
					for(tries = 0; tries < 50; tries++) {
						ret_code = microSD_write_block( block_start_address, buffer );
						if( ret_code == 0 ) {
							return DISKIO_SUCCESS;
						}
						_delay_ms(1);
						if( reinit == 0 && tries == 49 ) {
							tries = 0;
							reinit = 1;
							microSD_init();
						}
					}
					#ifdef DEBUG
					printf("\ndiskion_rw_op(): Unrecoverable Error!");
					#endif
					return DISKIO_ERROR_INTERNAL_ERROR;
					#else
					if( microSD_write_block( block_start_address, buffer ) == 0) {
						return DISKIO_SUCCESS;
					}
					return DISKIO_ERROR_TRY_AGAIN;
					#endif
					break;
				case DISKIO_OP_WRITE_BLOCKS:
					return DISKIO_ERROR_TO_BE_IMPLEMENTED;
					break;
				default:
					return DISKIO_ERROR_OPERATION_NOT_SUPPORTED;
					break;
			}
			break;
		case DISKIO_DEVICE_TYPE_GENERIC_FLASH:
			switch( op ) {
				case DISKIO_OP_READ_BLOCK:
					at45db_read_page_bypassed( block_start_address, 0, buffer, 512 );
					return DISKIO_SUCCESS;
					break;
				case DISKIO_OP_READ_BLOCKS:
					return DISKIO_ERROR_TO_BE_IMPLEMENTED;
					break;
				case DISKIO_OP_WRITE_BLOCK:
					at45db_write_buffer( 0, buffer, 512 );
					at45db_buffer_to_page( block_start_address );
					return DISKIO_SUCCESS;
					break;
				case DISKIO_OP_WRITE_BLOCKS:
					return DISKIO_ERROR_TO_BE_IMPLEMENTED;
					break;
				default:
					return DISKIO_ERROR_OPERATION_NOT_SUPPORTED;
					break;
			}			
			break;
		case DISKIO_DEVICE_TYPE_NOT_RECOGNIZED:
		default:
			return DISKIO_ERROR_DEVICE_TYPE_NOT_RECOGNIZED;
	}
	return DISKIO_SUCCESS;
}

void diskio_set_default_device( struct diskio_device_info *dev ) {
	default_device = dev;
}

struct diskio_device_info * diskio_devices() {
	return devices;
}

int diskio_detect_devices() {	
	struct mbr mbr;
	int dev_num = 0;
	int i = 0, index = 0;

	memset( devices, 0, DISKIO_MAX_DEVICES * sizeof(struct diskio_device_info) );
	if( at45db_init() == 0 ) {
		devices[index].type = DISKIO_DEVICE_TYPE_GENERIC_FLASH;
		devices[index].number = dev_num;
		/* This Flash has 4096 Pages */
		devices[index].num_sectors = 4096;
		/* A Page is 528 Bytes long, but for easier acces we use only 512 Byte*/
		devices[index].sector_size = 512;
		devices[index].first_sector = 0;
		index += 1;
	}

	if( microSD_init() == 0 ) {
		devices[index].type = DISKIO_DEVICE_TYPE_SD_CARD;
		devices[index].number = dev_num;
		devices[index].num_sectors = microSD_get_block_num();
		devices[index].sector_size = microSD_get_block_size();
		devices[index].first_sector = 0;
		if( devices[index].sector_size > DISKIO_MAX_SECTOR_SIZE ) {
			goto end_of_function;
		}

		mbr_init( &mbr );
		mbr_read( &devices[index], &mbr );
		index += 1;
		for( i = 0; i < 4; ++i ) {
			if( mbr_hasPartition( &mbr, i + 1 ) != 0 ) {
				devices[index].type = DISKIO_DEVICE_TYPE_SD_CARD | DISKIO_DEVICE_TYPE_PARTITION;
				devices[index].number = dev_num;
				devices[index].partition = i + 1;
				devices[index].num_sectors = mbr.partition[i].lba_num_sectors;
				devices[index].sector_size = devices[dev_num].sector_size;
				devices[index].first_sector = mbr.partition[i].lba_first_sector;
				index += 1;
			}
		}

		dev_num += 1;
		index += 1;
	}

	end_of_function:

	if(index == 0)
		return DISKIO_FAILURE;

	return DISKIO_SUCCESS;
}
