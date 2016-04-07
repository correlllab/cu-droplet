# -*- coding: utf-8 -*-
"""
Created on Wed Jul 29 15:28:58 2015

@author: Colab
"""
from __future__ import print_function
from SerialInterface import SerialInterface
from RoboRealmInterface import RoboRealmInterface
from collections import defaultdict, deque
import CustomMaths
import threading
import time
import random
from math import hypot
import numpy as np
import sys

ordered_ids = ["2B4E", "7D78", "8B46", "C806", "4177", "0A0B", "3B49", "028C", "1F08", "EEB0", "A649", "A5B5", "F60A", "B944", "3405", "43BA", "6648", "1B4B", "C24B", "4DB0"]



pose_dat = defaultdict(deque)
pose_dat_lock = threading.Lock()

serial_buffer = ""
serial_buffer_lock = threading.Lock()

rr_lock = threading.Lock()
serial_lock = threading.Lock()

def factory():
    def settings_factory():
        return [0,0]
    settings_dict = defaultdict(settings_factory)
    distances_dict = defaultdict(list)
    return (settings_dict, distances_dict)

history_dict = defaultdict(factory)

rr = RoboRealmInterface()
rr.connect()
serial_port = SerialInterface()
if not serial_port.open("COM16"):
    print('Error opening serial port %s'%"COM16")
    
def rr_monitor():
    HISTORY_LENGTH = 300        
    
    while rr_lock.locked():
        with pose_dat_lock:
            for k, v in rr.get_robot_positions().items():
                pose_dat[k].append(v)
                if(len(pose_dat[k])>HISTORY_LENGTH):
                    pose_dat[k].popleft()
            for robot_id in pose_dat.keys():
                print('\t{0}: {1}'.format(robot_id, pose_dat[robot_id][-1]))
            print()
        rr.wait_image()
    rr.disconnect()
        
def serial_monitor():
    global serial_buffer
    while serial_lock.locked():
        with serial_buffer_lock:
            if len(serial_buffer)>0:
                serial_port.write(serial_buffer)
                serial_buffer = ""
        serial_port.read()
    serial_port.close()
            
            
def calculate_dist_per_step(dir):
    for robot_id in pose_dat.keys():
        if dir==0:
            continue        
        elif dir>5:
            distance = (pose_dat[robot_id][-1][2]-pose_dat[robot_id][0][2])
        else:
            distance = 0
            pose_hist = pose_dat[robot_id]
            for i in range(len(pose_dat[robot_id])-1):
                distance += hypot(pose_hist[i][0]-pose_hist[i-1][0],pose_hist[i][1]-pose_hist[i-1][1])
        history_dict[robot_id][1][dir].append(distance)
            
            
def get_rand_dir():
    rand_int = random.randint(0,7)
    if rand_int<3:
        return 1
    elif rand_int<6:
        return 2
    else:
        return (rand_int-3)
        
        
def calculate_adjust_response((center, radius, residual, sign), direction):
    if direction>2:
        return 0
    if radius<10:
        #We haven't really moved. Do nothing.
        return 0
    elif radius>80000:
        #We're basically going straight. Call it good.
        return 0
    else:
        if sign<0:
            return 1
        elif sign>0:
            return 2
        else:
            return 0
    

def get_per_robot_command(new_direction, lsq_dat, last_direction):
    value = (96+(new_direction<<2))
    if lsq_dat is not None:
        value += calculate_adjust_response(lsq_dat, last_direction)
    return chr(value)

def main():
    global serial_buffer
    global pose_dat
    rr_thread = threading.Thread(target=rr_monitor)
    rr_lock.acquire()
    rr_thread.start()
    serial_thread = threading.Thread(target=serial_monitor)
    serial_lock.acquire()
    serial_thread.start() 
    
    with serial_buffer_lock:
        serial_buffer = 'cmd reset'
    time.sleep(1)
    try:
        new_direction = 1
        
        with serial_buffer_lock:
            serial_buffer = 'msg {0}{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}{11}{12}{13}{14}{15}{16}{17}{18}{19}'.format(*[chr(96+(new_direction<<2)) for _ in ordered_ids])
        last_direction = new_direction        
        while True:
            for _ in range(30):
                time.sleep(1)
            lsq_dat = {}
            for robot_id in ordered_ids:
                lsq_dat[robot_id] = None       
                
            def get_per_robot_command(bot_id):
                value = (96+(new_direction<<2))
                if (lsq_dat[bot_id] is not None) and last_direction is not 0:
                    adjust = calculate_adjust_response(lsq_dat[bot_id], last_direction)
                    value += adjust
                    if adjust is 1:
                        history_dict[bot_id][0][last_direction][0] -= 10
                        history_dict[bot_id][0][last_direction][1] += 10                        
                    elif adjust is 2:
                        history_dict[bot_id][0][last_direction][0] += 10
                        history_dict[bot_id][0][last_direction][1] -= 10                              
                return chr(value)                  

            with pose_dat_lock:
                for robot_id in pose_dat.keys():
                    try:
                        lsq_dat[robot_id] = CustomMaths.lsq_circle_fit(np.array(pose_dat[robot_id]))
                    except TypeError:
                        lsq_dat[robot_id] = (0,0,0,0)
                calculate_dist_per_step(last_direction)
                new_direction = get_rand_dir()
                
                with serial_buffer_lock:
                    serial_buffer = 'msg {0}{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}{11}{12}{13}{14}{15}{16}{17}{18}{19}'.format(*[get_per_robot_command(bot_id) for bot_id in ordered_ids])
                last_direction = new_direction
                pose_dat = defaultdict(deque)
            for bot in ordered_ids:
                print(bot,end='')
                for dir in [1,2,3,4]:
                    print('\t',end='')
                    print(history_dict[bot][1][dir])                
    except KeyboardInterrupt:
        for bot in ordered_ids:
            print(bot)   
            print("\tAdjusts")
            for dir in [1,2,3,4]:
                print('\t',end='')
                print(history_dict[bot][0][dir])
            print("\tDists:")
            for dir in [1,2,3,4]:
                print('\t',end='')
                print(history_dict[bot][1][dir])
        rr_lock.release()
        rr_thread.join()
        serial_lock.release()        
        serial_thread.join()
        sys.exit()
        
        
        #todo: throw out data where Droplets are too close to one another (or the wall?)
        #todo: take a longer term look at what I'm telling the Droplets to do: 
        # and, for example, stop if one dir has been wiggling back and for awhile

main()            