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


baud = 115200
motor_lower_bound = 40
motor_upper_bound = 76
#Below checked for: 0, 3
change_motor_signs_LEFT = [[0, -0.5, 0.5], [0.5, -0.5, 0], [0.5, 0, -0.5], [0, -0.5, 0.5], [-0.5, 0.5, 0], [0.5, 0, -0.5]] #TODO: Implement for turning.
change_motor_signs_RIGHT = [[0, 0.5, -0.5], [-0.5, 0.5, 0], [-0.5, 0, 0.5], [0, 0.5, -0.5], [0.5, -0.5, 0], [-0.5, 0, 0.5]] #TODO: Implement for turning.
current_motor_settings=[[0, 0.666, 0.666], [-0.666, 0.666, 0], [-0.666, 0, -0.666], [0, -0.666, -0.666], [0.666, -0.666, 0], [0.666, 0, 0.666], [0.666, 0.666, 0.666], [-0.666, -0.666, -0.666]]
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
        print('\t', end='')
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

def change_motor(dir, change_values, port_h=False):
    if not port_h:
        port_h = default_port
    for i in range(3):
        current_motor_settings[dir][i]+= change_values[i]
        if current_motor_settings[dir][i]>1.0:
            current_motor_settings[dir][i]=1.0
        if current_motor_settings[dir][i]<-1.0:
            current_motor_settings[dir][i]=-1.0
    print("%d: %f %f %f"%(dir, current_motor_settings[dir][0], current_motor_settings[dir][1], current_motor_settings[dir][2]))
    set_motor(dir,*map(get_raw_motor,current_motor_settings[dir]))

def blinky_led_party(port_h=False):
    if not port_h:
        port_h = default_port
    serial_write('cmd set_led rgb 0 100 100', port_h)
    serial_write('cmd set_led rgb 100 0 100', port_h)
    serial_write('cmd set_led rgb 100 100 0', port_h)
    serial_write('cmd set_led rgb 0 0 0', port_h)

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

def run_opt_phase(blob_id, dir, port_h=False, history = 120):
    """ Function run_opt_phase(port_h): Runs the optimization phase for droplet walking.
    This function runs almost all of it's code in a continuous loop.
    Arguments:
    port_h - The serial port to use when communicating with God Droplet
    history - Optional argument to control the number of data points to take before running the optimization routine.
    """
    if not port_h:
        port_h = default_port

    pos_list = np.zeros((history,2))
    h = 0
   #last_side=0 #0 if we haven't set this before, 1 if last side was left, -1 if last side was right. #OTHER CONTROL
    #cross_count=0 #number of times we've switched from right side to left side. #OTHER CONTROL
    try:
        walk(dir, 60)

        while(True):
            if(h == history):
                #update motor values
                serial_write('cmd stop_walk', port_h)
                (center, radius, residual, sign) = fc.lsq(pos_list)
                print("center: (%f,%f), rad: %f, residu: %f >> "%(center[0], center[1], radius,residual), end='')
                prop=get_prop_control_value(radius) #PROPORTIONAL CONTROL
                #prop=get_other_control_value(cross_count) #OTHER CONTROL
                if(sign>0):
                    print("Left")
                    #if last_side<0: #OTHER CONTROL
                    #    cross_count+=1 #OTHER CONTROL
                    #last_side=1 #OTHER CONTROL
                    change_values = map(lambda x: x*prop, change_motor_signs_LEFT[dir])
                    change_motor(dir, change_values, port_h)                    
                    sys.stdout.flush()
                else:
                    print("Right")
                    #if last_side>0: #OTHER CONTROL
                    #    cross_count+=1 #OTHER CONTROL
                    #last_side=-1 #OTHER CONTROL
                    change_values = map(lambda x: x*prop, change_motor_signs_RIGHT[dir])
                    change_motor(dir, change_values, port_h)  
                    sys.stdout.flush()
                dir = (dir+3)%6
                walk(dir, 60)
                h = 0
            else:
                # gather droplet position data
                blobs = sc.update_rr_data()
                try:
                    pos_list[h] = np.array(blobs[blob_id])
                except KeyError:
                    print("I lost my droplet friend!!! T_T")
                    serial_write('cmd stop_walk', port_h)
                    break
                # update counter
                h += 1

    except KeyboardInterrupt:
        print("Optimization phase interrupted by user. Don\'t forget to close the serial port!\n")
        serial_write('cmd stop_walk', port_h)

open_serial_port("COM9")