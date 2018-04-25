# -*- coding: utf-8 -*-
from comtypes.client import CreateObject

class RoboRealmInterface:
    def __init__(self, rr_x=1000, rr_y=1000):
        self.rr = CreateObject("RoboRealm.API.1")
        self.rr_x = rr_x
        self.rr_y = rr_y

    def connect(self):
         self.rr.Startup()
        
    def get_robot_positions(self):

        robots_id = [str(name.strip()[1:5]) for name in self.rr.GetArrayVariable("FIDUCIAL_NAME_ARRAY")[0]]
        robots_x = self.rr.GetFloatArrayVariable("FIDUCIAL_X_COORD_ARRAY")[0]
        robots_y = self.rr.GetFloatArrayVariable("FIDUCIAL_Y_COORD_ARRAY")[0]
        robots_orient = self.rr.GetFloatArrayVariable("FIDUCIAL_ORIENTATION_ARRAY")[0]        
        if (len(robots_id) is not len(robots_x)) or (len(robots_id) is not len(robots_y)) or (len(robots_y) is not len(robots_orient)):
            print("ERROR: API data length mismatch: ({0},{1},{2},{3})".format(len(robots_id),len(robots_x),len(robots_y),len(robots_orient)))
            return {}
        res = {}
        for i in range(len(robots_id)):
            res[robots_id[i]] = (robots_x[i], robots_y[i], robots_orient[i])
#        print ("RoboRealm Data")
#        print res
#        print ('')
        return res
        
    def wait_image(self):
        self.rr.WaitImage()
    
    def disconnect(self):
        self.rr.Disconnect()