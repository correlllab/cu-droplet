cu-droplet
==========

Droplets: The Correll Lab Swarm Robotics Platform

<blink>Please help us to make a 1000 droplets by supporting and sharing our <a href="http://www.colorado.edu/crowdfunding/?cfpage=project&project_id=10341">crowdfunding campaign</a>.</blink>

The platform is envisioned to be a scalable robotics experimentation environment. It consists of three major components, the hardware, the embedded software and the software simulation environment.

<table><tr>
<td><img border="0" src="https://lh6.googleusercontent.com/-US0UXltts_E/UQr8mrHlRII/AAAAAAAAHQQ/U2wi6PS017g/s400/newdrop.png" alt="Droplet Hardware 1" width="350" height="228"></td>
<td><img border="0" src="https://lh4.googleusercontent.com/-tKwPBAi_aYM/UQr8m2sCzGI/AAAAAAAAHQM/_FXSDIj36Vg/s400/IMAG0074.jpg" alt="Droplet Hardware 2" width="350" height="228"></td>
</tr></table>

The hardware component comprises of the actual robot and the test bed/floor that experiments are run on. The robots are Ping-Pong ball sized devices capable of sensing (RGB color and IR sensing), actuation using vibration motors and communication, using analog/digital IR sensors. The embedded software layer is what we use to write and compile on-board code for the droplets. Each robot is equipped with an Atmel xmega-128a3u microprocessor capable of running embedded C-code. For more information on the hardware and low-level software API I would recommend reading complete write-up.

<table><tr>
<td><img border="0" src="https://lh6.googleusercontent.com/-_bcse-VY5O0/UQr8mkPtBcI/AAAAAAAAHQI/yZwE15DyUxU/s400/newsim.png" alt="Droplet simulator 1" width="350" height="228"></td>
<td><img border="0" src="https://lh5.googleusercontent.com/-JXt1AaZOuDo/UQr8nEpOxVI/AAAAAAAAHQc/m6XyH4FgdKY/s400/newsim2.png" alt="Droplet simulator 2" width="350" height="228"></td>
</tr></table>

The software simulation environment consists of a simulator for the entire platform that allows you to run tests using droplet code on your PC. The idea here is to simulate the above two layers, using C/C++, as accurately as possible. This involves simulating each subsystem (sensing, actuation and communication) of each droplet using accurate physics laws, implementing collision detection and gathering all this data efficiently and in real-time.

The need for this simulator is twofold. Firstly, we would like to test the code we write for the hardware in a simulated environment to ensure it does what we think it's going to do before we decide to compile and run it on the actual hardware. Secondly, this gives us the opportunity to run our code on thousands of robots without the need to actually build a thousand robots. The second aspect is very important in the field of swarm robotics as many of the algorithms we deal with show "Emergent" behaviors such as flocking, schooling (for fish) and foraging (in insects) that only become apparent when we have thousands of individual bodies following simple pre-determined rules (the "program").

This work is available under a <a href="https://creativecommons.org/licenses/by-nc-sa/4.0/" >Creative Commons Attribution-NonCommercial-ShareAlike License. <img src="http://i.creativecommons.org/l/by-nc-sa/3.0/88x31.png" alt="CC License"></a>
