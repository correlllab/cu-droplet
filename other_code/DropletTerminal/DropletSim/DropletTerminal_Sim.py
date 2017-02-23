from random import randint
from random import normalvariate as randnormal
import math, cmd

################################HELPER FUNCTIONS###############################
"""Simple function, ensures an angle is between -180 and 180"""
def prettyAngleDeg(theta):
    return ((theta+180)%360) - 180

def getDist(d1, d2):
    return math.hypot(d2.x-d1.x,d2.y-d1.y)

def checkIntersecting(newBot, bots):
    for bot in bots:
        if getDist(bot, newBot)<(bot.radius+newBot.radius):
            return True
    return False

def generateBots(quantity, region):
    """
    Use caution!
    If the region isn't big enough for the quantity, this will run forever.
    If the region is almost not big enough for the quantity,
    this will run very slowly, and sometimes forever.
    """
    bots = []
    for i in range(quantity):
        newBot = Droplet(randint(region[0][0],region[0][1]),
                         randint(region[1][0],region[1][1]),
                         randint(-180,180))
        while(checkIntersecting(newBot, bots)):
            newBot = Droplet(randint(region[0][0],region[0][1]),
                             randint(region[1][0],region[1][1]),
                             randint(-180,180))
        bots.append(newBot)
    return bots


####################################CLASSES####################################
###############################################################################
class RnbMeas:
    def __init__(self, r, b, h, rxID, txID):
        self.r = r
        self.b = b
        self.h = h
        self.rxID = rxID
        self.txID = txID
        
    """
    Standard Deviation Values below are made up, and probably on
    the small side of what we see in hardware for the Droplets.
    The below also assumes errors are uncorrelated, which is false
    on the real hardware. I'm sure numpy has the multivariate gaussian
    support needed for better noise modeling.
    """
    def addNoise(self):
        self.r = int(round(randnormal(self.r,22)))
        self.b = prettyAngleDeg(int(round(randnormal(self.b,10))))
        self.h = prettyAngleDeg(int(round(randnormal(self.h,20))))
    
    def __repr__(self):
        return "{{{}, {}, {{{}, {}, {}}}}}".format(self.rxID, self.txID,
                                                   self.r, self.b, self.h)

###############################################################################
class DropletWorld:
    def __init__(self, xmin = -500, xmax = 500, ymin = -500, ymax = 500):
        self.x0 = xmin
        self.x1 = xmax
        self.y0 = ymin
        self.y1 = ymax

    bots = []
    numBots = len(bots)

###############################################################################
class Droplet:
    
    radius = 22
    #15.1 from center to foot
    
    def __init__(self, x, y, o, id=None):
        self.x = x
        self.y = y
        self.o = o
        if id is None:
            self.id = "{:04X}".format(randint(0,65535))
        else:
            self.id = id
    
    def move(self, dir, dist):
        assert(dir in range(0,7))

        dirs = {0 : 0, 1 : 180/3, 2 : 360/3, 3 : 180,
                4 : -360/3, 5 : -180/3, 6 : 1, 7 : -1}

        if dir <= 5:
            if abs(dist*math.cos(math.radians(self.o + dirs[dir]+90))) < .001:
                self.x += 0

            if abs(dist*math.cos(math.radians(self.o + dirs[dir]+90))) >= .001:
                self.x += dist*math.cos(math.radians(self.o + dirs[dir] + 90))

            if abs(dist*math.sin(math.radians(self.o + dirs[dir]+90))) < .001:
                self.y += 0

            if abs(dist*math.sin(math.radians(self.o + dirs[dir]+90))) >= .001:
                self.y += dist*math.sin(math.radians(self.o + dirs[dir] + 90))

        else:
            self.o += dist*dirs[dir]


    def rnbMeas(self, bot):
        """Generates a simulated measurement of bot by self."""
        xDiff = bot.x-self.x
        yDiff = bot.y-self.y
        oDiff = bot.o-self.o
        range = math.hypot(xDiff, yDiff)
        bearing = prettyAngleDeg(math.degrees(math.atan2(yDiff, xDiff))
                                 -self.o-90)
        heading = prettyAngleDeg(oDiff)
        meas = RnbMeas(int(round(range)), int(round(bearing)),
                       int(round(heading)), self.id, bot.id)
        #Comment out the below to turn off measurement noise (ie, for testing).
        meas.addNoise()
        return meas
    
    def __repr__(self):
        return "{{{}, {{{}, {}, {}}}}}".format(self.id, self.x,
                                               self.y, self.o)


###############################################################################
class DropletSim(cmd.Cmd):
    intro = 'Type help or ? to list commands.'
    prompt = '>'

    def do_Printworld(self, s):
        'Return the dimensions of Droplet World.\n\
        Command format: No Arguments'
        print("xmin: " + str(world.x0), "xmax: " + str(world.x1),
              "ymin: " + str(world.y0), "ymax: " + str(world.y1))

    def do_Genbots(self, s):
        'Generate n number of Droplets in Droplet World.\n\
        Command format: n'
        bot_count = int(s.split()[0])
        world_dims = ((world.x0, world.x1), (world.y0, world.y1))
        world.bots.append(generateBots(bot_count, world_dims))

    def do_Allbots(self, s):
        'Return the status of all Droplets.\n\
        Command format: No Arguments'
        print(world.bots)

    def do_Newbot(self, s):
        'Generate new Droplet at location x,y with orientation o \
        (0 = 12 oclock).\n\
        Command format: x y o'
        x = float(s.split()[0])
        y = float(s.split()[1])
        o = float(s.split()[2])

        newBot = Droplet(x, y, o)
        if (checkIntersecting(newBot, world.bots)
            or (newBot.x not in range(world.x0, world.x1))
            or (newBot.y not in range(world.y0, world.y1))):
            print('Collision, place bot at another location')
            return
        else:
            world.bots.append(newBot)
            print world.bots

    def do_move(self, s):
        'Move Droplet bot Direction dir a Distance dist.\
        \nCommand format: bot dir dist'
        bot = int(s.split()[0])
        dir = int(s.split()[1])
        dist = float(s.split()[2])
        world.bots[bot].move(dir, dist)
        print world.bots[bot]

    def do_get_rnb_data(self, s):
        'Get rnb data for bot a with respect to bot b.\
        \nCommand format: a b'


##################################MAIN PROGRAM#################################
if __name__ == '__main__':
    world = DropletWorld()
    DropletSim().cmdloop()
