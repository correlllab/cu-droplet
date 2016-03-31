from __future__ import print_function
import serial

class SerialInterface:
    def __init__(self):
        self.port = None
        self.in_dat = ''

    def open(self, port_name):
        """ Function init(port_name): Opens the serial port supplied as input 
        (e.g. 'COM9') and tests to make sure it's working. WARNING: The caller 
        of this function is responsible for closing the serial port connection 
        using <returned_port_handle>.close() when they are done.
        Returns True if the port was succesfully opened, False otherwise.
        """
        try:
            self.port = serial.Serial(port_name, baudrate=115200, timeout=5)
        except serial.SerialException:
            print("ERROR: Port " + port_name + " is already open!\n")
            return False

        if(self.port.closed):
            self.port.open()
            
        return True
    
    def read(self):
        gotDat = False
        while self.port.inWaiting() > 0:
            dat = self.port.readline()
            if len(dat.strip()) > 0:
                print('\t\t', end='')
                print(dat.strip())
                gotDat=True
        return gotDat
     
    def write(self, data):
        out_dat = data + '\n';
        self.port.write(out_dat)
        self.port.flushOutput()
        self.read()
    
    def close(self):
        """
        Close the serial port
        """
        self.port.close()
