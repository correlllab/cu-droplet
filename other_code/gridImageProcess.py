import numpy as np
import cv2
import time

dropletTrueCoord = [[0, 0], [60, 0], [120, 0], [180, 0], [300, 0], [360, 0], [420,   0], [480, 0], [30, 52], [90, 52], [150, 52], [210, 52], [270,   52], [330, 52], [390, 52], [450, 52], [60, 104], [120, 104], [180,   104], [240, 104], [300, 104], [360, 104], [420, 104], [30,   156], [90, 156], [150, 156], [210, 156], [270, 156], [330,   156], [390, 156], [450, 156], [60, 208], [120, 208], [180,   208], [240, 208], [300, 208], [360, 208], [420, 208], [30,   260], [90, 260], [150, 260], [210, 260], [270, 260], [330,   260], [390, 260], [450, 260], [60, 312], [120, 312], [180,   312], [240, 312], [300, 312], [360, 312], [420, 312], [30,   364], [90, 364], [150, 364], [270, 364], [330, 364], [390,   364], [450, 364], [60, 416], [120, 416], [180, 416], [240,   416], [300, 416], [360, 416], [420, 416], [30, 468], [90,   468], [150, 468], [210, 468], [270, 468], [330, 468], [390,   468], [450, 468], [0, 520], [60, 520], [120, 520], [180, 520], [240,   520], [300, 520], [360, 520], [420, 520], [480, 520]]

dropletPixLoc = [[229, 750], [288, 748], [346, 746], [409, 745], [527, 743], [590, 746], [650, 749], [710, 748], [255, 698], [316, 698],
[378, 697], [440, 697], [496, 695], [560, 696], [621, 698], [683, 697], [284, 646], [348, 645], [408, 645], [470, 645],
[530, 643], [589, 644], [652, 647], [253, 594], [316, 595], [376, 593], [436, 597], [500, 594], [560, 595], [621, 596],
[683, 597], [287, 543], [348, 541], [408, 545], [470, 545], [530, 545], [590, 543], [650, 543], [253, 494], [316, 494],
[379, 491], [439, 491], [500, 492], [557, 491], [615, 490], [682, 490], [287, 443], [348, 444], [411, 440], [469, 439],
[526, 440], [589, 438], [648, 440], [253, 391], [318, 390], [378, 391], [501, 388], [557, 386], [617, 388], [683, 385],
[288, 339], [350, 340], [414, 337], [474, 338], [532, 339], [590, 336], [650, 339], [256, 290], [318, 290], [381, 290],
[443, 288], [503, 283], [562, 285], [621, 286], [683, 285], [225, 244], [288, 240], [348, 236], [412, 238], [476, 236], [533, 236], [592, 237], [653, 238], [714, 235]]


def printCoords(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN:
        print("[{0}, {1}]".format(x,y))

def getColorAvgs(img):
    i=0
    for loc in dropletPixLoc:
        roi = tfmd[loc[0]-14:loc[0]+14,loc[1]-14:loc[1]+14]
        r = int(np.average(roi[:,:,0]))
        g = int(np.average(roi[:,:,1]))
        b = int(np.average(roi[:,:,2]))
        print("{0}: {1}, {2}, {3}".format(dropletTrueCoord[i], r, g, b))
        i+=1     

def printColorVals(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN:
        timestr = time.strftime("%Y%m%d-%H%M%S.jpg")
        cv2.imwrite(timestr, tfmd)
        getColorAvgs(tfmd)




#cap = cv2.VideoCapture(0)
cap.set(3,1920)
cap.set(4,1080)
cv2.namedWindow('frame')
cv2.setMouseCallback('frame', printColorVals)

source = np.array([[681.5, 4.5], [1620, 12], [1751.5, 1044.5], [556.5, 997.5]], dtype="float32")
dest = np.array([ [0, 0], [999, 0], [999,999], [0,999]], dtype="float32")
perpTfm = cv2.getPerspectiveTransform(source, dest)

try:
    while(True):
        ret, frame = cap.read()
        tfmd = cv2.warpPerspective(frame, perpTfm, (1000,1000))
        circled = cv2.copyMakeBorder(tfmd,0,0,0,0,cv2.BORDER_REPLICATE)
        for loc in dropletPixLoc:
            cv2.circle(circled,tuple(loc),19,(0,0,0),1,cv2.LINE_AA)
            cv2.circle(circled,tuple(loc),20,(0,255,0),1,cv2.LINE_AA)
            cv2.circle(circled,tuple(loc),21,(0,0,0),1,cv2.LINE_AA)
           # cv2.putText(tfmd,'{0}'.format(i), (loc[0]-12,loc[1]+6),cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,0,0),1,cv2.LINE_AA)
           # i+=1        
    
        cv2.imshow('frame',circled)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        elif cv2.waitKey(1) & 0xFF == ord(' '):
            printColorVals()
except Exception:
    cap.release()
    cv2.destroyAllWindows()
    raise
  
    
cap.release()
cv2.destroyAllWindows()