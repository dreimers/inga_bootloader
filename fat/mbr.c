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
 * \addtogroup Drivers
 * @{
 *
 * \addtogroup mbr_driver
 * @{
 */

/**
 * \file
 *      MBR Support implementation
 * \author
 *      Christoph Peltz <peltz@ibr.cs.tu-bs.de>
 */
 
#include "mbr.h"
#include <string.h>
#include <stdio.h>

uint8_t mbr_buffer[512];

void mbr_init( struct mbr *mbr ) {
	int i = 0;
	for( i = 0; i < 4; ++i ) {
		// Everything else is set to 0
		memset( (&(mbr->partition[i])), 0, 16 );		
		mbr->partition[i].status = 0x01;
	}
}

int mbr_read( struct diskio_device_info *from, struct mbr *to ) {
	int ret = diskio_read_block( from, 0, mbr_buffer );
	int i = 0;
	if( ret != 0 ) {
		return MBR_ERROR_DISKIO_ERROR;
	}
	/*test if 0x55AA is at the end, otherwise it is no MBR*/
	if( mbr_buffer[510] == 0x55 && mbr_buffer[511] == 0xAA ) {
		for( i = 0; i < 4; ++i ) {
			memcpy( &(to->partition[i]), &(mbr_buffer[446 + 16 * i]), 16 );
		}
		return MBR_SUCCESS;
	}
	return MBR_ERROR_NO_MBR_FOUND;
}

int mbr_write( struct mbr *from, struct diskio_device_info *to ) {
	int i = 0;
	memset( mbr_buffer, 0, 512 );
	for( i = 0; i < 4; ++i ) {
		memcpy( &(mbr_buffer[446 + 16 * i]), &(from->partition[i]), 16 );
	}
	mbr_buffer[510] = 0x55;
	mbr_buffer[511] = 0xAA;
	return diskio_write_block( to, 0, mbr_buffer );
}

int mbr_addPartition(struct mbr *mbr, uint8_t part_num, uint8_t part_type, uint32_t start, uint32_t len ) {
	int ret = 0;
	uint8_t sectors_per_track = 63, heads_per_cylinder = 16;
	uint16_t cylinder = 0;
	
	ret = mbr_hasPartition( mbr, part_num );
	if( ret != 0 ) {
		return MBR_ERROR_PARTITION_EXISTS;
	}
	
	mbr->partition[part_num - 1].status = 0x00;
	
	cylinder = start / (sectors_per_track * heads_per_cylinder);
	mbr->partition[part_num - 1].chs_first_sector[0] = (start / sectors_per_track) % heads_per_cylinder;
	mbr->partition[part_num - 1].chs_first_sector[1] = ((uint8_t)((cylinder  >> 8) << 6)) + ((start % sectors_per_track) + 1);
	mbr->partition[part_num - 1].chs_first_sector[2] = (uint8_t) cylinder;
	
	mbr->partition[part_num - 1].type = part_type;
	
	// the end address isn't capable of fitting into a uint32_t
	// or the end address is further out then chs addressing is capable of
	// 1023*254*63 is the max value for chs
	if( start + len <= start || start + len >= ((uint32_t) 1023)*254*63 ) {
		mbr->partition[part_num - 1].chs_last_sector[0] = 254;
		mbr->partition[part_num - 1].chs_last_sector[1] = ((uint8_t)((1023 >> 8) << 6)) + 63;
		mbr->partition[part_num - 1].chs_last_sector[2] = (uint8_t) 1023;
	} else {
		cylinder = (start + len) / (sectors_per_track * heads_per_cylinder);
		mbr->partition[part_num - 1].chs_last_sector[0] = ((start + len)/ sectors_per_track) % heads_per_cylinder;
		mbr->partition[part_num - 1].chs_last_sector[1] = ((uint8_t)((cylinder  >> 8) << 6)) + (((start + len)% sectors_per_track) + 1);
		mbr->partition[part_num - 1].chs_last_sector[2] = (uint8_t) cylinder;
	}	
	mbr->partition[part_num - 1].lba_first_sector = start;
	mbr->partition[part_num - 1].lba_num_sectors = len;
	return MBR_SUCCESS;
}

int mbr_delPartition(struct mbr *mbr, uint8_t part_num) {
	if( part_num > 4 || part_num < 1 ) {
		return MBR_ERROR_INVALID_PARTITION;
	}
	// Set Status to invalid (everything other than 0x00 and 0x80 is invalid
	mbr->partition[part_num - 1].status = 0x01;
	// Everything else is set to 0
	memset( (&(mbr->partition[part_num - 1])) + 1, 0, 15 );
	return MBR_SUCCESS;
}

int mbr_hasPartition(struct mbr *mbr, uint8_t part_num) {
	if( part_num > 4 || part_num < 1 ) {
		return 0;
	}
	if( mbr->partition[part_num - 1].status != 0x00 && mbr->partition[part_num - 1].status != 0x80 ) {
		return 0;
	}
	if( mbr->partition[part_num - 1].type == 0 )
		return 0;
	return 1;
}
