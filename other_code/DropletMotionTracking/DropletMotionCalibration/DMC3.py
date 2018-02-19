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

how_change_motors_LEFT = [[2, 1], [0, 1], [0, 1], [1, 1], [1, 1], [2, 1]]
how_change_motors_RIGHT = [[1, 0], [1, 0], [2, 0], [2, 0], [0, 0], [0, 0]]

bases = [np.array([0,1]),np.array([np.sqrt(3)/2,0.5]),np.array([np.sqrt(3)/2,-0.5])]
expected_travel_dirs = [0, -np.pi/3, -2*np.pi/3, -np.pi, 2*np.pi/3, np.pi/3]
x_boundaries = [6, 181, 352, 525, 696]
y_boundaries = [6, 178, 352, 525, 696]

class MyException(Exception):
    pass

def set_led(r, g, b):
    serial.write("cmd set_led rgb %d %d %d"%(r, g, b))

def set_motors(direction, mot0val, mot1val, mot2val):
    serial.write("cmd set_motors %d %d %d %d"%(direction, mot0val, mot1val, mot2val))
    #Once more, just to be sure.
    serial.write("cmd set_motors %d %d %d %d"%(direction, mot0val, mot1val, mot2val))

def move_steps(direction, num_steps):
    serial.write("cmd move_steps %d %d"%(direction, num_steps))
    #Once more, just to be sure.
 #   serial.write("cmd move_steps %d %d"%(direction, num_steps))

def stop_walk():
    serial.write("cmd stop_walk")
    #Once more, just to be sure.
  #  serial.write("cmd stop_walk")

def get_motor_vals_for_dir(x, direction):
    if direction is 0:
        return [0, x[0], x[1]]
    elif direction is 1:
        return [x[0], x[1], 0]
    elif direction is 2:
        return [x[0], 0, x[1]]
    elif direction is 3:
        return [0, x[0], x[1]]
    elif direction is 4:
        return [x[0], x[1], 0]
    elif direction is 5:
        return [x[0], 0, x[1]]
    else:
        print("Invalid direction request")

class RoboRealmInterface:

    def __init__(self):
        self.rr = RR_API()
        self.rr.Connect("localhost")   

    def get_pos(self):
        try:
            vals=(float(self.rr.GetVariable("FIDUCIAL_X_COORD")), float(self.rr.GetVariable("FIDUCIAL_Y_COORD")))
        except ValueError:
            print("RR didn't have the fiducial.")
            time.sleep(1)
            return self.get_rr_pos(self)
            return
        return np.array(vals)
    
    def get_orient(self):
        try:
            val = float(self.rr.GetVariable("FIDUCIAL_ORIENTATION"))
        except ValueError:
            print("RR didn't have the fiducial.")
            time.sleep(1)
            return self.get_rr_orient(self)
            return
        val = np.mod(val+180.,360.)-180.
        return val

    def wait(self):
        self.rr.WaitImage(10)

