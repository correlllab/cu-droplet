import sys
import os
import re
# error handle function
def error(message):
    sys.stderr.write("error: %s\n" % message)
    sys.exit(1)

def main():
    """ set input and output files depending on command
    case1: run script on all dropletPrograms
    case2: run script on a specific program using:
    Python CrossCompiler src/DropletProgramIn.cpp DropletProgramOut.c
    """
    if len(sys.argv) == 1:
        inputFile = ['src/DropletCommTest.cpp', 'src/DropletMarch.cpp', 'src/DropletRainbow.cpp',
                     'src/DropletRandomWalk.cpp', 'src/DropletRGBSense.cpp', 'src/DropletStickPullers.cpp',
                     'src/DropletTurnTest.cpp', 'src/DropletPowerTest.cpp', 'src/DropletStickPullersUpdated.cpp']
        outputFile = ['DropletCommTestHardware.c', 'DropletMarchHardware.c', 'DropletRainbowHardware.c',
                      'DropletRandomWalkHardware.c','DropletRGBSenseHardware.c', 'DropletStickPullersHardware.c',
                      'DropletTurnTestHardware.c', 'DropletPowerTestHardware.c', 'DropletStickPullersHardwareUpdated.c']
    elif len(sys.argv) == 3:
        inputFile = sys.argv[1]
        outputFile = sys.argv[2]
    else:
        error("Invalid arguments. Input should be:Python crossCompile.py newPorgram.c oldProgram.cpp");
    """ open files to write """
    if len(sys.argv) == 1:
        for i in range(len(inputFile)):
            # opening file for writing
            try:
                writer = open(outputFile[i], "w")
            except IOError:
                print ("Can not open file:", outputFile[i])
                exit(0)
            print("Opening and Writing output in File:", outputFile[i])
            fileReadWrite(inputFile[i],writer)
            writer.close()
            print("Writing Completed...")
    elif len(sys.argv) == 3:
        # opening file for writing
        try:
            writer = open(outputFile, "w")
        except IOError:
            print ("Can not open file:", outputFile)
            exit(0)
            
        print("Opening and Writing output in File:", outputFile)
        fileReadWrite(inputFile,writer)
        writer.close()
        print("Writing Completed...")
    
""" attach global definitions to code"""
def readGlobals(writer):
    try:
        reader = open("include/DropletSimGlobals.h")
        print("Writing Globals from: include/DropletSimGlobals.h")
    except IOError:
        print("Cannot Find the File 'include/DropletSimGlobals.h'")
        exit(0)
    
    """ Following code assumes that the global constants are immediately 
    preceded and followed by some kind of comments indicating type of constants.
    eg. begin and end
    """
    writer.write('\n')
    while True:
        line = reader.readline()
        if len(line) == 0:
            break
        elif line.find('begin') != -1:
            writer.write(line)
            line = reader.readline()
            while line.find('end') == -1:
                writer.write(line)
                line = reader.readline()
            writer.write(line)
    writer.write('\n')
    
""" read in header files to initialize programs """
def readHeaderFile(headerFile, writer):
    writer.write('\t//code from header droplet program\n')
    try:
        # look for dropletProgram.h
        file = "include"+headerFile
        if not os.path.isfile(file):
            print("Not Exist: ",headerFile)
        else:
            # open file
            print("Reading: "+file)
            reader = open(file,"r")
            line = reader.readline()
                    
            #Loops around till we reach class statement
            while(len(line) != 0 and line.find('class') == -1):
                line = reader.readline()
                        
                    
            if(len(line) != 0):
                #Captures the class name to identify constructors and destructors
                className = re.findall(r'[\s]([^\s]+)[\s]', line)[0]
                line = reader.readline()
                #Discards empty lines and Loops around till we reach first '{'
                while(len(' '.join(line.split())) == '0'):    line = reader.readline()
                        
                #Discards the first '{' after class
                if(len(' '.join(line.split())) == 1):
                    line = reader.readline()
                else:
                    line = (' '.join(line.split()))[1:]
                            
                #Runs till class ends
                while(line.find('};') == -1):
                    if(line.find('private') == -1 and line.find('public') == -1 and line.find(className+'(') == -1
                        and line.find('DropletInit') == -1 and line.find('DropletMainLoop') == -1):
                        writer.write(('\t'+' '.join(line.split()))+'\n')
                    line = reader.readline()              
    except IOError:
        print("Exception in reading Header Files")
        exit(0)

