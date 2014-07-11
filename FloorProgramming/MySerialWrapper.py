from __future__ import print_function
import serial
import time
def open_serial_port(port):
    """ Function open_serial_port(port): Opens the serial port supplied as input (e.g. 'COM9') 
    and tests to make sure it's working. WARNING: The caller of this function is responsible for 
    closing the serial port connection using <returned_port_handle>.close() when they are done.
    Returns:
    A handle to the serial port
    """
    # Open the serial port
    try:
        port_h = serial.Serial(port, baudrate=4800,timeout=5)
    except serial.SerialException:
        print("ERROR: Port " + port + " is already open!\n")
        return

    if(port_h.closed):
        port_h.open()

    return port_h

def set_global_port(port_h):
    global global_port
    global_port = port_h

def cleanup():
    global_port.close()

def read():
    while global_port.inWaiting() > 0:
        dat = global_port.readline()
        print('\t\t', end='')
        print(dat.strip())

def write(data):
    out_dat = data + '\r\n';
    global_port.write(out_dat)
    global_port.flushOutput()
