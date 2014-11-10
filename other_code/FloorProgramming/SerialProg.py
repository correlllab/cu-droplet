import MySerialWrapper as Serial
import time

PROGRAM_DELAY = 6.0 #seconds
programTime = PROGRAM_DELAY-0.50
bytespersec = Serial.baud/8
secondsPerLine = (21./bytespersec) #21 is the no. of bytes of each line
lineCount = int(PROGRAM_DELAY*0.9/secondsPerLine)

#need to set the two lines below based on your system.
port = Serial.open_serial_port("COM7")
hexFilePath = "C:\Users\Kailaash\Documents\Atmel Studio\Droplets\Droplets\Debug\Droplets.hex"

Serial.set_global_port(port)

#count = lineCount
now = 0
def process_and_transmit_line(line):
    byte_strs = [str(line[i])+str(line[i+1]) for i in range(1,len(line)-1,2)]
    byte_sum=0
    for b in byte_strs:
        byte_sum+=int(b,16)
    byte_sum-=int(byte_strs[-1],16)
    if(format(0xFF&((0xFF^(byte_sum & 0xFF))+1),'02X') != byte_strs[-1]):
        print("checksum mismatch?")
        print("\t" + format(0xFF&((0xFF^(byte_sum & 0xFF))+1),'02X'))
        print("\t" + byte_strs[-1])
    dat = ''.join([chr(int(b,16)) for b in byte_strs])
    #print(dat)
    Serial.write(dat)
    
try:
    with open(hexFilePath,"r") as f:
        for line in f:
            if((time.time()-now)>programTime):
                while not Serial.check_chars():
                    1+1 #do nothing
                now=time.time()
            process_and_transmit_line(line)
except(KeyboardInterrupt, SystemExit):
    Serial.cleanup()
    raise

#print(time.time()-now)
Serial.cleanup()
