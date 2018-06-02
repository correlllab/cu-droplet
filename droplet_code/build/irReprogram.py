import serial
import time
import os
import re
from collections import OrderedDict

port = "COM14"
baud = 115200

class DropletReprogrammer:
    
    trackedSections = ['.wrapper', '.usrtxt', '.usrdata']
    sectionsDict = {}
    hexParsingExtendedAddressOffset = 0
    
    def __init__(self, portName, buildName=None):
        self.getFiles(buildName)
        self.readLSSFile()
        self.readHexFile()
        self.getDataFromLinesDict()        
        for section in self.trackedSections:
            print("{}: {}, {} [{}]\r\n\t{}\r\n".format(section, self.sectionsDict[section][0], self.sectionsDict[section][1], len(self.trackedData[section]), self.trackedData[section]))
        self.port = serial.Serial(portName, 115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,timeout=2)
        self.sendData()
        #for (addr, data) in self.linesDict.items():
            #print('{} -> {}'.format(addr, data))
        #print(self.sectionsDict)
    
    
    
    def sendData(self):
        ((wrapperAddr, wrapperLen), (txtAddr, txtLen), (dataAddr, dataLen)) = self.sectionsDict.values()
        (wrapperAddr, wrapperLen, txtAddr, txtLen, dataAddr, dataLen) = map(lambda x: bytearray(x.to_bytes(2, byteorder='little')), (wrapperAddr, wrapperLen, txtAddr, txtLen, dataAddr, dataLen))
        vals = wrapperAddr + wrapperLen + txtAddr + txtLen + dataAddr + dataLen
        self.port.write("ir_prog ".encode('utf-8'))
        self.port.write(vals)
        self.port.write('D\r\n'.encode('utf-8'))
        time.sleep(2)
        #TODO: send all of the data in self.trackedData, and update the droplets code to match.
        
    
    def getDataFromLinesDict(self):
        self.trackedData = OrderedDict()
        for section in self.trackedSections:
            self.trackedData[section] = bytearray()  
            (address, totalSize) = self.sectionsDict[section]
            while totalSize>0:
                line = self.linesDict[address]
                arr = bytearray.fromhex(line)
                self.trackedData[section].extend(arr)
                address += len(arr)
                totalSize -= len(arr)
                if totalSize<0:
                    raise Exception("Unexpected size parsing data from lines dict in section {} ({}, {}, {}, {})\r\n\t{}".format(section, self.sectionsDict[section], totalSize, len(arr),address, line))
            
    
    #This function identifies the .hex and .lss files we'll be reading from.
    def getFiles(self, buildName):
        self.lssFile = None
        self.hexFile = None
        if buildName is None:
            for f in os.listdir('.'):
                if re.match(r'.*?\.lss', f):
                    if self.lssFile is None:
                        self.lssFile = f
                    else:
                        raise Exception("Multiple *.lss files detected! Please resolve ambiguity or specify which you want to use.")
                if re.match(r'.*?\.hex', f):
                    if self.hexFile is None:
                        self.hexFile = f
                    else:
                        raise Exception("Multiple *.hex files detected! Please resolve ambiguity or specify which you want to use.")
            if self.lssFile is None:
                raise Exception("No *.lss files found in working directory. Please specify which file you want to use.")
            if self.hexFile is None:
                raise Exception("No *.hex files found in working directory. Please specify which file you want to use.")
        else:
            self.lssFile = buildName + '.lss'
            self.hexFile = buildName + '.hex'
            if self.lssFile not in os.listdir('.'):
                raise Exception('Specified file ( {} ) not found in working directory.'.format(self.lssFile))
            if self.hexFile not in os.listdir('.'):
                raise Exception('Specified file ( {} ) not found in working directory.'.format(self.lssFile))                
    
    #This function reads in the .lss file and stores the start address and sizes of the sections we need to send.
    def readLSSFile(self):
        self.sectionsDict = {}
        with open(self.lssFile,'r') as ref:
            for line in ref:
                vals = line.strip().split()
                if len(vals) is 7:
                    (idx, section, size, vma, lma, offset, align) = vals
                    if section in self.trackedSections:
                        self.sectionsDict[section] = (int(lma,16), int(size, 16))

    #This function reads in the .hex file and stores each line as a dict of address:value pairs.
    def readHexFile(self):
        self.linesDict = OrderedDict()
        with open(self.hexFile, 'r') as f:
            for line in f:
                result = self.parseHexLine(line)
                if result is not None:
                    (address, data) = result
                    self.linesDict[address] = data
                    prevAddress = address

    #Helper function for readHexFile
    def parseHexLine(self, line):
        line = line.strip()
        match = re.match(r'\:([0-9A-F]{2})([0-9A-F]{4})([0-9A-F]{2})((?:[0-9A-F](?=[0-9A-F]{2}))*)([0-9A-F]{2})', line)
        if match is None:
            print("Error parsing hex line: {}".format(line))
            return None
        
        (length, address, type, data, checksum) = match.group(1, 2, 3, 4, 5)
        (length, address, type) = map(lambda x: int(x,16), (length, address, type))
        if type is 2:
            self.hexParsingExtendedAddressOffset = int(data,16)
            return None
        elif type is not 0:
            return None
        if length is len(data)//2:
            #print("Addr Before: {}".format(address))
            address+= (self.hexParsingExtendedAddressOffset<<4)
            #print("Addr After: {}".format(address))
            return (address, data)
        else:
            raise Exception("Error parsing hex line: {}->({}, {}, {}, {}, {}) [{}]".format(line, length, address, type, data, checksum,len(data)))
                
DropletReprogrammer("COM3")
	