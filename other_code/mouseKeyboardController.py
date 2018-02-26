import time
import threading
import serial
from pynput.keyboard import Key, Controller as KeyboardController
from pynput.mouse import Button, Controller as MouseController

class MouseKeyboard:
    def __init__(self):
        self.keyboard = KeyboardController()
        self.mouse = MouseController()
        self.controlKeys = {37:self.keyLeft, 38:self.keyUp, 39:self.keyRight, 
                            40:self.keyDown, 60:self.leftClick, 62:self.rightClick}
                            
    def handleButtonPress(self, key):
        key = key.strip("'\\")
        try:
            keyNum = int(key)
        except ValueError:
            keyNum = ord(key)
        try:
            print('Got {}.'.format(keyNum))
        except OSError:
            pass
        if keyNum in self.controlKeys.keys():
            self.controlKeys[keyNum]()
        else:
            self.keyboard.press(key)
            self.keyboard.release(key)
            
    def handleMouseMove(self, key):
        key = key.strip("'\\")
        (xStr, yStr) = key.split()
        try:
            xVal = int(xStr)
            yVal = int(yStr)
        except ValueError:
            print("Got MouseMove but x, y string format unexpected: '{}', '{}'".format(xStr, yStr))
        self.mouse.move(xVal,yVal)
                
    def keyUp(self):
        self.keyboard.press(Key.up)
        
    def keyLeft(self):
        self.keyboard.press(Key.left)

    def keyRight(self):
        self.keyboard.press(Key.right)

    def keyDown(self):
        self.keyboard.press(Key.down)
        
    def leftClick(self):
        self.mouse.click(Button.left)
    
    def rightClick(self):
        self.mouse.click(Button.right)
        
class SerialThread:

    def __init__(self, portName, mk):
        self.output = mk
        try:
           self.serialPort = serial.Serial(portName, baudrate=115200, timeout=5)
        except serial.SerialException:
           print("ERROR: Port " + portName + " is already open!\n")
           return
        self.eventBuffer = []
        self.eventBufferLock = threading.Lock()
        self.serialLock      = threading.Lock()
        self.readThread = threading.Thread(target=self.serialReader)
        self.wakerThread = threading.Thread(target=self.serialWaker)
        self.writerThread = threading.Thread(target=self.serialWriter)
        self.readThread.start()
        self.wakerThread.start()
        self.writerThread.start()
           
    def serialReader(self):
        while True:
            while self.serialPort.inWaiting() > 0:
                dat = self.serialPort.readline()
                dat = dat.decode().strip()
                try:
                    (keyword, key) = dat.split(maxsplit=1)
                except ValueError:
                    continue
                if keyword == 'ButtonPress':
                    self.output.handleButtonPress(key)
                elif keyword == 'MouseMove':
                    self.output.handleMouseMove(key)
            
    def serialWaker(self):
        while True:
            self.serialPort.write("WAKE\n".encode())
            time.sleep(5)
     
    def serialWriter(self):
        while True:
            userInput = input('>')
            if(len(userInput.strip()) > 0):
                userInput = userInput.strip() + '\n'
                self.serialPort.write(userInput.encode())

output = MouseKeyboard()
mySerial = SerialThread("Com3", output)
