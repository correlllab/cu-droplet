from __future__ import print_function
import serial
import sys
import time
import os
import platform
import csv
import numpy as np
import LP as lp # For Linear Programming code
import FindCircle as fc
import SocketConnection as sc
import copy
from collections import Counter

baud = 115200
motor_lower_bound = 40
motor_upper_bound = 76

class MyException(Exception):
    pass

class PositionException(MyException):
    pass

def reset_motor_values(dir=-1):
    global current_motor_settings
    if dir<0:
        current_motor_settings=[[0.666*i for i in subList] for subList in default_motor_settings]
    else:
        current_motor_settings[dir]=[0.666*i for i in default_motor_settings[dir]]

change_motor_signs_LEFT = [[0, -0.5, 0.5], [-0.5, 0.5, 0], [-0.5, 0, 0.5], [0, 0.5, -0.5], [0.5, -0.5, 0], [0.5, 0, -0.5]] #TODO: Implement for turning.
change_motor_signs_RIGHT = [[0, 0.5, -0.5], [0.5, -0.5, 0], [0.5, 0, -0.5], [0, -0.5, 0.5], [-0.5, 0.5, 0], [-0.5, 0, 0.5]] #TODO: Implement for turning.
motors_involved=[[1,2],[0,1],[0,2],[1,2],[0,1],[0,2]]
default_motor_settings=[[0, 1, -1], [-1, 1, 0], [-1, 0, 1], [0, -1, 1], [1, -1, 0], [1, 0, -1], [1, 1, 1], [-1, -1, -1]]
reset_motor_values()
#dat_file_name = os.path.expanduser('~')+'\\Desktop\\'+'droplet_pos'

def open_serial_port(port):
    """ Function open_serial_port(port): Opens the serial port supplied as input (e.g. 'COM9') 
    and tests to make sure it's working. WARNING: The caller of this function is responsible for 
    closing the serial port connection using <returned_port_handle>.close() when they are done.
    Returns:
    A handle to the serial port
    """
    # Open the serial port
    try:
        port_h = serial.Serial(port, baudrate=baud,timeout=5)
    except serial.SerialException:
        print("ERROR: Port " + port + " is already open!\n")
        return

    if(port_h.closed):
        port_h.open()

    # Blink some LEDs to make sure it's working
    serial_write('set_led rgb 0 5 5', port_h)
    serial_write('set_led rgb 5 0 5', port_h)
    serial_write('set_led rgb 5 5 0', port_h)
    serial_write('set_led rgb 0 0 0', port_h)

    global default_port
    default_port = port_h

    return port_h

def serial_read(port_h=False):
    if not port_h:
        port_h = default_port

    while port_h.inWaiting() > 0:
        dat = port_h.readline()
        print('\t\t', end='')
        print(dat.strip())
    #sys.stdout.write(dat)


def serial_write(data, port_h=False):
    """ Function serial_write(): Appends a newline character and writes <data> to opened serial port <port_h>.
    """
    if not port_h:
        port_h = default_port

    if(port_h.closed):
        port_h.open()

    out_dat = data + '\n';
    port_h.write(out_dat)
    port_h.flushOutput()
    time.sleep(0.5)
    serial_read(port_h)

def testRR():
    print(rr.GetArrayVariable("BLOB_TRACKING"))

# (N, S, NE, SW, SE, NW, CW, CCW) = (0, 2, 1, 4, 5, )
def init_motor_values(port_h=False):
    
    if not port_h:
        port_h = default_port

    #for i in range(3):
    for i in range(8):
        set_motor(i, *map(get_raw_motor,current_motor_settings[i]))
           # print('cmd set_motor %s %s'%(k, motor_values[k]))
           # sys.stdout.flush()

def get_default_port():
    return default_port

def get_raw_motor(val):
    ret_val = int(round(abs(val)*(motor_upper_bound-motor_lower_bound)+motor_lower_bound))
    if val>0:
        return ret_val
    elif val<0:
        return -ret_val
    else:
        return 0

def get_motor_value(raw_val):
    ret_val = (abs(raw_val)-motor_lower_bound)/(1.0*(motor_upper_bound-motor_lower_bound))
    if raw_val>0:
        return ret_val
    elif raw_val<0:
        return -ret_val
    else:
        return 0