class Calibrator:
    
    current_motor_settings = [[0, 0], [0, 0], [0, 0]]

    data_points_for_straighten = 1000 #Number of data points we ask for while straightening.
    trusted_data_points = 200 #Number of data points we collect before worrying about running in to walls.
    spin_settings_history = [{}, {}]
    straight_settings_history = [{}, {}, {}]

    def __init__(self):
        self.rr = RoboRealmInterface()
        port_h = serial.open_serial_port("COM10")
        serial.set_global_port(port_h)

    def full_calibrate(self):
        #first, calibrate droplet spin for one direction. This will give us motor sign and relative values for the three motors.
        #then calibrate directions 0, 2, and 4 (or 1, 3, and 5)
        timestamp = time.strftime('%m-%d_%H%M')
        fileName = "data\\dmc_calib_" + timestamp
        self.calibrate_both_spins(fileName)
        self.calibrate_all_straight(fileName)
        for i in range(2):
            serial.write("cmd write_motor_settings")
        print("!!!\nDone calibrating!\n!!!")

    def calibrate_both_spins(self, fileName):
        self.calibrate_droplet_spin(True, fName=fileName) #CW
        self.calibrate_droplet_spin(False, fName=fileName) #CCW

    def calibrate_all_straight(self, fileName):
        for direction in range(6):
            self.calibrate_droplet_straight(direction, fName=fileName)

    def get_position_data_batch(self, direction, length):
        stop_walk()
        pos_list = np.zeros((length,2))
        orient_list = np.zeros(length)
        h = 0
        early_abort = False
        new_blobs = 0
        blobs= 0
        move_steps(direction, length)
        while (h<length) and not early_abort:
            pos_list[h] = self.rr.get_pos()
            orient_list[h] = self.rr.get_orient()
            self.rr.wait()
            if h>self.trusted_data_points:
                for x in x_boundaries:
                    if abs(pos_list[h][0]-x)<=15:
                        print("Blob position (%d, %d) too close to x=%d. h=%d"%(pos_list[h][0], pos_list[h][1], x, h))
                        early_abort = True
                for y in y_boundaries:
                    if abs(pos_list[h][1]-y)<=15:
                        print("Blob position (%d, %d) too close to y=%d. h=%d"%(pos_list[h][0], pos_list[h][1], y, h))
                        early_abort = True
            h += 1 #update counter            
        stop_walk()
        return (pos_list, orient_list, h)

    def test_spin_settings(self, motor_values, cw_q, output_stream):
        try:
            val = self.spin_settings_history[cw_q][str(motor_values)]
        except KeyError:
            '''We haven't done that one before.'''
        else:
            return val
        set_motors(7-cw_q, *motor_values)
        try:
            (pos_list, orient_list, num_good_points) = self.get_position_data_batch(7-cw_q, self.data_points_for_straighten)
        except MyException:
            return
        if num_good_points<0.8*self.data_points_for_straighten:
            print("Not enough good data.")
            return self.test_spin_settings(motor_values, cw_q, output_stream)
        (center, radius, residual, sign) = maths.lsq(pos_list, orient_list)
        radial_velocity = maths.get_radial_velocity(orient_list)
        print("mean delta orient: %f"%(radial_velocity))

        print("Radius: %f"%(radius))
        self.spin_settings_history[cw_q][str(motor_values)] = (radius, radial_velocity)
        orient_changed = orient_list[-1]-orient_list[0]
        output_stream.write("%f, %f, %f, %f, %f, %f\n"%(motor_values[0], motor_values[1], motor_values[2], radius, radial_velocity, orient_changed))
        output_stream.flush()
        return self.test_spin_settings(motor_values, cw_q, output_stream)

    def calibrate_droplet_spin(self, cw_q, fName=None):
        alpha = 1.  #reflection coefficient
        gamma = 2.  #expansion coefficient
        rho = -0.5  #contraction coefficient
        sigma = 0.5 #shrink coefficient

        x_0 = np.array([0, 0, 0])
        x_1 = np.array([-200, 200, 200])
        x_2 = np.array([200, -200, 200])
        x_3 = np.array([200, 200, -200])
        spx = np.array([x_0, x_1, x_2, x_3])
        old_spx = np.zeros((4,3))

        if fName is None:
            settings_history_fName = 'dmc_data_%s_all.csv'%("cw" if cw_q else "ccw")
            simplex_history_fName = 'dmc_data_%s_spx.csv'%("cw" if cw_q else "ccw")
        else:
            settings_history_fName = fName + "_%s_hist.csv"%("cw" if cw_q else "ccw")
            simplex_history_fName = fName + "_%s_spx.csv"%("cw" if cw_q else "ccw")

        settings_history_file = open(settings_history_fName,'w')
        settings_history_file.write('mot0set, mot1set, mot2set, radius, mean delta orient, total delta orient\n')

        simplex_history_file = open(simplex_history_fName,'w')
        simplex_history_file.write('spx00, spx01, spx02, rad0, spx10, spx11, spx12, rad1, spx20, spx21, spx22, rad2, spx30, spx31, spx32, rad3\n')

        finished = False
        def fun(x):
            (radius, radial_velocity) =self.test_spin_settings(x, cw_q, settings_history_file)
            desired_spin_dir = -1 if cw_q else 1
            if desired_spin_dir*radial_velocity>0:
                return radius - 10*abs(radial_velocity) #if we're spinning in the right direction
            else:
                return radius + 100*abs(radial_velocity) #if we're spinning in the wrong direction
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

                if finished:
                    spx = np.array(sorted(spx, key=fun))
                    for i in range(3):
                        set_motors(7-cw_q, *spx[0])
                    print("Done with spinning %s!"%("cw" if cw_q else "ccw"))
                    settings_history_file.close()
                    simplex_history_file.close()
                    return cw_q
        except KeyboardInterrupt:
            print("Calibration interrupted by user.")
            settings_history_file.close()
            simplex_history_file.close()

    def spin_for_safe_straight(self, direction):
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


        desired_orientation = desired_orientationInit + direction*60
        desired_orientation = np.mod(desired_orientation+180,360)-180
        
        print("direction: %d, temp_pos: (%f, %f), Desired Orientation: %f, Desired orientation, adjusted: %f"%(direction, temp_x, temp_y, desired_orientationInit, desired_orientation))

        while(True):
            last_orient = self.get_rr_orient()
            delta_orient = np.mod(desired_orientation-last_orient+180,360)-180
            if delta_orient>0:
                move_steps(7, 2000)
            else:
                move_steps(6,2000)

            count=0
            while count<200: #timeout of ~2 seconds of no movement.
                orient = self.get_rr_orient()
                #print("Orient: %f"%(orient))
                if abs(orient-last_orient)>0.5:
                    count=0
                self.rr.WaitImage(10)
                orient = np.mod(orient+180,360)-180
                if abs(desired_orientation-orient)<10:
                    print("Spin complete.")
                    return
                last_orient=orient
                count+=1
            print("Droplet not moving? Trying again.")


    def test_straight_settings(self, x, direction, output_stream):
        mot_vals = get_motor_vals_for_dir(x, direction)
        try:
            return self.straight_settings_history[direction][str(mot_vals)]
        except KeyError:
            '''We haven't done that one before.'''
        set_motors(direction,*mot_vals)
        try:
            self.spin_for_safe_straight(direction)
            (pos_list, orient_list, num_good_points) = self.get_position_data_batch(direction, self.data_points_for_straighten)
        except MyException:
            return
        if num_good_points<0.8*self.data_points_for_straighten:
            print("Not enough good data.")
            return self.test_straight_settings(mot_vals, direction, output_stream)
        (center, radius, residual, sign) = maths.lsq(pos_list, orient_list)
        distance_traveled = np.linalg.norm(pos_list[-1]-pos_list[0])
        travel_dir = maths.get_travel_dir(pos_list[0],pos_list[-1],orient_list[0],self.expected_travel_dirs[direction])
        self.straight_settings_history[direction][str(mot_vals)] = (radius, travel_dir, distance_traveled,)
        #print("settings_history%s = %f"%(str(motor_values), radius))
        #print("settings_history: %s"%(str(self.settings_history)))
        output_stream.write("%d, %d, %d, %f, %f, %f\n"%(mot_vals[0], mot_vals[1], mot_vals[2], radius, sign, distance_traveled))
        output_stream.flush()
        return self.test_straight_settings(mot_vals, direction, output_stream)

    def calibrate_droplet_straight(self, direction, fName=None):
        alpha = 1.  #reflection coefficient
        gamma = 2.  #expansion coefficient
        rho = -0.5  #contraction coefficient
        sigma = 0.5 #shrink coefficient

        x_0 = np.array([0,0])
        x_1 = np.array([100,-100])
        x_2 = np.array([-100,100])
        spx = np.array([x_0, x_1, x_2])
        old_spx = np.zeros((3,2))

        if fName is None:
            settings_history_fName = 'dmc_data_%s.csv'%("cw" if cw_q else "ccw")
            simplex_history_fName = 'dmc_data_%s.csv'%("cw" if cw_q else "ccw")
        else:
            settings_history_fName = fName + "_%s_hist.csv"%(direction)
            simplex_history_fName = fName + "_%s_spx.csv"%(direction)

        settings_history_file = open(settings_history_fName,'w')
        settings_history_file.write('mot0set, mot1set, mot2set, radius, sign, distance_traveled\n')

        simplex_history_file = open(simplex_history_fName,'w')
        simplex_history_file.write('spx00, spx01, spx02, rad0, spx10, spx11, spx12, rad1, spx20, spx21, spx22, rad2, spx30, spx31, spx32, rad3\n')

        finished = False
        def fun(x):
            (radius, travel_dir, dist_traveled) =self.test_straight_settings(x, direction, settings_history_file)
            return -radius - travel_dir*dist_traveled
        try:
            while(True):
                spx = np.array(sorted(spx, key=fun))
                if str(spx) == str(old_spx):
                    finished = True
                old_spx = copy.deepcopy(spx)

                func_values = [fun(point) for point in spx]
                output_array = np.array(map(lambda spx_point, radius: np.append(spx_point, radius), spx, func_values)).flatten().tolist()
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
                        for i in range(1,3):
                            spx[i] = (spx[0] + sigma*(spx[i]-spx[0])).astype(int)

                if finished:
                    set_motors(i, *spx[0])
                    print("We're done straightening direction %d"%direction)
                    settings_history_file.close()
                    simplex_history_file.close()
                    return cw_q
        except KeyboardInterrupt:
            print("Calibration interrupted by user.")
            settings_history_file.close()
            simplex_history_file.close()
            serial.cleanup()
    
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

    def calibrate_dist_per_step(self, direction, num_steps = 100, fName = None):
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
            move_steps(direction, num_steps)

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