@ECHO OFF
:begin

REM SET n=64
REM SET p=1.0
REM SET t=6000

SET /a i=1
SET /a stop=%1+1
:loop
REM Begin loop
IF %i%==%stop% GOTO END

REM Do loop stuff here
ECHO %i%

REM Create a unique file name for storing output
REM SET t=%time::=-%
REM SET t=%t:.=-%
REM SET t=%t: =%
REM SET d=%date:/=-%
REM SET d=%d: =-%
SET fname=PhysicsSim-64-%i%.txt

REM Run the Simulator
START /WAIT /MIN DropletConsole.exe -f %fname% -n 64 -p 0.1 -t 12000

SET /a i=%i%+1
GOTO LOOP
REM End loop

:end