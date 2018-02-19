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
from math import hypot, degrees
import numpy as np

class MotionTuner:
    
    ordered_ids = ["2B4E", "7D78", "8B46", "C806", "4177", "0A0B", "3B49", 
                   "028C", "1F08", "EEB0", "A649", "A5B5", "F60A", "B944", 
                   "3405", "43BA", "6648", "1B4B", "C24B", "4DB0"]
    last_direction = 0
    droplet_pixel_radius=36.0
    last_direction_dict = defaultdict(lambda: 0)             
    
    def __init__(self):
        self.pose_dat = defaultdict(deque)
        self.pose_dat_lock = threading.Lock()
        
        self.serial_buffer = ""
        self.serial_buffer_lock = threading.Lock()
        
        self.rr_lock = threading.Lock()
        self.serial_lock = threading.Lock()
        
        def factory():
            def settings_factory():
                return [0,0]
            settings_dict = defaultdict(settings_factory)
            distances_dict = defaultdict(list)
            return (settings_dict, distances_dict)
        
        self.history_dict = defaultdict(factory)   
        
        self.rr = RoboRealmInterface()
        self.rr.connect()
        
        self.serial_port = SerialInterface()
        self.serial_port.open("COM16")

        self.rr_thread = threading.Thread(target=self.rr_monitor)
        self.rr_lock.acquire()
        self.rr_thread.start()
        self.serial_thread = threading.Thread(target=self.serial_monitor)
        self.serial_lock.acquire()
        self.serial_thread.start()  
                             
    
    def rr_monitor(self):         
        while self.rr_lock.locked():
            with self.pose_dat_lock:
                for k, v in self.rr.get_robot_positions().items():
                    self.pose_dat[k].append(v)
                    if(len(self.pose_dat[k])>300):
                        self.pose_dat[k].popleft()
#                for robot_id in self.pose_dat.keys():
#                   print('\t{0}: {1}'.format(robot_id, self.pose_dat[robot_id][-1]))
#                print()
            self.rr.wait_image()
        self.rr.disconnect()
        
    def serial_monitor(self):
        while self.serial_lock.locked():
            with self.serial_buffer_lock:
                if len(self.serial_buffer)>0:
                    self.serial_port.write(self.serial_buffer)
                    self.serial_buffer = ""
            self.serial_port.read()
            time.sleep(0.1)
        self.serial_port.close()
            
            
    def calculate_dist_per_step(self):
        for robot_id in self.pose_dat.keys():
            last_dir = self.last_direction_dict[robot_id]
            if last_dir==0:
                continue        
            elif last_dir>2:
                distance = 0
                pose_hist = self.pose_dat[robot_id]
#                print(pose_hist)
#                time.sleep(30)
                for i in range(1,len(self.pose_dat[robot_id])):
                    deltaTheta = pose_hist[i][2]-pose_hist[i-1][2]
                    if deltaTheta>180:
                        deltaTheta = deltaTheta-360
                    elif deltaTheta<-180:
                        deltaTheta = deltaTheta+360
                    distance += deltaTheta
#                print('{0}: {1}'.format(robot_id, distance))
            else:
                distance = 0
                pose_hist = self.pose_dat[robot_id]
                for i in range(1,len(self.pose_dat[robot_id])):
                    distance += hypot(pose_hist[i][0]-pose_hist[i-1][0],pose_hist[i][1]-pose_hist[i-1][1])
            self.history_dict[robot_id][1][last_dir].append(distance)
            
            
    def get_rand_dir(self):
        rand_int = random.randint(0,7)
        if rand_int<3:
            return 1
        elif rand_int<6:
            return 2
        else:
            return (rand_int-3)
            
        
    def calculate_adjust_response(self, bot_id):
        (center, radius, residual, sign) = self.lsq_dat[bot_id]
        direction = self.last_direction_dict[bot_id]
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

    def calculate_desired_directions(self):
        forces_dict = defaultdict(lambda: np.array([0.0,0.0]))
        for botId in self.ordered_ids:
            if(len(self.pose_dat[botId])<=0):
                continue
            botPos = np.array([self.pose_dat[botId][-1][0], 
                               self.pose_dat[botId][-1][1]])
#            print('{0} @ {1}'.format(botId, botPos))       
        
            bl_diff = (botPos/self.droplet_pixel_radius)+np.array([1.0,1.0])
            tr_diff = ((np.array([1000.0, 1000.0])-botPos)/self.droplet_pixel_radius)+np.array([1.0,1.0])
            bl_wall_force = 1.8/(bl_diff**1.0)
            tr_wall_force = 1.8/(tr_diff**1.0)
            wall_forces = bl_wall_force-tr_wall_force

            forces_dict[botId]+=wall_forces                                    
