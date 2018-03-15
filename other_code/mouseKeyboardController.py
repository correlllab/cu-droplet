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
                            40:self.keyDown, 60:self.leftClick, 62:self.rightClick,
                            8:self.keyBksp, 13:self.keyEnter}
                            
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
            try:
                self.keyboard.press(key)
                self.keyboard.release(key)
            except ValueError:
                print("Failed to press {}: ValueError.".format(key))
            
    def handleMouseMove(self, key):
        key = key.strip("'\\")
        (xStr, yStr) = key.split()
        try:
            xVal = int(xStr)
            yVal = int(yStr)
        except ValueError:
            print("Got MouseMove but x, y string format unexpected: '{}', '{}'".format(xStr, yStr))
        print("Mouse Moving by: {}, {}".format(xVal,yVal))
        self.mouse.move(xVal,yVal)
                
    def keyUp(self):
        self.keyboard.press(Key.up)
        self.keyboard.release(Key.up)
        
    def keyLeft(self):
        self.keyboard.press(Key.left)
        self.keyboard.release(Key.left)        

    def keyRight(self):
        self.keyboard.press(Key.right)
        self.keyboard.release(Key.right)        

    def keyDown(self):
        self.keyboard.press(Key.down)
        self.keyboard.release(Key.down)        
        
    def keyBksp(self):
        self.keyboard.press(Key.backspace)
        self.keyboard.release(Key.backspace)        

    def keySpace(self):
        self.keyboard.press(Key.space)
        self.keyboard.release(Key.space)    
        
    def keyEnter(self):
        self.keyboard.press(Key.enter)
        self.keyboard.release(Key.enter)
        
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
        self.serialLock      = threading.Lock()
        self.readThread = threading.Thread(target=self.serialReader)
        self.wakerThread = threading.Thread(target=self.serialWaker)
        self.writerThread = threading.Thread(target=self.serialWriter)
        self.readThread.start()
        self.wakerThread.start()
        self.writerThread.start()
           
    def checkAnythingToRead(self):
        with self.serialLock:
            return (self.serialPort.inWaiting() > 0)
            
    def serialReadLine(self):
        try:
            with self.serialLock:
                dat = self.serialPort.readline()
            dat = dat.decode().strip()
            (keyword, key) = dat.split(maxsplit=1)
            if keyword == 'ButtonPress':
                self.output.handleButtonPress(key)
            elif keyword == 'MouseMove':
                self.output.handleMouseMove(key)
        except (UnicodeDecodeError, OSError, ValueError):
            return    
            
    def serialReader(self):
        while True:
            while self.checkAnythingToRead():
                self.serialReadLine()
            
    def serialWaker(self):
        while True:
            with self.serialLock:
                self.serialPort.write("WAKE\n".encode())
            time.sleep(5)
     
    def serialWriter(self):
        while True:
            userInput = input('>')
            if(len(userInput.strip()) > 0):
                userInput = userInput.strip() + '\n'
                with self.serialLock:
                    self.serialPort.write(userInput.encode())

output = MouseKeyboard()
mySerial = SerialThread("Com3", output)
