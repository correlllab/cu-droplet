import MySerialWrapper as Serial
import time


#need to set the two lines below based on your system.
port = Serial.open_serial_port("COM13")
hexFilePath = "C:\Users\John Klingner\Documents\Atmel Studio\\6.1\NewDroplets\NewDroplets\Debug\NewDroplets.hex"

Serial.set_global_port(port)
now = time.time()

with open(hexFilePath,"r") as f:
    for line in f:
        byte_strs = [line[i]+line[i+1] for i in range(1,len(line)-1,2)]
        byte_sum=0
        for b in byte_strs:
            byte_sum+=int(b,16)
        byte_sum-=int(byte_strs[-1],16)
        if(format((0xFF^(byte_sum & 0xFF))+1,'X') != byte_strs[-1]):
            print("checksum mismatch?")
            print("\t" + format((0xFF^(byte_sum & 0xFF))+1,'X'))
            print("\t" + byte_strs[-1])
        dat = ''.join([chr(int(b,16)) for b in byte_strs])
        print(dat)
        Serial.write(dat)
        break

print(time.time()-now)
Serial.cleanup()