#            print('\tWALLS: {0}'.format(wall_forces))
                        
            for otherBotId in reversed(self.ordered_ids):
                if botId is otherBotId:
                    break
                if len(self.pose_dat[otherBotId])<=0:
                    continue
                
                otherBotPos = np.array([self.pose_dat[otherBotId][-1][0], 
                                        self.pose_dat[otherBotId][-1][1]])
                    
                diffVec = (botPos - otherBotPos)/self.droplet_pixel_radius
                r = np.linalg.norm(diffVec)
                forceVec = ((1.5*diffVec)/(r**3))
#                print('\t({0}, {1}): {2}  \t{3}  \t{4}'.format(botId, otherBotId, diffVec, r, forceVec))
                
                forces_dict[botId] += forceVec
                forces_dict[otherBotId] -= forceVec
        direction_dict = defaultdict()
        for botId in self.ordered_ids:
            delta_theta = 0.0
            (r, theta) = (hypot(*forces_dict[botId]), degrees(np.arctan2(*(forces_dict[botId][::-1]))))
            if r<0.1:
                direction_dict[botId] = self.get_rand_dir()
            elif len(self.pose_dat[botId])<=0:
                direction_dict[botId] = self.get_rand_dir()
            else:
                delta_theta = CustomMaths.pretty_angle((theta-90)-self.pose_dat[botId][-1][2])
                if abs(delta_theta)<=50:
                    direction_dict[botId] = 1 #straight forward
                elif abs(delta_theta)>=130:
                    direction_dict[botId] = 2 #straight backward
                elif delta_theta>0:
                    direction_dict[botId] = 4 #turn left
                elif delta_theta<0:
                    direction_dict[botId] = 3 #turn right
#            print('{0}: ({1}, {2}, {3}): {4}'.format(botId, r, theta, delta_theta, direction_dict[botId]))
        return direction_dict            
    
    def get_per_robot_command(self, bot_id, direction):
        value = (96+(direction<<2))
        if (self.lsq_dat[bot_id] is not None) and self.last_direction_dict[bot_id] is not 0:
            adjust = self.calculate_adjust_response(bot_id)
            value += adjust
            if adjust is 1:
                self.history_dict[bot_id][0][self.last_direction_dict[bot_id]][0] -= 10
                self.history_dict[bot_id][0][self.last_direction_dict[bot_id]][1] += 10                        
            elif adjust is 2:
                self.history_dict[bot_id][0][self.last_direction_dict[bot_id]][0] += 10
                self.history_dict[bot_id][0][self.last_direction_dict[bot_id]][1] -= 10                              
        return chr(value)    

    def send_message(self):
        direction_dict = self.calculate_desired_directions()
        commands = [self.get_per_robot_command(bot_id, direction_dict[bot_id]) for bot_id in self.ordered_ids]
        self.last_direction_dict = direction_dict
        with self.serial_buffer_lock:
            self.serial_buffer = 'msg {0}{1}{2}{3}{4}{5}{6}{7}{8}{9}{10}{11}{12}{13}{14}{15}{16}{17}{18}{19}'.format(*commands)        

    def main_loop(self):
        self.lsq_dat = defaultdict(lambda: (0,0,0,0))
    
        with self.pose_dat_lock:
            for robot_id in self.pose_dat.keys():
                try:
                    self.lsq_dat[robot_id] = CustomMaths.lsq_circle_fit(np.array(self.pose_dat[robot_id]))
                except TypeError:
                    continue
            self.calculate_dist_per_step()
            
            self.send_message()           
            self.pose_dat = defaultdict(deque)
            self.lsq_dat = defaultdict(lambda: (0,0,0,0))
#        for bot in self.ordered_ids:
#            print(bot,end='')
#            for dir in [1,2,3,4]:
#                print('\t',end='')
#                print(self.history_dict[bot][1][dir])                

    def clean_up(self):
        print('{')
        for bot in self.ordered_ids:
            print('{{{0}, '.format(bot), end='')
            for dir in [1,2]:
                print('{{{0}, {1}}}, '.format(*self.history_dict[bot][0][dir]), end='')
            for dir in [1,2,3,4]:
                print('{', end='')
                for dist in self.history_dict[bot][1][dir]:
                    print('{0}, '.format(dist), end='')
                print('}, ', end='')
            print('}, ')
        print('}')
        self.rr_lock.release()
        self.rr_thread.join()
        self.serial_lock.release()        
        self.serial_thread.join()        

    def main(self):
        with self.serial_buffer_lock:
            self.serial_buffer = 'cmd reset'
        time.sleep(1)
        try:     
            while True:
                for _ in range(30):
                    time.sleep(1)                
                self.main_loop()
        except:
            self.clean_up()
            raise
        
        
        #todo: throw out data where Droplets are too close to one another (or the wall?)
        #todo: take a longer term look at what I'm telling the Droplets to do: 
        # and, for example, stop if one dir has been wiggling back and for awhile

mt = MotionTuner()
mt.main()