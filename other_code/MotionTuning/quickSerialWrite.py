# -*- coding: utf-8 -*-
"""
Created on Mon Aug 24 12:13:36 2015

@author: Colab
"""

from SerialInterface import SerialInterface
import time
import threading

class QuickWriter:
    
    def __init__(self):
        self.serial_port = SerialInterface()
        self.serial_port.open("COM16")
        
        self.serial_buffer = ""
        self.serial_buffer_lock = threading.Lock()    
        self.serial_lock = threading.Lock()
        self.serial_thread = threading.Thread(target=self.serial_monitor)
        self.serial_lock.acquire()
        self.serial_thread.start()    

    def serial_monitor(self):
        while self.serial_lock.locked():
            with self.serial_buffer_lock:
                if len(self.serial_buffer)>0:
                    self.serial_port.write(self.serial_buffer)
                    self.serial_buffer = ""
            self.serial_port.read()
            time.sleep(0.1)
        self.serial_port.close()
        
    def go(self):
        with open('dump.txt','r') as f:
            for line in f:
                with self.serial_buffer_lock:
                    self.serial_buffer = (line+'\n')
                time.sleep(1)
        self.serial_lock.release()


qw = QuickWriter()
time.sleep(2)
qw.go()
