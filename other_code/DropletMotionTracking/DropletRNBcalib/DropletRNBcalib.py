from __future__ import print_function
import comtypes
import comtypes.client as cc
import time
import sys
import MySerialWrapper as serial
import re

rr = cc.CreateObject("RoboRealm.API.1")

rr.Startup()

def printRRData(file, rootID):
    rr.Pause()
    time = rr.GetVariable("RUN_TIME")
    names = rr.GetArrayVariable("FIDUCIAL_NAME_ARRAY")[0]
    xPositions = rr.GetFloatArrayVariable("FIDUCIAL_X_COORD_ARRAY")[0]
    yPositions = rr.GetFloatArrayVariable("FIDUCIAL_Y_COORD_ARRAY")[0]
    orientations = rr.GetFloatArrayVariable("FIDUCIAL_ORIENTATION_ARRAY")[0]
    rr.Resume()
    numFids = len(names)
    if (numFids is not len(xPositions)) or (numFids is not len(yPositions)) or (numFids is not len(orientations)):
        print('ERROR: arrays not same length.')
        file.write('\n')
        return
    str = "{0:.3f}, {{".format(float(time))
    print(str)
    file.write(str)
    for i in range(numFids):
        try:
            nameMatch = re.match(r"\s*\\([0-9A-F]{4}|ROOT)",names[i])
            id = nameMatch.group(1)
        except AttributeError:
            print("Error: No match for namestr: {0}".format(names[i]))
            file.write('\n')
            return
        if id == "ROOT":
            id = rootID
        str = '"{0}"->{{{1:.0f}, {2:.0f}, {3:.1f}}}'.format(id, xPositions[i], yPositions[i], orientations[i])
        print(str,end="")
        file.write(str)
        if i is not (numFids-1):
            print(", ",end="")
            file.write(", ")
    print("}},")
    file.write('}},\n')
    
timeStr = time.strftime("%d%m%Y_%H%M%S")
root = "ROOT"

with open("rnbCalibData_{0}.txt".format(timeStr),'w') as f:
    with serial.Wrapper("COM3") as port:
        while True:
            while port.inWaiting() > 0:
                data = port.readLine()
                data = data.strip()
                idMatch = re.match("My ID is: ([0-9A-F]{4})", data)
                dataMatch = re.match('\{"([0-9A-F]{4})", "([0-9A-F]{4})", \{\{-?\d+,-?\d+,-?\d+,-?\d+,-?\d+,-?\d+\},\{-?\d+,-?\d+,-?\d+,-?\d+,-?\d+,-?\d+\},\{-?\d+,-?\d+,-?\d+,-?\d+,-?\d+,-?\d+\},\{-?\d+,-?\d+,-?\d+,-?\d+,-?\d+,-?\d+\},\{-?\d+,-?\d+,-?\d+,-?\d+,-?\d+,-?\d+\},\{-?\d+,-?\d+,-?\d+,-?\d+,-?\d+,-?\d+\}\}\},', data)
                if idMatch:
                    root = idMatch.group(1)
                elif dataMatch:
                    root = dataMatch.group(2)
                    print(data[:-2],end=', ')
                    f.write(data[:-2])
                    f.write(', ')
                    printRRData(f, root)
                else:
                    print("Unexpected line?!")
                    print("\t",end="")
                    print(data)
            sys.stdout.flush()
            f.flush()
            time.sleep(0.1)