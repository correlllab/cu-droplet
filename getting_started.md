These instructions are specific to the Windows operating system. All hardware referenced is simply the hardware we use. I'm confident that it's possible to get up and running with other operating systems and other hardware, but I haven't tried and so can't comment on doing so.

You will need a PDI programmer. We use the JTAGICE3 http://store.atmel.com/PartDetail.aspx?q=p:10500269

First, install the latest version of Atmel Studio (6.2sp1 at time of writing) http://www.atmel.com/tools/atmelstudio.aspx
This will also install the USB drivers for the JTAGICE3.

Once Atmel Studio is installed, 


The Droplets are capable of serial communication with a computer. We find this primarily useful for debugging (by letting us read print statements) though Atmel Studio allows debugger support as well.

To do this, you'll need an FTDI cable like the one available on sparkfun: https://www.sparkfun.com/products/9718
You'll have to modify the female connector on that cable to reduce it to a 4-wire cable.  **picture**


