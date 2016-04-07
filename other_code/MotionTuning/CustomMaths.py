#
# FROM : http://wiki.scipy.org/Cookbook/Least_Squares_Circle
#
#  == METHOD 2 ==
import numpy as np
from scipy import optimize

def calc_R(cX, cY, x, y):
    """ calculate the distance of each 2D points from the center (xc, yc) """
    return np.sqrt((x-cX)**2 + (y-cY)**2)

def f_2(x, y):
    """ calculate the algebraic distance between the data points and the mean circle centered at c=(xc, yc) """
    def f2_helper(c):
        Ri = calc_R(c[0],c[1], x, y)
        return Ri - Ri.mean()
    return f2_helper;

def get_radial_velocity(orient_list):
    orient_grad = np.gradient(orient_list)
    orient_grad[np.abs(orient_grad)>90]=0
    return np.mean(orient_grad)

def get_sign((x, y, orient), center):
    pos = (x,y)
    dirVector = np.array([np.cos(np.deg2rad(orient)),np.sign(np.deg2rad(orient))])
    return np.linalg.det(np.array([dirVector, center-pos]).transpose())

def pretty_angle(theta):
    return ((theta+180)%360)-180
        
def lsq_circle_fit(pose_list):
    (x, y, th) = pose_list.transpose()
    x_m = x.mean()
    y_m = y.mean()
    center_estimate = x_m, y_m
    calc_center, ier = optimize.leastsq(f_2(x,y), center_estimate)

    ccX, ccY = calc_center
    calc_Ri       = calc_R(ccX, ccY, x, y)
    radius        = calc_Ri.mean()
    residual   = np.sum((calc_Ri - radius)**2)

    center=np.array(calc_center)
    
    sign = get_sign(pose_list[-1], center)
    
    #print("center: (%f,%f), rad: %f, residu: %f, sign: %f"%(ccX,ccY,radius,residual, sign))
    return (center, radius, residual, sign)