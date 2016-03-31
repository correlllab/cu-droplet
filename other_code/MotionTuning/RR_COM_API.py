# DON'T FORGET TO 
# regsvr32 "c:\Program Files (x86)\RoboRealm\RR_COM_API.dll"
# and/or
# regsvr32 "c:\Program Files (x86)\RoboRealm\RR_COM_API_64.dll"
# before running!!
# You will also need to install comtypes and pillow (Image)
# c:\python34\python -m pip install comtypes
# c:\python34\python -m pip install pillow

from PIL import Image
import comtypes
import comtypes.client as cc
from array import array
import io

rr = cc.CreateObject("RoboRealm.API.1")

rr.Startup()

print(rr.GetVersion())

rr.SetVariable("var_from_api", "hello world")

(pixels, width, height, res) = rr.GetImage("processed", "RGB")

pixels = bytes(pixels)

print("Width:",width," Height: ", height)

img = Image.frombuffer ('RGB', (width, height), pixels, 'raw', 'RGB', 0, 1)
                           
img.show()

wait = input("PRESS ENTER TO CONTINUE.")