def change_motor(dir, change_values, port_h=False):
    if not port_h:
        port_h = default_port
    for i in range(3):
        current_motor_settings[dir][i] = (abs(current_motor_settings[dir][i])+change_values[i])*np.sign(current_motor_settings[dir][i])
        if current_motor_settings[dir][i]>1.0:
            current_motor_settings[dir][i]=1.0
        if current_motor_settings[dir][i]<-1.0:
            current_motor_settings[dir][i]=-1.0
    print("%d: %f %f %f"%(dir, current_motor_settings[dir][0], current_motor_settings[dir][1], current_motor_settings[dir][2]))
    set_motor(dir,*map(get_raw_motor,current_motor_settings[dir]))

def get_prop_control_value(radius):
    val=5/(radius+20)
    if val>0.2:
        val=0.2
    return val

def get_other_control_value(cross_count):
    exp = 4-cross_count
    if exp<0:
        return 0
    return (2**exp)/100.0

def walk(dir, steps, port_h=False):
    if not port_h:
        port_h = default_port
    serial_write("cmd walk %d %d"%(dir, steps), port_h)

def set_motor(dir, mot0, mot1, mot2, port_h=False):
    if not port_h:
        port_h = default_port
    serial_write("cmd set_motor %d %d %d %d"%(dir, mot0, mot1, mot2), port_h)

def calibrate_droplet(blob_id, dir=-1, port_h=False):
    if not port_h:
        port_h = default_port
    if dir<0:
        directions=[0,1,2]
    else:
        directions=[dir]
    settings_history = {}
    for dir in directions:
        if pick_sign(blob_id, dir, port_h)<0:
            print("Error in pick_sign")
            return
        try:
            settings_history.update(straighten_direction(blob_id, dir, port_h))
        except TypeError:
            print("Error in run_opt_phase")
            return
    for dir in directions:
        most_freq = settings_history[dir].most_common(1)[0][0]
        current_motor_settings[dir] = [get_motor_value(int(val)) for val in most_freq.split()]
        most_freq = settings_history[(dir+3)%6].most_common(1)[0][0]
        current_motor_settings[(dir+3)%6] = [get_motor_value(int(val)) for val in most_freq.split()]
        #np.linalg.solve()

bad_x_vals = [14,180,345,510,670]
bad_y_vals = [8,339,659]
def get_position_data_batch(dir, blob_id, length, port_h=False, escaping=False):
        if not port_h:
            port_h = default_port
        serial_write("cmd stop_walk", port_h)
        pos_list = np.zeros((length,2))
        h = 0
        try:
            walk(dir, length/2)
            while(h<length):
                # gather droplet position data
                blobs = sc.update_rr_data()
                try:
                    new_pos = np.array(blobs[blob_id])
                except KeyError:
                    serial_write('cmd stop_walk', port_h)
                    raise MyException("I lost my droplet friend!!! T_T")
                if not escaping:
                    for x in bad_x_vals:
                        if abs(new_pos[0]-x)<=15:
                            raise PositionException("Blob position (%d, %d) too close to x=%d"%(new_pos[0], new_pos[1], x))
                    for y in bad_y_vals:
                        if abs(new_pos[1]-y)<=15:
                            raise PositionException("Blob position (%d, %d) too close to y=%d"%(new_pos[0], new_pos[1], y))
                pos_list[h] = new_pos
                # update counter
                h += 1
        except KeyboardInterrupt:
            serial_write('cmd stop_walk', port_h)
            raise MyException("Optimization phase interrupted by user.")
        serial_write('cmd stop_walk', port_h)
        return pos_list
    
def carefully_get_position_data_batch(dir, blob_id, length, port_h=False):
    if not port_h:
        port_h = default_port
    for attempt in range(1,10):
        try:
            pos_list = get_position_data_batch(dir, blob_id, length, port_h)
        except PositionException as posErr:
            print(posErr)
            print("Trying to avoid edge, attempt: %d."%attempt)
            get_position_data_batch((dir+3)%6, blob_id, attempt*length, port_h, escaping=True)#walk the other way for a bit.
            continue
        except MyException as myErr:
            print(myErr)
            raise MyException()
        return pos_list
    print("Still too close after ten attempts. Giving up.")
    raise MyException()

