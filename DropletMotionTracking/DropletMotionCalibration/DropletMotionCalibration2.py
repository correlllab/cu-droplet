import sys
import numpy as np
import copy
import threading
import time
from collections import Counter
from rr import RR_API
import CustomMaths as maths
import MySerialWrapper as serial
import SocketConnection as sc


class MyException(Exception):
    pass

def set_led(r, g, b):
    serial.write("cmd set_led rgb %d %d %d"%(r, g, b))

def set_motor(motor_num, ccw_val, cw_val):
    serial.write("cmd set_motor %d %d %d"%(motor_num, ccw_val, cw_val))
    #Once more, just to be sure.
    serial.write("cmd set_motor %d %d %d"%(motor_num, ccw_val, cw_val))

def set_motors_flipped(mot0, mot1, mot2):
    
    serial.write("cmd set_motors_flipped %d %d %d"%(mot0, mot1, mot2))
    #Once more, just to be sure.
   # serial.write("cmd set_motors_flipped %d %d %d"%(mot0, mot1, mot2))

def move_steps(dir, num_steps):
    serial.write("cmd move_steps %d %d"%(dir, num_steps))
    #Once more, just to be sure.
 #   serial.write("cmd move_steps %d %d"%(dir, num_steps))

def stop_walk():
    serial.write("cmd stop_walk")
    #Once more, just to be sure.
  #  serial.write("cmd stop_walk")

