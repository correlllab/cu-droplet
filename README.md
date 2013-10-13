cu-droplet
==========

Droplets: The Correll Lab Swarm Robotics Platform

The platform is envisioned to be a scalable robotics experimentation environment. It consists of three major components, the hardware, the embedded software and the software simulation environment.

The hardware component comprises of the actual robot and the test bed/floor that experiments are run on. The robots are Ping-Pong ball sized devices capable of sensing (RGB color and IR sensing), actuation using vibration motors and communication, using analog/digital IR sensors. The embedded software layer is what we use to write and compile on-board code for the droplets. Each robot is equipped with an Atmel xmega-128a3u microprocessor capable of running embedded C-code. For more information on the hardware and low-level software API I would recommend reading complete write-up.

	

The software simulation environment consists of a simulator for the entire platform that allows you to run tests using droplet code on your PC. The idea here is to simulate the above two layers, using C/C++, as accurately as possible. This involves simulating each subsystem (sensing, actuation and communication) of each droplet using accurate physics laws, implementing collision detection and gathering all this data efficiently and in real-time.

	

The need for this simulator is twofold. Firstly, we would like to test the code we write for the hardware in a simulated environment to ensure it does what we think it's going to do before we decide to compile and run it on the actual hardware. Secondly, this gives us the opportunity to run our code on thousands of robots without the need to actually build a thousand robots. The second aspect is very important in the field of swarm robotics as many of the algorithms we deal with show "Emergent" behaviors such as flocking, schooling (for fish) and foraging (in insects) that only become apparent when we have thousands of individual bodies following simple pre-determined rules (the "program"). 