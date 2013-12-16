# import rr # for RoboRealm handles
import serial
import sys
import time
import os
import platform
import csv
import numpy as np
import LP as lp # For Linear Programming code

baud = 115200
default_motor_value = 64
dat_file_name = os.path.expanduser('~')+'\\Desktop\\'+'droplet_pos'

def serial_write(port_h, data):
    """ Function serial_write(): Appends a newline character and writes <data> to opened serial port <port_h>.
    """
    if(port_h.closed):
        port_h.open()

    out_dat = data + '\n';
    port_h.write(out_dat)
    port_h.flushOutput()

# Just for John! :)
def serial_writeln(port_h, data):
    serial_write(port_h, data)

def open_serial_port(port):
    """ Function open_serial_port(port): Opens the serial port supplied as input (e.g. 'COM9') 
    and tests to make sure it's working. WARNING: The caller of this function is resonsible for 
    closing the serial port connection using <returned_port_handle>.close() when they are done.
    Returns:
    A handle to the serial port
    """
    # Open the serial port
    try:
        port_h = serial.Serial(port, baudrate=baud)
    except serial.SerialException:
        print("ERROR: Port " + port + " is already open!\n")
        return

    if(port_h.closed):
        port_h.open()


    # Blink some LEDs to make sure it's working
    serial_write(port_h, 'set_led rgb 0 5 5')
    time.sleep(.1)
    serial_write(port_h, 'set_led rgb 5 0 5')
    time.sleep(.1)
    serial_write(port_h, 'set_led rgb 5 5 0')
    time.sleep(.1)
    serial_write(port_h, 'set_led rgb 0 0 0')

    return port_h

# (N, S, NE, SW, SE, NW, CW, CCW) = (0, 2, 1, 4, 5, )

def init_motor_values(port_h, sleep_dur):

    #motor_values = {'N':'0 64 64', 'S':'0 -64 -64', 'SW':'64 64 0', 'NE':'-64 -64 0', 'SE':'64 0 64', 'NW':'-64 0 -64', 'CW':'-64 64 64', 'CCW':'64 -64 -64'}
    motor_values = {'0':'0 64 64', '1':'-64 -64 0', '2':'64 0 64', '3':'0 -64 -64', '4':'64 64 0', '5':'-64 0 -64', '6':'64 64 64', '7':'-64 -64 -64'}
    for i in range(3):
        for k in motor_values.keys():
            serial_write(port_h, 'cmd set_motor %s %s'%(k, motor_values[k]))
           # print('cmd set_motor %s %s'%(k, motor_values[k]))
           # sys.stdout.flush()
            time.sleep(sleep_dur)


def run_opt_phase(port_h, history = 10):
    """ Function run_opt_phase(port_h): Runs the optimization phase for droplet walking.
    This function runs almost all of it's code in a continuous loop.
    Arguments:
    port_h - The serial port to use when communicating with God Droplet
    history - Optional argument to control the number of data points to take before running the optimization routine.
    """
    try:
        datfile_h = open(dat_file_name,'rb')
    except (OSError, IOError) as e:
        print("ERROR: File not found, or something else went wrong when trying to open the data file.\n")
        return

    pos_list = np.zeros((history,2))
    h = 0
    try:
        while(True):
            if(h == history):
                # run opt phase
                print map(np.average, pos_list.transpose())
                sys.stdout.flush()
                # reset counter
                h = 0

            else:
                # gather droplet position data
                line_dat = datfile_h.readline()
                if(line_dat):
                    [obj_id, pos_x, pos_y] = map(int, line_dat.replace('\r\n', '').split(','))
                    pos_list[h] = np.array([pos_x, pos_y])
                    # update counter
                    h += 1

    except KeyboardInterrupt:
        print("Optmization phase interrupted by user. Don\'t forget to close the serial port!\n")
        datfile_h.close()
