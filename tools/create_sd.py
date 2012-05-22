#!/usr/bin/python
import sys
import struct
def crc16(data,acc):
	#acc ^=  i
	#for y in xrange(8,0):
	#	if ((acc & 0x8000)==0x8000):
	#		acc= (acc <<1) ^ 0x1021
	#	else:
	#		acc= acc << 1
	#return acc
	
	
	acc ^= data & 0xff
	acc  = ((acc >> 8)&0xffff) | ((acc << 8)&0xffff)
	acc ^= ((acc & 0xff00) << 4)&0xffff
	acc ^= ((acc >> 8) >> 4)&0xffff
	acc ^= ((acc & 0xff00) >> 5)&0xffff
	return (acc&0xffff)


magic_num=5
start_address=1
flags=0
if (sys.argv[1] == "TEST"):
	of=open(sys.argv[2],'wb')
	for j in range(256):
		for i in range(512):
			s="%c" % (j%255)
			of.write(s)
	of.close()
else:
	with open(sys.argv[1], "rb") as f:
		byte=f.read(1)
		byte_count=0
		crc=0
		byte_array= []
		while byte != "":
			byte_count+=1
			byte_array.append(byte)
			value = struct.unpack('B', byte)[0]
			crc=crc16(value &0xff,crc&0xffff)
			#crc=crc16(byte,crc)
			byte=f.read(1)
		block_count= byte_count/512
		for i in range(512-(byte_count%512)):
			crc=crc16(0 &0xff,crc&0xffff)
		#crc=crc16(0,crc)
		print block_count
		#crc=crc16(0,crc)
		print crc
		if byte_count%512 >0:
			block_count+=1
		print "size="+str(byte_count)+", "+str(block_count)+" blocks used"
		header=[magic_num]
		header.append((block_count)&0xff)
		header.append((block_count>>8)&0xff)
		header.append((start_address)&0xff)
		header.append((start_address>>8)&0xff)
		header.append((start_address>>16)&0xff)
		header.append((start_address>>24)&0xff)
		header.append(flags)
		header.append(0)
		header.append(1)
		header.append(crc&0xff)
		header.append((crc>>8)&0xff)
		print header
		of=open(sys.argv[2],'wb')
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



