import serial
import time
import os
import re
from collections import OrderedDict
from threading import Thread, Lock

class SerialHandler:

    encoding='utf-8'

    def __init__(self, ioLock, portName="COM3", baudRate=115200,byteSize=serial.EIGHTBITS, parity=serial.PARITY_NONE):
        self.ioLock = ioLock
        self.port = serial.Serial(portName, baudRate, bytesize=byteSize, parity=parity)
        self.serialLock = Lock()
        self.readerThread = Thread(target=self.readerLoop)
        self.readerThread.start()
        self.write('\n')

    def writeLine(self, *data):
        data = list(data)
        data.append('\n')
        self.write(*data)

    def write(self, *data):
        fullString = "".encode(self.encoding)
        for string in data:
            if type(string) is bytearray:
                fullString += bytes(string)
            elif type(string) is str:
                fullString += bytes(string.encode('utf-8'))
            else:
                fullString += "{}".format(string).encode(self.encoding)
        self.__write(fullString)
        with self.ioLock:
            print('{}> {}'.format(len(fullString), fullString))

    def __write(self, str):
        with self.serialLock:
            self.port.write(str)

    def readerLoop(self):
        while self.port.is_open:
            while self.port.in_waiting>0:
                with self.serialLock:
                    line = self.port.readline()
                with self.ioLock:
                    print('  < {}'.format(line))

class DropletReprogrammer:
    
    dataMsgLength = 16
    trackedSections = ['.wrapper', '.usrtxt', '.usrdat']
    sectionsDict = {}
    hexParsingExtendedAddressOffset = 0
    
    def __init__(self, portName, buildName=None):
        self.ioLock = Lock()
        self.getFiles(buildName)
        self.readLSSFile()
        self.readHexFile()
        self.getDataFromLinesDict()        
        for section in self.trackedSections:
            with self.ioLock:
                print("{}: {}, {} [{}]\r\n\t{}\r\n".format(section, self.sectionsDict[section][0], self.sectionsDict[section][1], len(self.trackedData[section]), self.trackedData[section]))
        self.serial = SerialHandler(self.ioLock, portName="COM3")
        self.sendData()
        #for (addr, data) in self.linesDict.items():
            #print('{} -> {}'.format(addr, data))
        #print(self.sectionsDict)
    
    def sendData(self):
        ((wrapperAddr, wrapperLen), (txtAddr, txtLen), (dataAddr, dataLen)) = self.sectionsDict.values()
        #def convert(val):
            #return bytearray(val.to_bytes(2, byteorder='little'))
        #(wrapperAddr, wrapperLen, txtAddr, txtLen, dataAddr, dataLen) = map(convert, (wrapperAddr, wrapperLen, txtAddr, txtLen, dataAddr, dataLen))
        self.sendStartMessage([wrapperAddr,wrapperLen],[txtAddr,txtLen],[dataAddr,dataLen])
        self.sendProgDatSection(self.trackedData['.wrapper'])
        self.sendProgDatSection(self.trackedData['.usrtxt'])
        self.sendProgDatSection(self.trackedData['.usrdat'])
    
    def sendProgDatSection(self, sectionDat):
        curPos=0
        sectionLength = len(sectionDat)
        while curPos<(sectionLength-self.dataMsgLength):
            self.sendProgDatMsg(format(self.dataMsgLength,'02x'), sectionDat[curPos:curPos+self.dataMsgLength].hex())
            curPos += self.dataMsgLength
        self.sendProgDatMsg(format(sectionLength-curPos,'02x'),sectionDat[curPos:].hex())
    
    def sendProgDatMsg(self, length, dat):
        self.serial.writeLine("prgD ", length, " ",dat)
        time.sleep(1)
    
    def sendStartMessage(self, *args):
        for pair in args:
            if pair[0]%2 is not 0:
                with self.ioLock:
                    print("Warning! Section Address is Odd!")
        strings = [" {:04x} {:04x}".format(pair[0]>>1, pair[1]) for pair in args]
        self.serial.writeLine("prg", *strings)
        time.sleep(5)
    
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
                
reprogr=DropletReprogrammer("COM3")
	