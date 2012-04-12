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
 * \defgroup fat_driver FAT Driver
 *
 * <p></p>
 * @{
 *
 */

/**
 * \file
 *		FAT driver definitions
 * \author
 *      Christoph Peltz <peltz@ibr.cs.tu-bs.de>
 */
 
#ifndef _FAT_H_
#define _FAT_H_

#include "diskio.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "cfs/cfs.h"

#define FAT12 0
#define FAT16 1
#define FAT32 2
#define FAT_INVALID 3

#define EOC 0x0FFFFFFF

#define FAT_FD_POOL_SIZE 5

struct FAT_Info {
	uint8_t type; /** Either FAT16, FAT32 or FAT_INVALID */
	uint16_t BPB_BytesPerSec;
	uint8_t BPB_SecPerClus;
	uint16_t BPB_RsvdSecCnt;
	uint8_t BPB_NumFATs;
	uint16_t BPB_RootEntCnt;
	uint32_t BPB_TotSec;
	uint8_t BPB_Media;
	uint32_t BPB_FATSz;
	uint32_t BPB_RootClus; /** only valid for FAT32 */
};

struct dir_entry {
	uint8_t DIR_Name[11];
	uint8_t DIR_Attr;
	uint8_t DIR_NTRes;
	uint8_t CrtTimeTenth;
	uint16_t DIR_CrtTime;
	uint16_t DIR_CrtDate;
	uint16_t DIR_LstAccessDate;
	uint16_t DIR_FstClusHI;
	uint16_t DIR_WrtTime;
	uint16_t DIR_WrtDate;
	uint16_t DIR_FstClusLO;
	uint32_t DIR_FileSize;
};

struct file {
	//metadata
	/** Cluster Position on disk */
	uint32_t cluster;
	uint32_t dir_entry_sector;
	uint16_t dir_entry_offset;
	struct dir_entry dir_entry;
	uint32_t size;
	uint32_t nth_cluster;
	uint32_t n;
};

struct file_desc {
	cfs_offset_t offset;
	struct file *file;
	uint8_t flags;
};

int mkfs_fat( struct diskio_device_info *dev );

int fat_mkdir(char *);
int fat_rmdir(char *);

/**
 * Tries to mount the defined device.
 *
 * \param dev The device on which a FAT-FS should be mounted.
 * \return 0 on success, 1 if the bootsector was not found or corrupted, 2 if the FAT-Type wasn't supported.
 */
uint8_t fat_mount_device( struct diskio_device_info *dev );

/**
 * Umounts the mounted device. Invalidates all file descriptors.
 * Syncs all FATs and flushes cached data.
 */
void fat_umount_device(void);

/**
 * Populates the give FAT_Info with the mounted FAT_Info.
 *
 * \param *info The FAT_Info struct which should be populated.
 */
void get_fat_info( struct FAT_Info *info );

/**
 * Syncs every FAT with the first FAT. Can take much time.
 */
void fat_sync_fats(void);

uint32_t get_free_cluster(uint32_t start_cluster);
void write_fat_entry( uint32_t cluster_num, uint32_t value );
void fat_flush(void);
uint32_t fat_file_size(int fd);
#endif