class Calibrator:
    
    how_change_motors_LEFT = [[2, 1], [0, 1], [0, 1], [1, 1], [1, 1], [2, 1]]
    how_change_motors_RIGHT = [[1, 0], [1, 0], [2, 0], [2, 0], [0, 0], [0, 0]]

    bases = [np.array([0,1]),np.array([np.sqrt(3)/2,0.5]),np.array([np.sqrt(3)/2,-0.5])]
    expected_travel_dirs = [0, -np.pi/3, -2*np.pi/3, -np.pi, 2*np.pi/3, np.pi/3]
    x_boundaries = [6, 181, 352, 525, 696]
    y_boundaries = [6, 178, 352, 525, 696]
    current_motor_settings = [[0, 0], [0, 0], [0, 0]]

    data_points_for_straighten = 1000 #Number of data points we ask for while straightening.
    trusted_data_points = 200 #Number of data points we collect before worrying about running in to walls.
    spin_settings_history = [{}, {}]
    straight_settings_history = [{}, {}, {}]

    def __init__(self):
        self.rr = RR_API()
        self.rr.Connect("localhost")    
        port_h = serial.open_serial_port("COM9")
        serial.set_global_port(port_h)

    def get_proportional_adjustment(self, radius):
        return (40.0/np.power(radius+20.0, 1./3.))

    def change_motor_settings(self, dir, left_side, amount): 
        if left_side:
            (prim_mot, prim_sign) = self.how_change_motors_LEFT[dir]
            (second_mot, second_sign) = self.how_change_motors_RIGHT[dir]
        else:
            (prim_mot, prim_sign) = self.how_change_motors_RIGHT[dir]
            (second_mot, second_sign) = self.how_change_motors_LEFT[dir]
        if (self.current_motor_settings[second_mot][second_sign]>self.current_motor_settings[prim_mot][prim_sign]) and ((self.current_motor_settings[second_mot][second_sign]-amount)>=0):
            self.current_motor_settings[second_mot][second_sign] -= amount
            set_motor(second_mot, *self.current_motor_settings[second_mot])
        else:
            self.current_motor_settings[prim_mot][prim_sign] += amount
            set_motor(prim_mot, *self.current_motor_settings[prim_mot])

    def get_position_data_batch(self, dir, length):
        stop_walk()
        pos_list = np.zeros((length,2))
        orient_list = np.zeros(length)
        h = 0
        early_abort = False
        new_blobs = 0
        blobs= 0
        move_steps(dir, length)
        while (h<length) and not early_abort:
            pos_list[h] = np.array(self.get_rr_pos())
            orient_list[h] = self.get_rr_orient()
            self.rr.WaitImage(10)#Wait for roborealm to get a new image, so we aren't just polling the same var over and over again.
            if h>self.trusted_data_points:
                for x in self.x_boundaries:
                    if abs(pos_list[h][0]-x)<=15:
                        print("Blob position (%d, %d) too close to x=%d. h=%d"%(pos_list[h][0], pos_list[h][1], x, h))
                        early_abort = True
                for y in self.y_boundaries:
                    if abs(pos_list[h][1]-y)<=15:
                        print("Blob position (%d, %d) too close to y=%d. h=%d"%(pos_list[h][0], pos_list[h][1], y, h))
                        early_abort = True
            h += 1 #update counter            
        stop_walk()
        return (pos_list, orient_list, h)

    def initialize_motor_settings(self):
        set_motor(0,0,0)
        set_motor(1,0,0)
        set_motor(2,0,0)

    def get_rr_pos(self):
        try:
            vals=(float(self.rr.GetVariable("FIDUCIAL_X_COORD")), float(self.rr.GetVariable("FIDUCIAL_Y_COORD")))
        except ValueError:
            print("RR didn't have the fiducial.")
            return
        return vals
    
    def get_rr_orient(self):
        try:
            val = float(self.rr.GetVariable("FIDUCIAL_ORIENTATION"))
        except ValueError:
            print("RR didn't have the fiducial.")
            return
        val = np.mod(val+180.,360.)-180.
        return val

    def full_calibrate(self):
        #first, calibrate droplet spin for one direction. This will give us motor sign and relative values for the three motors.
        #then calibrate directions 0, 2, and 4 (or 1, 3, and 5)
        timestamp = time.strftime('%m-%d_%H%M')
        fileName = "data\\dmc_calib_" + timestamp
        cw_q = self.calibrate_droplet_spin(True, fName=fileName) #We're going to try and spin cw, but this will return false if we ended up spinning ccw. Should be okay, just let the next function know.
        self.calibrate_droplet_straight(cw_q, fName=fileName)
        for i in range(2):
            serial.write("cmd write_motor_settings")
        print("Done calibrating!")


    def test_spin_settings(self, motor_values, cw_q, output_stream):
        try:
            val = self.spin_settings_history[cw_q][str(motor_values)]
        except KeyError:
            '''We haven't done that one before.'''
        else:
            return val
        for i in range(3):
            self.current_motor_settings[i][cw_q] = abs(motor_values[i])
            set_motor(i, self.current_motor_settings[i][0], self.current_motor_settings[i][1])
        set_motors_flipped(*[motor_values[i]<0 for i in range(3)])
        try:
            (pos_list, orient_list, num_good_points) = self.get_position_data_batch(7-cw_q, self.data_points_for_straighten)
        except MyException:
            return
        print("mean delta orient: %f"%(np.mean(np.gradient(orient_list))))
        if num_good_points<0.8*self.data_points_for_straighten:
            print("Not enough good data.")
            return self.test_spin_settings(motor_values, cw_q, output_stream)
        (center, radius, residual, sign) = maths.lsq(pos_list, orient_list)
        direction_of_spin = np.mean(np.gradient(orient_list))
        print("Radius: %f"%(radius))
        self.spin_settings_history[cw_q][str(motor_values)] = (radius, direction_of_spin)
        #print("settings_history%s = %f"%(str(motor_values), radius))
        #print("settings_history: %s"%(str(self.settings_history)))
        orient_changed = orient_list[-1]-orient_list[0]
        output_stream.write("%f, %f, %f, %f, %f, %f\n"%(motor_values[0], motor_values[1], motor_values[2], radius, direction_of_spin, orient_changed))
        output_stream.flush()
        return self.test_spin_settings(motor_values, cw_q, output_stream)

    def calibrate_droplet_spin(self, cw_q, fName=None):
        alpha = 1.  #reflection coefficient
        gamma = 2.  #expansion coefficient
        rho = -0.5  #contraction coefficient
        sigma = 0.5 #shrink coefficient

        #self.initialize_motor_settings()

        x_0 = np.array([0, 0, 0])
        x_1 = np.array([128, 0, 0])
        x_2 = np.array([0, 128, 0])
        x_3 = np.array([0, 0, 128])
        spx = np.array([x_0, x_1, x_2, x_3])
        old_spx = np.zeros((4,3))

        if fName is None:
            settings_history_fName = 'dmc_nm_data_spin_all_%s.csv'%("cw" if cw_q else "ccw")
            simplex_history_fName = 'dmc_nm_data_spin_spx_%s.csv'%("cw" if cw_q else "ccw")
        else:
            settings_history_fName = fName + "_hist_spin.csv"
            simplex_history_fName = fName + "_spx_spin.csv"

        settings_history_file = open(settings_history_fName,'w')
        settings_history_file.write('mot0set, mot1set, mot2set, radius, mean delta orient, total delta orient\n')

        simplex_history_file = open(simplex_history_fName,'w')
        simplex_history_file.write('spx00, spx01, spx02, rad0, spx10, spx11, spx12, rad1, spx20, spx21, spx22, rad2, spx30, spx31, spx32, rad3\n')

        finished = False
        def fun(x):
            (radius, mean_delta_orient) =self.test_spin_settings(x, cw_q, settings_history_file)
            return radius + -4*abs(mean_delta_orient)
        def full_fun(x):
            return self.test_spin_settings(x, cw_q, settings_history_file)
        try:
            while(True):
                spx = np.array(sorted(spx, key=fun))
                if str(spx) == str(old_spx):
                    finished = True
                old_spx = copy.deepcopy(spx)

                radii = [full_fun(point)[0] for point in spx]
                output_array = np.array(map(lambda spx_point, radius: np.append(spx_point, radius), spx, radii)).flatten().tolist()
                simplex_history_file.write(",".join(map(str, output_array))+"\n")
                simplex_history_file.flush()



                print("spx: %s"%(str(spx)))
                x_o = np.mean(spx[:-1],0).astype(int)
                x_r = (x_o + alpha*(x_o - spx[-1])).astype(int)
                if (fun(x_r)<fun(spx[-2])) and (fun(x_r)>=fun(spx[0])):
                    print("Reflecting.")
                    spx[-1] = x_r
                elif fun(x_r)<fun(spx[0]):

                    x_e = (x_o + gamma*(x_o-spx[-1])).astype(int)
                    if fun(x_e)<fun(x_r):
                        print("Expanding.")
                        spx[-1] = x_e
                    else:
                        print("Reflecting.")
                        spx[-1] = x_r
                else:

                    x_c = (x_o + rho*(x_o-spx[-1])).astype(int)
                    if fun(x_c)<fun(spx[-1]):
                        print("Contracting.")
                        spx[-1] = x_c
                    else:
                        for i in range(1,4):
                            spx[i] = (spx[0] + sigma*(spx[i]-spx[0])).astype(int)

                if fun(spx[0])<1 or finished:
                    (radius, delta_orient) = full_fun(spx[0]);
                    if delta_orient<0:
                        cw_q=True
                    else:
                        cw_q=False
                    for i in range(3):
                        self.current_motor_settings[i][cw_q] = abs(spx[0][i])
                        set_motor(i, self.current_motor_settings[i][0], self.current_motor_settings[i][1])
                    for i in range(4):
                        serial.write("cmd write_motor_settings")
                    print("We're done with spinning!")
                    settings_history_file.close()
                    simplex_history_file.close()
                    return cw_q
        except KeyboardInterrupt:
            print("Calibration interrupted by user.")
            settings_history_file.close()
            simplex_history_file.close()

    def spin_for_safe_straight(self, dir, cw_q):
        pos = self.get_rr_pos()

        i=1
        while pos[0] >= self.x_boundaries[i]:
            i+=1
        temp_x = pos[0] - self.x_boundaries[i-1]
        x_bound = self.x_boundaries[i]-self.x_boundaries[i-1]

        j=1 
        while pos[1] >= self.y_boundaries[j]:
            j+=1
        temp_y = pos[1] - self.y_boundaries[j-1]
        y_bound = self.y_boundaries[j]-self.y_boundaries[j-1]

        if (temp_x>(x_bound/3)) and (temp_x<((2*x_bound)/3)) and (temp_y>(y_bound/3)) and (temp_y<((2*y_bound)/3)):
            print("We're basically in the middle. Should be fine.")
            return
            
        if temp_x<(x_bound/2):
            if temp_y<(y_bound/2): #bottom left quadrant of our square
                desired_orientationInit = -45
            else: #top left quadrant of our square
                desired_orientationInit = -135
        else:
            if temp_y<(y_bound/2): #bottom right quadrant of our square
                desired_orientationInit = 45
            else: #top right quadrant of our square
                desired_orientationInit = 135


        desired_orientation = desired_orientationInit + dir*60
        desired_orientation = np.mod(desired_orientation+180,360) - 180
        
        print("dir: %d, temp_pos: (%f, %f), Desired Orientation: %f, Desired orientation, adjusted: %f"%(dir, temp_x, temp_y, desired_orientationInit, desired_orientation))

        while(True):
            move_steps(7-cw_q, 2000)
            last_orient = 0
            count=0
            while count<200: #timeout of ~2 seconds of no movement.
                orient = self.get_rr_orient()
                if abs(orient-last_orient)>0.5:
                    count=0
                self.rr.WaitImage(10)
                orient = np.mod(orient+180,360)-180
                if abs(desired_orientation-orient)<1:
                    print("Spin complete.")
                    return
                last_orient=orient
                count+=1
            print("Droplet not moving? Trying again.")


    def test_straight_settings(self, mot_val, mot_num, cw_q, output_stream):
        try:
            return self.straight_settings_history[mot_num][mot_val]
        except KeyError:
            '''We haven't done that one before.'''
        self.current_motor_settings[mot_num][1-cw_q] = mot_val
        set_motor(mot_num, self.current_motor_settings[mot_num][0], self.current_motor_settings[mot_num][1])
        try:
            dir = ((2*(mot_num+1+cw_q))%6)
            self.spin_for_safe_straight(dir, cw_q)
            (pos_list, orient_list, num_good_points) = self.get_position_data_batch(dir, self.data_points_for_straighten)
        except MyException:
            return
        if num_good_points<0.8*self.data_points_for_straighten:
            print("Not enough good data.")
            return
        (center, radius, residual, sign) = maths.lsq(pos_list, orient_list)
        distance_traveled = np.linalg.norm(pos_list[-1]-pos_list[0])
        self.straight_settings_history[mot_num][mot_val] = (radius, sign, distance_traveled)
        #print("settings_history%s = %f"%(str(motor_values), radius))
        #print("settings_history: %s"%(str(self.settings_history)))
        output_stream.write("%d, %d, %f, %f, %f\n"%(mot_num, mot_val, radius, sign, distance_traveled))
        output_stream.flush()
        return self.test_straight_settings(mot_val, mot_num, cw_q, output_stream)

    def get_proportional_control(self, radius, signSwapped):
        return (int(round(40./radius))+1)
        #if signSwapped:
        #    return 1
        #elif radius>80:
        #    return 10
        #elif radius>40:
        #    return 20
        #elif radius>35:
        #    return 40
        #elif radius>25:
        #    return 80
        #else:
        #    return 160

    def get_change_direction(self, sign, cw_q):
        if sign < 0: #center on right,
            if cw_q: #
                return 1
            else: #
                return -1
        elif sign > 0: #center on left, so it needs to be bigger (ONLY IF WE SPAN CW)
            if cw_q: #
                return -1
            else: #
                return 1
        else:
            print("Should never see this. Droplet moved perfectly straight?")

    def calibrate_droplet_straight(self, cw_q, fName=None):
        try:
            if fName is None:
                fName = 'dmc_nm_data_mot.csv'
            else:
                fName = fName + "_straight.csv"
            output_file = open(fName,'w')
            output_file.write('mot_num,mot_val,radius,sign,dist\n')
            try:
                for mot_num in range(3):
                    def fun(x):
                        while(True):
                            val = self.test_straight_settings(x, mot_num, cw_q, output_file)
                            if val is not None:
                                return val
                            print("Failed to get good data. Trying again?")



                    baseVal = 0
                    change = self.get_change_direction(fun(baseVal)[1], cw_q)
                    if change<0:
                        extremaVal = -128
                    else:
                        extremaVal = 128

                    while self.get_change_direction(fun(baseVal)[1],cw_q) == self.get_change_direction(fun(extremaVal)[1],cw_q):
                        extremaVal = extremaVal*2

                    if extremaVal<baseVal:
                        minVal = extremaVal
                        maxVal = baseVal
                    else:
                        minVal = baseVal
                        maxVal = extremaVal

                    #Code above is setting bounds for the binary search

                    curVal = int((minVal+maxVal)/2)

                    while (abs(minVal-maxVal)>1): #while newVal is actually changing...
                        print("mot_num: %d, newVal: %d, radius: %f, sign: %f, dist: %f"%(mot_num, curVal, fun(curVal)[0], fun(curVal)[1], fun(curVal)[2]))
                        if self.get_change_direction(fun(curVal)[1],cw_q)<0:
                            maxVal = curVal
                            curVal = int((minVal+maxVal)/2)
                        else:
                            minVal = curVal
                            curVal = int((minVal+maxVal)/2)

                    sorted_settings = sorted(self.straight_settings_history[mot_num].iteritems(), key=lambda x: x[1], reverse=True)
                    print("Best radius was %f, for setting %d."%(sorted_settings[0][1][0], sorted_settings[0][0]))
                    self.current_motor_settings[mot_num][1-cw_q] = sorted_settings[0][0]
                    set_motor(mot_num, self.current_motor_settings[mot_num][0], self.current_motor_settings[mot_num][1])
                    print("Finishe mot %d."%(mot_num))
                print("Finished straight.")

            except KeyboardInterrupt:
                print("Calibration interrupted by user.")
                stop_walk()
                output_file.close()
        except:
            output_file.close()
            serial.cleanup()
            self.rr.close()
            raise
    
    def calibrate_degrees_per_step(self, cw_q, num_steps = 50, num_runs = 5, fName = None):
        """
        Calibrates the number of degrees per kilostep of droplet motion rotation
        """
        orient_buf = np.array([0] * 100)
        rot_angle = np.array([0] * num_runs)

        if fName is None:
            fName = 'data\\dmc_data_deg_per_kilostep.csv'
        else:
            fName = fName + "_deg_per_kilostep.csv"
        try:
            fh = open(fName, 'w')
            for r in range(num_runs):            
                start_orient = self.get_rr_orient()
                move_steps(7 - cw_q, num_steps)

                while(True):
                    for i in range(100):
                        orient_buf[i] = self.get_rr_orient()
                        self.rr.WaitImage(10)

                    if(abs(orient_buf[-1]-orient_buf[0])<0.01):
                        #print("Breaking, orient_buf: %s"%orient_buf)
                        print("Breaking.")
                        time.sleep(2)
                        #This condition is firing prematurely.
                        break
            
                stop_orient = self.get_rr_orient()
                rot_angle[r] = stop_orient - start_orient
                if(cw_q):
                    rot_angle[r]=rot_angle[r]*-1
                if(rot_angle[r] < 0):
                    rot_angle[r] += 360
            
                print("[Run %d] Rotation Angle: %d (deg)\n"%(r, rot_angle[r]))
                fh.write("%d, %d\n"%(r, rot_angle[r]))

            deg_per_kilostep = int(np.mean(rot_angle) * 1000. / num_steps)
            print("Degrees per kilostep: %d\n"%deg_per_kilostep)
            fh.write("%d\n"%deg_per_kilostep)
            fh.close()
        except KeyboardInterrupt:
            print("Calibration interrupted by user.")
            stop_walk()
            fh.close()

    def calibrate_dist_per_step(self, dir, num_steps = 100, fName = None):
        """
        Calibrates the number of degrees per kilostep of droplet motion rotation
        """
        pos_buf = np.zeros((100,2))

        if fName is None:
            fName = 'data\\dmc_data_deg_per_kilostep.csv'
        else:
            fName = fName + "_deg_per_kilostep.csv"
        try:   
            start_pos = np.array(self.get_rr_pos())
            move_steps(dir, num_steps)

            while(True):
                for i in range(100):
                    pos_buf[i] = np.array(self.get_rr_pos())
                    self.rr.WaitImage(10)
                if np.linalg.norm(pos_buf[0]-pos_buf[-1])<1:
                    break
                
            stop_pos = np.array(self.get_rr_pos())

            dist = np.linalg.norm(start_pos-stop_pos)
            mm_per_kilostep = int(dist/.69 * 1000. / num_steps)
            print("dist: %f, mm per kilostep: %d\n"%(dist,mm_per_kilostep))
        except KeyboardInterrupt:
            print("Calibration interrupted by user.")
            stop_walk()
                            
    def __del__(self):
        serial.cleanup()
        self.rr.close()
        print("Finished cleaning up myself.")


cal = Calibrator()



