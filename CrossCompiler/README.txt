The cross compiler directory is a stand alone directory. The CrossCompile Python scirpt takes a simulator droplet program and converts it to a hardware program that can be compiled and ran by Atmel Studio.

Run:
>Python CrossCompiler.py
OR:
>Python CrossCompiler.py src\DropletProgram.cpp DropletProgramHardware.c


Note:
ReadGlobals(writer) assumes that the global constants are immediately preceded and followed by some kind of comments indicating type of constants. eg. begin and end
