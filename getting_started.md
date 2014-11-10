NOTE: These instructions are specific to the Windows operating system. All hardware referenced is simply the hardware we use. I believe that it would be possible to use other operating systems and other hardware, but I haven't tried and so can't comment on doing so.

##### Physical Supplies
* A PDI programmer. We use the <a href="http://store.atmel.com/PartDetail.aspx?q=p:10500269">JTAGICE3</a>.
* (Optional, for serial communication) FTDI Cable. We use those available on <a href="https://www.sparkfun.com/products/9718">sparkfun</a> with a minor modication (described below).

##### Software Supplies
* The latest version of <a href="http://www.atmel.com/tools/atmelstudio.aspx">Atmel Studio</a> (6.2sp1 at time of writing).
  * This should also install the USB drivers for the PDI programmer, which are needed.
* This repo! Clone a copy to your local machine.
* (Optional, for serial communication) <a href="http://ttssh2.sourceforge.jp/index.html.en">Tera Term</a>, or any other serial program.
* (Optional, for serial communication) FTDI Drivers for the cable. The sparkfun product page linked above has links for these.

##### Setup

1. Once Atmel Studio is installed, plug your JTAGICE3 programmer in to the computer (since the drivers were just installed, it should be recognized), then open Atmel Studio. It should open to a 'start page'. Near the top-left of this page is a 'New Project...' button. Click that, which should open a dialog. 

2. In this dialog, you should name your project and select a location for it to be stored. The middle of the dialog should have a list of project types. Choose 'GCC C Executable Project'. The bottom of this dialog lets you choose a project name and location on disk. Whatever you prefer is fine. and click 'OK'. This should open a 'Device Selection' dialog. You want to select the device with name 'ATxmega128A3U'. Now, your peojct should be created. 

4. Atmel Studio wiill start your project off with a <Project Name>.c file. Go ahead and delete it.

5. Next, we need to set up the project with the appropriate file structure.
  
  1. In the 'Solution Explorer' sidebar, right-click on '<Project Name>->Add->New Folder', and make a folder called 'src'.
  2. Repeat the above to make another folder called 'include'.
  3. In the same sidebar, right-click on the src folder you just created and go to 'Add->Existing Item...'. This will open up a file browser, which you should use to navigate to the src folder in the repository. Specifically, 'cu-droplet/droplet_code/src/'.
  4. *Important* Select (highlight) all files in this folder, and then click the little down-arrow on the right side of the 'Add' button, which should open a drop-down menu. In this menu, select 'Add as Link'. The reason for this is that if you just clicked 'Add', the files would be copied to the location of the project folder, and any changes you made would not be tracked by git. 
  5. Make sure that the 'src' folder in the Solution Explorer has been populated with c and s files, and that each file icon has a little 'shortcut' arrow in the bottom-right, which indicates that the file has been correctly added as a link.
  6. Repeat steps 3-5 for the 'include' folder: right-click on the folder in the solution explorer of Atmel Studio, select 'Add->Existing Item...', and add all files in 'cu-droplet/droplet_code/include/', making sure again to add them as links.
  7. Finally, repeat steps 3-5 one more time, right-clicking on '<Project Name>' in the solution explorer to add 'cu-droplet/droplet_code/user_template.*' to your project as links. These are the files you will modify for your code.
6. Next, we need to set up the project settings appropriately.

  1. Right-click on '<Project Name>' again, and select 'Properties', which opens the main dialog for editing settings. There are a lot of things you could edit in this dialog, but this document will only tell you what needs to be changed from the defaults that should have been set if you started the project correctly.
  2. On the left, click the 'Toolchain' tab.
    1. In 'AVR/GNU C Compiler->Directories', click the little button with a piece of paper and a green plus sign, then add then navigate to wherever you saved the repo on your machine, and add the repos' include folder ('cu-droplet/droplet_code/include/') to your include path. Then, add 'cu-droplet/droplet_code/' in the same way, so that any '*.h' files you create are included as well. (It doesn't matter if the path is relative or not.)
    2. In 'AVR/GNU Linker->General', check 'Use vprintf library'.
    3. In 'AVR/GNU Linker->Memory Settings', Click the little paper-with-green-plus-sign button for the 'FLASH segment'. Enter '.BOOT = 0x010000' and hit 'OK'.
    4. In 'AVR/GNU Linker->Miscellaneous', in 'Other Linker Flags:' add '-lprintf_flt'
  3. On the left, click the 'Tool' tab.
    1. Use the drop-down menu under 'Selected debuger/programmer' and choose the option which starts 'JTAGICE3' (note: this option will only show up if the JTAGICE3 programmer is actually plugged in to and recognized by your machine).
    2. In the 'Interface:' drop-down menu, select 'PDI'. 
    3. Make sure that PDI Clock is set to 4MHz and, under 'Programming settings', the 'Preserve EEPROM' box is checked.

Okay! You're done! You should now be able to compile code and upload it to your Droplet. Good luck!

#### TODO: Programming Connector
#### TODO: Talk about fuse settings for first-time droplets?

##### Programming

The 'user_template.c' and 'user_template.h' files have comments to explain the functionality of the three key functions: 'init()', 'loop()', and 'handle_msg(msg_struct* msg)'. Feel free to delete these two files and replace them with your own. As long as you define those three functions somewhere in your code the linker should be able to sort it out.

The /src/ and /include/ folders contain all of the hardware code. In theory, you shouldn't need to modify these files, but if you feel need to, be our guests!

##### Serial Communication

The Droplets are capable of serial communication with a computer. Primarily, we use this to view print statements from the Droplets, and occasionally to override their code for debugging purposes.

### TODO: Put in instructions on how to modify the FTDI cable for Droplets.
