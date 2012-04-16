#!/usr/bin/python
import sys
magic_num=5
start_address=1
flags=0
with open(sys.argv[1], "rb") as f:
	byte=f.read(1)
	byte_count=0
	byte_array= []
	while byte != "":
		byte_count+=1
		byte_array.append(byte)
		byte=f.read(1)
	block_count= byte_count/512
	if byte_count%512 >0:
		block_count+=1
	print "size="+str(byte_count)+", "+str(block_count)+" blocks used"
	header=[magic_num,(block_count>>8)&0xff,block_count&0xff]
	header.append((start_address>>24)&0xff)
	header.append((start_address>>16)&0xff)
	header.append((start_address>>8)&0xff)
	header.append((start_address)&0xff)
	header.append(flags)
	header.append(0)
	print header
	of=open("test.bin",'wb')
	write_count=0
	for i in range(len(header)):
		s="%c" % (header[i])
		of.write(s)
	for i in range(512-len(header)):
		s="%c" % (0)
		of.write(s)
	for i in range(len(byte_array)):
		of.write(byte_array[i])
		write_count+=1
	for i in range((block_count*512)-write_count):
		s="%c" % (0)
		of.write(s)
	for i in range(len(header)):
		s="%c" % (header[i])
		of.write(s)
	of.close()



