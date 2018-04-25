from __future__ import print_function
import serial
import time

class Wrapper:
    
    def __init__(self, portName):
        try:
            self.port = serial.Serial(portName, baudrate=115200,timeout=5)
        except serial.SerialException:
            print("ERROR: Port " + portName + " is already open!\n")
    
    def __enter__(self):
        if(self.port.closed):
            self.port.open()
        return self
    
    def __exit__(self, exception_type, exception_value, traceback):
        self.port.close()
    
    def inWaiting(self):
        return self.port.inWaiting()
    
    def readLine(self):
        dat = self.port.readline()
        return dat.strip()