""" write hardware includes """
def includes(ipFile, writer):
    print("Writing includes")
    writer.write('#define F_CPU 32000000UL\n\n')
    writer.write('#include <avr/io.h>\n')
    writer.write('#include <math.h>\n')
    writer.write('#include <util/delay.h>\n\n')
    writer.write('#include "motor.h"\n')
    writer.write('#include "RGB_LED.h"\n')
    writer.write('#include "pc_com.h"\n')
    writer.write('#include "RGB_sensor.h"\n')
    writer.write('#include "power.h"\n')
    writer.write('#include "random.h"\n')
    writer.write('#include "IRcom.h"\n')
    writer.write('\n')
    writer.write('extern uint8_t rsenbase,gsenbase,bsenbase;\n')
    
""" write dropletProgram code to new hardware program """
def fileReadWrite(ipFile, writer):
    # look for dropletProgram.cpp
    try:
        reader = open(ipFile, "r")
        print ('Reading:', ipFile)
    except IOError:
        print ("Can not find the filerr: " + ipFile)
        exit(0)
        
    includes(ipFile, writer)

    # set up tmp variable to the different file names tmp=/progName, tmp1=progName
    tmp = ipFile.strip("src")
    print(tmp)
    tmp = tmp.strip('.cpp')
    print(tmp)
    tmp1 = tmp.strip('/')
    print(tmp1)
    # main code to read header program
    while True:
        line = reader.readline();
        if len(line) == 0:
            break
        # look for DropletInit() and then call readGlobals and readHeaderFile
        elif line.find('DropletInit()') != -1:
            readGlobals(writer)
            print("Writing needed headers")
            tmp2 = tmp+".h"
            readHeaderFile(tmp2, writer)
        
        # look for back_to_search function
        # This should be automaticely generated
        # without looking for a specific function
        elif line.find(tmp1+'::back_to_search()') != -1:
            print(tmp1)
            writer.write(line.replace(tmp1+"::","",1))
            line = reader.readline()
            writer.write(line)
            bracketCount = 1
            while bracketCount != 0:
                line = reader.readline()
                if line.find('}') != -1:
                    bracketCount = bracketCount - 1
                    writer.write(line)
                    if bracketCount != 0:
                        writer.write(line)
                elif line.find('{') != -1:
                    bracketCount = bracketCount + 1
                    writer.write(line)
                else:
                    if line.find('::') != -1:
                        writer.write(line.replace("phases::","",1))
                    else:
                        writer.write(line)
    reader.close()

    # main code to read hardware program
    try:
        reader = open(ipFile, "r")
        print ('Reading: '+ipFile)
    except IOError:
        print ("Can not find the file: " + ipFile)
        exit(0)
        
    while True:
        line = reader.readline();
        if len(line) == 0:
            break
        elif line.find('DropletInit()') != -1:
            writer.write("int main()\n")
            line = reader.readline()
            if line.find("{") != -1:
                writer.write(line)
            else:
                writer.write("{\n")
            writer.write('\t//initialize all systems\n')
            writer.write('\tinit_all_systems();\n')
            writer.write("\t//set blue color to blink twice\n")
            writer.write('\tset_blue_led(50); _delay_ms(250); led_off();\n')
            writer.write('\t_delay_ms(250); set_blue_led(50); _delay_ms(250); led_off();\n')
            writer.write("\n")

            # write dropletInit code
            bracketCount = 1
            while bracketCount != 0:
                line = reader.readline()
                if line.find('}') != -1:
                    bracketCount = bracketCount - 1
                    if bracketCount != 0:
                        writer.write(line)
                elif line.find('{') != -1:
                    bracketCount = bracketCount + 1
                    writer.write(line)
                else:
                    if line.find('reset_all_systems();') != -1:
                        pass
                    elif line.find('::') != -1:
                        writer.write(line.replace("phases::","",1))
                    else:
                        writer.write(line)
                        
        # write mainLoop code  
        elif line.find('DropletMainLoop') != -1:
            writer.write('\twhile(1)\n\t{')
            line = reader.readline()
            if line.find("{") != 1:
                writer.write("\t"+line[1:])
            bracketCount = 1
            while bracketCount != 0:
                line = reader.readline()
                if line.find('}') != -1:
                    bracketCount = bracketCount - 1
                    if bracketCount != 0:
                        writer.write("\t"+line)
                elif line.find('{') != -1:
                    bracketCount = bracketCount + 1;    writer.write("\t"+line)
                else:
                    if line.find('::') != -1:
                        writer.write(line.replace("phases::","",1))
                    else:
                        writer.write(line)
            writer.write('\t}\n')
        
    writer.write('}\n')
    reader.close()
    
    
if __name__=="__main__":
    main()
