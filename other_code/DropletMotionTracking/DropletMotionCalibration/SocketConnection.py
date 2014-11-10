import socket
import re
import array

# socket read/write timeout in seconds
TIMEOUT = 30

# the port number to listen on ... needs to match that used in RR interface
SERVER_PORT = 6060

############################# RR API CLASS ##################################

#precompiling all needed regular expressions
VarParReq=re.compile('<response><[^>]+>([^<]*)</[^>]+></response>')
HitWidReq=re.compile('<response><width>([^<]*)</width><height>([^<]*)</height></response>')

class RR_API:
  def __init__(self):
    #create an INET, STREAMing socket
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #only wait TIMEOUT seconds for any request
    self.sock.settimeout(TIMEOUT)

  def escape(self, str):
    str = str.replace("&", "&amp;")
    str = str.replace('"', "&quot;")
    str = str.replace("<", "&lt;")
    str = str.replace(">", "&gt;")
    return str

  def write(self, msg, msgLen):
    totalsent = 0
    # keep sending while there is more data to send
    while totalsent < msgLen:
      sent = self.sock.send(msg[totalsent:msgLen])
      if sent == 0:
        raise RuntimeError, "socket connection broken"
      totalsent = totalsent + sent

  def read(self, msgLen):
    msg = ""
    max = 0
    while (max < msgLen):
      chunk = self.sock.recv(msgLen-max)
      if chunk == '':
          raise RuntimeError, "socket connection broken"
      msg = msg + chunk
      max = max + len(chunk)
    return msg
      
  # Buffered socket image read. Since we don't know how much data was read from a
  # previous socket operation we have to add in any previously read information
  # that may still be in our buffer. We detect the end of XML messages by the
  # </response> tag but this may require reading in part of the image data that
  # follows a message. Thus when reading the image data we have to move previously
  # read data to the front of the buffer and continuing reading in the
  # complete image size from that point. 
  def readMessage(self):
    msg = ""
    while True:
      byte = self.sock.recv(1)
      if byte == '':
        raise RuntimeError, "socket connection broken"
      msg = msg + byte
      if (msg[-11:] == "</response>"):
        return msg

  # Initiates a socket connection to the RoboRealm server
  def Connect(self, hostname):
    self.sock.connect((hostname, SERVER_PORT))

  def GetDimension(self):
    self.sock.send('<request><get_dimension/></request>')
    data = self.readMessage()
    m = HitWidReq.match(data)
    if m:
      width = m.group(1)
      height = m.group(2)
    else:
      width = 0
      height = 0
    return width, height

  # close the socket handle
  def close(self):
    self.sock.close()

  # Returns the current processed image. 
  def GetImage(self, name):
    self.sock.send("<request><get_image>" + name + "</get_image></request>")
    data = self.readMessage()
    m = re.match('<response><length>([^<]*)</length><width>([^<]*)</width><height>([^<]*)</height></response>', data)
    if m:
      len = int(m.group(1))
      width = int(m.group(2))
      height = int(m.group(3))
      newPixels = self.read(len)
    else:
      newPixels = array.array('B', [0 for i in range(1)])
      width = 0
      height = 0
    return newPixels, width, height

  # Sets the current source image. 
  def SetImage(self, name, pixels, width, height):
    self.sock.send("<request><set_image><source>"+name+"</source><width>"+str(width)+"</width><height>"+str(height)+"</height></set_image></request>")
    self.write(pixels, width*height*3)
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0

  # Returns the value of the specified variable.
  def GetVariable(self, name):
    self.sock.send("<request><get_variable>"+str(name)+"</get_variable></request>")
    data = self.readMessage()
    m = VarParReq.match(data)
    if m:
      value = m.group(1)
    else:
      value = ""
    return value

  # Returns all variables in RoboRealm as a dict
  def GetAllVariables(self): 
    self.sock.send("<request><get_all_variables></get_all_variables></request>")
    response = self.readMessage()
    response = re.sub("(<response>)|(</response>)", "", response) 
    values = re.findall("<(?P<VAR>.+)>(.+)</(?P=VAR)>", response) 
    data = dict({}) 
    for key, value in values: 
      data[key] = value 
    return data 

  # Sets the value of the specified variable.
  def SetVariable(self, name, value):
    self.sock.send("<request><set_variable><name>"+self.escape(str(name))+"</name><value>"+self.escape(str(value))+"</value></set_variable></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0

  # Deletes the specified variable
  def DeleteVariable(self, name):
    self.sock.send("<request><delete_variable>"+self.escape(str(name))+"</delete_variable></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0
  
  # Executes the provided image processing pipeline
  def Execute(self, source):
    self.sock.send("<request><execute>"+self.escape(str(source))+"</execute></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0

  # Executes the provided .robo file. Note that the file needs to be on the machine
  # running RoboRealm. This is similar to pressing the 'open program' button in the 
  # main RoboRealm dialog.
  def LoadProgram(self, filename):
    self.sock.send("<request><load_program>"+self.escape(str(filename))+"</load_program></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0

  # Loads an image into RoboRealm. Note that the image needs to exist
  # on the machine running RoboRealm. The image format must be one that
  # RoboRealm using the freeimage.dll component supports. This includes
  # gif, pgm, ppm, jpg, png, bmp, and tiff. This is 
  # similar to pressing the 'load image' button in the main RoboRealm
  # dialog.
  def LoadImage(self, name, filename):
    self.sock.send("<request><load_image><name>"+self.escape(str(name))+"</name><filename>"+self.escape(str(filename))+"</filename></load_image></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0

  # Saves the specified image in RoboRealm to disk. Note that the filename is relative
  # to the machine that is running RoboRealm. The image format must be one that
  # RoboRealm using the freeimage.dll component supports. This includes
  # gif, pgm, ppm, jpg, png, bmp, and tiff. This is 
  # similar to pressing the 'save image' button in the main RoboRealm
  # dialog.
  def SaveImage(self, filename):
    self.sock.send("<request><save_image><filename>"+self.escape(str(filename))+"</filename></save_image></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0

  # Sets the current camera driver. This can be used to change the current viewing camera 
  # to another camera installed on the same machine. Note that this is a small delay
  # when switching between cameras. The specified name needs only to partially match
  # the camera driver name seen in the dropdown picklist in the RoboRealm options dialog.
  # For example, specifying "Logitech" will select any installed Logitech camera including
  # "Logitech QuickCam PTZ".
  def SetCamera(self, name):
    self.sock.send("<request><set_camera>"+self.escape(name)+"</set_camera></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0

  # This routine provides a way to stop processing incoming video. Some image processing
  # tasks can be very CPU intensive and you may only want to enable processing when
  # required but otherwise not process any incoming images to release the CPU for other
  # tasks. The run mode can also be used to processing individual frames or only run
  # the image processing pipeline for a short period. This is similar to pressing the
  # "run" button in the main RoboRealm dialog.
  def Run(self, mode):
    self.sock.send("<request><run>"+self.escape(mode)+"</run></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0

  # There is often a need to pause your own Robot Controller program to wait for
  # RoboRealm to complete its task. The eaisest way to accomplish this is to wait
  # on a specific variable that is set to a specific value by RoboRealm. Using the
  # waitVariable routine you can pause processing and then continue when a variable
  # changes within RoboRealm.
  def WaitVariable(self, name, value, timeout):
    self.sock.send("<request><wait_variable><name>"+self.escape(name)+"</name><value>"+self.escape(value)+"</value><timeout>"+str(timeout)+"</timeout></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0

  # If you are rapdily grabbing images you will need to wait inbetween each
  # get_image for a new image to be grabbed from the video camera. The wait_image
  # request ensures that a new image is available to grab. Without this routine
  # you may be grabbing the same image more than once.
  def WaitImage(self, timeout):
    self.sock.send("<request><wait_image><timeout>"+str(timeout)+"</timeout></wait_image></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0

  # Mainly used for reference or degugging. Prints out all the current variables from
  # RoboRealm as it appears in an XML string.
  def ReadAll(self):
    self.sock.send("<request><get_all_variables></get_all_variables></request>")
    data=self.readMessage()
    print data

  #Gets the Value of the Specified Parameter (as apposed to a variable value)
  def GetParamVal(self, modname, modindex, paramname):
    self.sock.send("<request><set_parameter><module>" + self.escape(str(modname)) + "</module><module_number>" + self.escape(str(modindex)) + "</module_number><name>" + self.escape(str(paramname)) + "</name></set_parameter></request>")
    data=self.readMessage()
    m=VarParReq.match(data)
    if m:
      value=m.group(1)
    else:
      value=""
    return value
    
  #Sets the Value of the Specified Parameter
  def SetParamVal(self,modname,modindex,paramname,paramvalue):
    self.sock.send("<request><set_parameter><module>" + self.escape(str(modname)) + "</module><module_number>" + self.escape(str(modindex)) + "</module_number><name>" + self.escape(str(paramname)) + "</name><value>" + self.escape(str(paramvalue)) + "</value></set_parameter></request>")
    if (self.readMessage() == "<response>ok</response>"):
      return 1
    else:
      return 0 

############################## Test program ##################################

## initialize the API class
#rr = RR_API()
#width = 0
#height = 0

##connect to RoboRealm
#rr.Connect("localhost")

## get and print current image dimension
#width, height = rr.GetDimension()
#print "Width: ",width,"Height: ",height

#rr.SetCamera("off")
#rr.LoadImage("", "c:\\www\\RoboRealm\\bin\\remo.gif")

##get and set current image pixels .. although it is best to use RoboRealm 
## extensions for image processing
#pixels, width, height = rr.GetImage("")
#rr.SetImage("", pixels, 320, 240)

## variable handling
#print "IMAGE_COUNT = "+rr.GetVariable("IMAGE_COUNT")
#rr.SetVariable("test", "this")
#rr.DeleteVariable("my_test")

## execute a red and green RGBFilter in RoboRealm
##rr.Execute("<head><version>1.50</version></head><RGB_Filter><min_value>40</min_value><channel>3</channel></RGB_Filter>")

##rr.LoadProgram("c:\\www\\RoboRealm\\scripts\\red.robo")

##rr.SaveImage("c:\\temp\\remo.jpg")

##rr.SetCamera("on")

##rr.WaitVariable("test", "that", 20000)
##rr.WaitImage(5000)

#rr.close()
