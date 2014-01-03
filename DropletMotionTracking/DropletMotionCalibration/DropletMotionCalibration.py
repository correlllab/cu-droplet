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
motor_lower_bound = 30
motor_upper_bound = 75
current_motor_settings=[[0, 0.5, 0.5], [-0.5, -0.5, 0], [0.5, 0, 0.5], [0, -0.5, -0.5], [0.5, 0.5, 0], [-0.5, 0, -0.5], [0.5, 0.5, 0.5], [-0.5, -0.5, -0.5]]
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

    dat = port_h.readline()
    #sys.stdout.write(dat)
    print(dat, end='')

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
        settings = ' '.join(map(str,map(get_raw_motor,current_motor_settings[i])))
        serial_write('cmd set_motor %d %s'%(i, settings), port_h)
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

def update_motor(dir, port_h=False):
    if not port_h:
        port_h = default_port
    settings = ' '.join(map(str,map(get_raw_motor,current_motor_settings[dir])))
    #print("Setting %d to %s, sending:"%(dir, settings))
    #print('cmd set_motor %d %s'%(dir, settings))
    serial_write('cmd set_motor %d %s'%(dir, settings), port_h)

def change_motor(dir, val0, val1, val2, port_h=False):
    if not port_h:
        port_h = default_port
    current_motor_settings[dir][0]+= val0
    current_motor_settings[dir][1]+= val1
    current_motor_settings[dir][2]+= val2 
    print("New Values: %s"%(str(current_motor_settings[dir])))
    update_motor(dir)

def blinky_led_party(port_h=False):
    if not port_h:
        port_h = default_port
    serial_write('cmd set_led rgb 0 100 100', port_h)
    serial_write('cmd set_led rgb 100 0 100', port_h)
    serial_write('cmd set_led rgb 100 100 0', port_h)
    serial_write('cmd set_led rgb 0 0 0', port_h)

def run_opt_phase(blob_id, port_h=False, history = 80):
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
    try:
        serial_write('cmd walk 0 60', port_h)

        while(True):
            if(h == history):
                # run opt phase
                (center, radius, residual) = fc.lsq(pos_list)
                # reset counter
                first_pos = pos_list[1]
                last_pos = pos_list[-1]
                sign = np.linalg.det(np.array([last_pos-first_pos, center-first_pos]).transpose())
                serial_write('cmd stop_walk', port_h)
               # blinky_led_party()
                #time.sleep(0.5) #We think that this is to make sure it finishes the last step (so it gets to the if cancelled line at top of take_step)
                if(sign>0):
                    change_motor(0,0,-0.05,0.05, port_h)                    
                    print("center: (%f,%f), rad: %f, residu: %f, left"%(center[0], center[1], radius,residual))
                    sys.stdout.flush()
                else:
                    change_motor(0,0,0.05,-0.05, port_h)  
                    print("center: (%f,%f), rad: %f, residu: %f, right"%(center[0], center[1], radius,residual))
                    sys.stdout.flush()
                serial_write('cmd walk 0 60', port_h)
                h = 0
            else:
                # gather droplet position data
                blobs = sc.update_rr_data()
                #print [pos_x, pos_y]
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