def pick_sign(blob_id, dir, port_h=False, history = 60):
    """ Function straighten_direction(port_h): Runs the optimization phase for droplet walking.
    This function runs almost all of it's code in a continuous loop.
    Arguments:
    port_h - The serial port to use when communicating with God Droplet
    history - Optional argument to control the number of data points to take before running the optimization routine.
    """
    if not port_h:
        port_h = default_port
    raw_settings = map(get_raw_motor,current_motor_settings[dir])
    possible_signs = [copy.deepcopy(current_motor_settings[dir]) for i in range(4)]
    possible_signs[0][motors_involved[dir][0]]*=-1
    possible_signs[1][motors_involved[dir][1]]*=-1
    possible_signs[2][motors_involved[dir][0]]*=-1
    possible_signs[2][motors_involved[dir][1]]*=-1

    distances = []
    radii = []
    for setting in possible_signs:
        set_motor(dir, *map(get_raw_motor, setting))
        try:
            pos_list = get_position_data_batch(dir, blob_id, history, port_h)
        except MyException:
            return -1
        (center, radius, residual, sign) = fc.lsq(pos_list)
        distance = np.linalg.norm(pos_list[0]-pos_list[-1])
        radii.append(radius)
        distances.append(distance)
        print("rad: %f, dist: %f"%(radius,distance))

    radiiA = (radii[0]+radii[1])/2.0
    radiiB = (radii[2]+radii[3])/2.0
    if radiiA>radiiB:
        forward=possible_signs[0]
        backward=possible_signs[1]
    else:
        forward=possible_signs[2]
        backward=possible_signs[3]

    '''At this point, we have the signs right, except we need to figure out if it's going forward or backward.'''
    current_motor_settings[dir]=copy.deepcopy(forward)
    set_motor(dir, *map(get_raw_motor, forward))
    try:
        pos_listA = get_position_data_batch(dir, blob_id, history, port_h)
    except MyException:
        return -1
    change_values = map(lambda x: x*0.333, change_motor_signs_LEFT[dir])
    change_motor(dir, change_values, port_h) 
    try:
        pos_listB = carefully_get_position_data_batch(dir, blob_id, history, port_h)
    except MyException:
        return -1

    (centerA, radiusA, residualA, signA) = fc.lsq(pos_listA)
    (centerB, radiusB, residualB, signB) = fc.lsq(pos_listB)
    
    print("radA: %f, signA: %f, radB: %f, signB: %f"%(radiusA, signA, radiusB, signB))
    if signA>0:
        if radiusA>radiusB and signB>0:
            print("Center on left, but radius got smaller. Switching.")
            (forward, backward) = (backward, forward)
        else:
            print("Center on left, and radius got bigger. Should be good.")
    elif signA<0:
        if radiusA<radiusB or signB>0:
            print("Center on right, but radius got bigger. Switching.")
            (forward, backward) = (backward, forward)
        else:
            print("Center on right, and radius got smaller. Should be good.")

    current_motor_settings[dir]=forward
    current_motor_settings[(dir+3)%6]=backward
    set_motor(dir, *map(get_raw_motor, forward))
    set_motor((dir+3)%6, *map(get_raw_motor, backward))
    return 1 #successful completion


def straighten_direction(blob_id, dir, port_h=False, history = 120):
    """ Function straighten_direction(port_h): Runs the optimization phase for droplet walking.
    This function runs almost all of it's code in a continuous loop.
    Arguments:
    port_h - The serial port to use when communicating with God Droplet
    history - Optional argument to control the number of data points to take before running the optimization routine.
    """
    settings_history = {}
    settings_history[dir] = Counter()
    settings_history[(dir+3)%6] = Counter()
    if not port_h:
        port_h = default_port
    init_dir = dir
    while(True):
        try:
            pos_list = carefully_get_position_data_batch(dir, blob_id, history, port_h)
        except MyException:
            return None
        (center, radius, residual, sign) = fc.lsq(pos_list)
        print("center: (%f,%f), rad: %f, residu: %f >> "%(center[0], center[1], radius,residual), end='')
        prop=get_prop_control_value(radius)
        settings_history[dir]["%d %d %d"%tuple(map(get_raw_motor,current_motor_settings[dir]))]+=1
        if(sign>0):
            print("Left")
            change_values = map(lambda x: x*prop, change_motor_signs_LEFT[dir])
            change_motor(dir, change_values, port_h)                    
            sys.stdout.flush()
        else:
            print("Right")
            change_values = map(lambda x: x*prop, change_motor_signs_RIGHT[dir])
            change_motor(dir, change_values, port_h)  
            sys.stdout.flush()
        dir = (dir+3)%6
        if dir is init_dir:
            if settings_history[dir].most_common(1)[0][1]>4 and settings_history[(dir+3)%6].most_common(1)[0][1]>4:
                return settings_history #successful completion

open_serial_port("COM9")