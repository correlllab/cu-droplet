from __future__ import print_function
import serial
import time

port = "COM3"
baud = 115200

ser = serial.Serial(port, baud, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,timeout=2)
global open_port
open_port = ser


def write(data):
    out_dat = data
    open_port.write(out_dat)
    open_port.flushOutput()

def my_func(Port):
	#Port.write("reprog_begin\n")
	#time.sleep(1)
	rec = []
	with open('C:/Users/niharika/Documents/Atmel Studio/7.0/cu-droplet-master/droplet_code/build/My_Droplets.lss','r') as ref:
		for linesLSS in ref:
			if ".USERCODE" in linesLSS:
				store = linesLSS.split()
				section = store[1]
				size = store[2]
				start_address = store[3]
				start_address = start_address.lstrip('0')
				start_address = start_address.upper()
				print(section, size, start_address)
				size = int(size, 16)
				if (size <= 16):
					offset = 1
					print(offset)
				elif (size%16 == 0) :
					offset = size/16
					print(offset)
				else : 
					offset = size/16 + 1
					print(offset)
				break
		
	first_msg = 'R ' + str(offset) + '\n'
	Port.write(first_msg)	
	time.sleep(1)
	
	with open('C:/Users/niharika/Documents/Atmel Studio/7.0/cu-droplet-master/droplet_code/build/My_Droplets.hex','r') as hexFile:
		for linesHEX in hexFile:
			linesHEX = linesHEX.strip(':')
			if (linesHEX[2:6] == start_address):
				while(offset > 0):
					linesHEX = 'M ' + linesHEX[0:6] + linesHEX[8:]
					Port.write(linesHEX)
					time.sleep(1)
					offset = offset - 1
					linesHEX = hexFile.next()
					linesHEX = linesHEX.strip(':')
					if(offset == 0) :
						break
					

				while(rec != 'Done'):
					while(Port.inWaiting() > 0):
						rec = Port.readline()
						rec = rec.strip()
						print(rec)
						if(rec == 'Done'):
							break
					
			
				
	


if open_port.isOpen():
	print(ser.name + 'is open......')
	my_func(open_port)
	
	
	