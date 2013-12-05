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
dat_file_name = os.path.expanduser('~')+'\\Desktop\\'+'droplet_pos'

def serial_write(port_h, data):
    if(port_h.closed):
        port_h.open()

    out_dat = data + '\n';
    port_h.write(out_dat)
    port_h.flushOutput()

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

def run_opt_phase(port_h, history = 100):
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
                # gather droplet position data from the serial port
                line_dat = datfile_h.readline()
                if(line_dat):
                    [obj_id, pos_x, pos_y] = map(int, line_dat.replace('\r\n', '').split(','))
                    pos_list[h] = np.array([pos_x, pos_y])

                    # update counter
                    h += 1

                else:
                    continue

    except KeyboardInterrupt:
        print("Optmization phase interrupted by user. Don\'t forget to close the serial port!\n")
        datfile_h.close()
