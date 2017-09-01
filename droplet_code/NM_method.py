"""
Simulate how Droplets will perform with Nelder-Mead Method

Author: Yang Li
Date: August 29, 2017

# randomly generate N droplets with diameter = 450mm within a square
	of length 450mm


"""

import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import minimize
from numpy.random import rand
import random
import math
import time
from math import sqrt

# draw ellipse
from matplotlib.patches import Ellipse

LoG_SIGMA = 0.5

def LoGx(x,y):
	scalarTerm = 1.0/(2*math.pi*(LoG_SIGMA**6))
	x_y = x**2 + y**2
	res = math.exp( -x_y/(2*(LoG_SIGMA**2)) ) * (x**2 - LoG_SIGMA**2) * scalarTerm
	return res


class NelderMeadMethod:
	def __init__(self, lims, num, width):
		self.lims = lims
		self.num = num
		self.width = width
		self.xs = []
		self.ys = []
		self.colors = []
		self.results = []

		self.NM_ALPHA = 1.0
		self.NM_GAMMA = 2.0
		self.NM_RHO = 0.5
		self.NM_SIGMA = 0.5

	def testLoG(self, i_me, p):
		theta = math.atan2(p[1], p[0])
		cosTheta = math.cos(theta)
		sinTheta = math.sin(theta)
		val = 0
		for i in range(self.num):
			xDiff = self.xs[i]-self.xs[i_me]
			yDiff = self.ys[i]-self.ys[i_me]
			x = xDiff*cosTheta - yDiff*sinTheta
			y = xDiff*sinTheta + yDiff*cosTheta
			x /= (self.width)
			y /= (self.width)
			val += self.colors[i] * LoGx(x,y)
		p[-1] = abs(val)

	def NMStep_min(self, i_me, spx):
		# sort in descending order on spx[-1]
		spx.sort(key=lambda x: x[-1], reverse=False)

		x = (spx[0][0]+spx[1][0])/2
		y = (spx[0][1]+spx[1][1])/2
		xo = [x, y, 0]
		self.testLoG(i_me, xo)

		x = xo[0]+self.NM_ALPHA*(xo[0]-spx[-1][0])
		y = xo[1]+self.NM_ALPHA*(xo[1]-spx[-1][1])
		xr = [x, y, 0]
		self.testLoG(i_me, xr)

		if(xr[-1] >= spx[0][-1] and xr[-1] < spx[-2][-1]):
			tmp = sqrt((xr[0]-spx[-1][0])**2 + (xr[1]-spx[-1][1])**2)
			spx[-1] = xr
			return tmp
		elif xr[-1] < spx[0][-1]:
			x = xo[0]+self.NM_GAMMA*(xr[0]-xo[0])
			y = xo[1]+self.NM_GAMMA*(xr[1]-xo[1])
			xe = [x, y, 0]
			self.testLoG(i_me, xe)

			if(xe[-1] < xr[-1]):
				tmp = sqrt((xe[0]-spx[-1][0])**2 + (xe[1]-spx[-1][1])**2)
				spx[-1] = xe
				return tmp
			else:
				tmp = sqrt((xr[0]-spx[-1][0])**2 + (xr[1]-spx[-1][1])**2)
				spx[-1] = xr
				return tmp
		else:
			x = xo[0]+self.NM_RHO*(spx[-1][0]-xo[0])
			y = xo[1]+self.NM_RHO*(spx[-1][1]-xo[1])
			xc = [x, y, 0]
			self.testLoG(i_me, xc)

			if(xc[-1] < spx[-1][-1]):
				tmp = sqrt((xc[0]-spx[-1][0])**2 + (xc[1]-spx[-1][1])**2)
				spx[-1] = xc
				return tmp
			else:
				spx[1][0] = spx[0][0] + self.NM_SIGMA*(spx[1][0] - spx[0][0])
				spx[1][1] = spx[0][1] + self.NM_SIGMA*(spx[1][1] - spx[0][1])
				spx[2][0] = spx[0][0] + self.NM_SIGMA*(spx[2][0] - spx[0][0])
				spx[2][1] = spx[0][1] + self.NM_SIGMA*(spx[2][1] - spx[0][1])	
				return 5

	def decidePattern(self, i_me): #Using optimal rotation.
		spx=[]
		spx.append([math.cos(math.radians(15)),math.sin(math.radians(15)),0])
		self.testLoG(i_me, spx[0])
		spx.append([math.cos(math.radians(135)),math.sin(math.radians(135)),0])
		self.testLoG(i_me, spx[1])
		spx.append([math.cos(math.radians(-105)),math.sin(math.radians(-105)),0])
		self.testLoG(i_me, spx[2])

		# print("\n[{: 0.3f}, {: 0.3f}, {: 0.3f}], [{: 0.3f}, {: 0.3f}, {: 0.3f}], [{: 0.3f}, {: 0.3f}, {: 0.3f}]".format(spx[0][0],spx[0][1],spx[0][2],spx[1][0],spx[1][1],spx[1][2],spx[2][0],spx[2][1],spx[2][2]))
		
		result = 1e8
		c = 0
		while result>0.1 and c<50:
			result = self.NMStep_min(i_me, spx)
			c += 1
		# print("[{: 0.3f}, {: 0.3f}, {: 0.3f}], [{: 0.3f}, {: 0.3f}, {: 0.3f}], [{: 0.3f}, {: 0.3f}, {: 0.3f}]".format(spx[0][0],spx[0][1],spx[0][2],spx[1][0],spx[1][1],spx[1][2],spx[2][0],spx[2][1],spx[2][2]))
		# print(c)
		if spx[0][1]<0:
			spx[0][0] = - spx[0][0]
			spx[0][1] = - spx[0][1]		
		theta = -math.atan2(spx[0][1], spx[0][0])

		ori = [math.cos(theta), math.sin(theta)]

		spx[0][0] = math.cos(2*theta)
		spx[0][1] = math.sin(2*theta)
		spx[0][-1] = math.degrees(theta)

		return spx[0], ori

	def amorphousCoordinates(self):
		self.xs = []
		self.ys = []
		# randomly generate a set of coordination
		# make sure there is no duplicated ones
		dd = 1
		while len(self.xs) < self.num:
			x = self.lims[0]*rand()
			y = self.lims[1]*rand()

			# Constraint: distance betweeen two Droplets is larger than 4.5 + dd (by default dd=1.5)
			isFeasible = True
			for di in range(len(self.xs)):
				if (x-self.xs[di])**2 + (y-self.ys[di])**2 < (44+dd)**2:
					isFeasible = False
					break
			if isFeasible:
				self.xs.append(x)
				self.ys.append(y)

	def gridCoordinates(self):
		self.xs = []
		self.ys = []
		n = sqrt(self.num)
		s1 = self.lims[0]/n
		s2 = self.lims[1]/n
		offset = 3
		for x in np.arange(offset, self.lims[0]+offset, s1):
			for y in np.arange(offset, self.lims[1]+offset, s2):
				self.xs.append(x)
				self.ys.append(y)

	def assignColors(self):
		self.colors = []
		SPOTS = False
		patternAngle = 45
		rotationAngle = math.radians(90-patternAngle)
		# assign colors
		for i in range(self.num):
			rotX = self.xs[i]*math.cos(rotationAngle)-self.ys[i]*math.sin(rotationAngle)
			rotY = self.xs[i]*math.sin(rotationAngle)+self.ys[i]*math.cos(rotationAngle)
			# the check below goes wrong for negative numbers, so we
			# add an offset to ensure that rotY is positive.
			rotX += 2*max(self.lims)
			rotY += 2*max(self.lims)
			if SPOTS:
				if int(rotX/self.width)&0x1 and int(rotY/self.width)&0x1:
					self.colors.append(1)
				else:
					self.colors.append(0)
			else:
				if int(rotX/self.width)&0x1 :
					self.colors.append(1)
				else:
					self.colors.append(0)		

	def run(self):
		self.results = []
		self.resultsOri = []
		for i in range(self.num):
			result, ori = self.decidePattern(i)
			self.results.append(result)
			self.resultsOri.append(ori)

	def drawPattern_Grid(self, path_to, name):
		fig, ax = plt.subplots()	

		# draw Droplets
		for i in range(self.num):
			droplet = Ellipse(xy=(self.xs[i], self.ys[i]), width=44, height=44, linewidth=0.0)
			ax.add_artist(droplet)
			droplet.set_clip_box(ax.bbox)
			
			if self.colors[i]:
				droplet.set_alpha(0.8)
				rgb = [ 1,  0,  0]
			else:
				droplet.set_alpha(0.3)
				rgb = [ 0,  0,  1]
			droplet.set_facecolor(rgb)

		ax.set_xlim((0,self.lims[0]))
		ax.set_ylim((0,self.lims[1]))
		x0,x1 = ax.get_xlim()
		y0,y1 = ax.get_ylim()
		ax.set_aspect(abs(x1-x0)/abs(y1-y0))
		# ax.grid(b=True, which='major', color='k', linestyle='--')

		fname = "{}/{}.png".format(path_to, name)
		fig.savefig(fname, bbox_inches='tight') # , dpi=fig.dpi

	def drawPatternWithOrientation(self, path_to, name):
		fig, ax = plt.subplots()

		# draw Droplets
		for i in range(self.num):
			theta = math.degrees(math.atan2(self.results[i][1], self.results[i][0])/2)
			droplet = Ellipse(xy=(self.xs[i], self.ys[i]), width=50.0, height=25.0, angle=theta, linewidth=0.0)
			ax.add_artist(droplet)
			droplet.set_clip_box(ax.bbox)
			
			if self.colors[i]:
				droplet.set_alpha(0.8)
				rgb = [ 1,  0,  0]
			else:
				droplet.set_alpha(0.3)
				rgb = [ 0,  0,  1]
			droplet.set_facecolor(rgb)

		ax.set_xlim((0,self.lims[0]))
		ax.set_ylim((0,self.lims[1]))
		x0,x1 = ax.get_xlim()
		y0,y1 = ax.get_ylim()
		ax.set_aspect(abs(x1-x0)/abs(y1-y0))
		# ax.grid(b=True, which='major', color='k', linestyle='--')

		fname = "{}/{}_orientation.png".format(path_to, name)
		fig.savefig(fname, bbox_inches='tight') # , dpi=fig.dpi

# 'factor' value kind of means how much strength to push each activator againt each other
def test_NelderMead(path_to, name):
	# length unit here is millimeter
	lims = [450, 450]
	num = 8**2
	width = 90
	results = []

	nm = NelderMeadMethod(lims, num, width)
	# nm.gridCoordinates()
	nm.amorphousCoordinates()
	nm.assignColors()
	nm.drawPattern_Grid(path_to, name)
	print ("--> Coordination and Colors ready to go!")

	nm.run()
	nm.drawPatternWithOrientation(path_to, name)

	print("\n\nResults\n")
	for res in nm.results:
		print("{: 0.3f}, {: 0.3f}, {: 4d}, {: 4d}".format(res[0], res[1], (int)(math.degrees(math.atan2(res[1],res[0]))/2), int(res[-1]/2)))

	s = [sum(i)/num for i in zip(*nm.results)]
	print("\nSum")
	print("{: 0.3f}, {: 0.3f}, {: 4d}, {: 4d}".format(s[0], s[1], int(math.degrees(math.atan2(s[1],s[0]))/2), int(s[-1]/2)))
	print("Variance: {}".format(1-math.hypot(s[0],s[1])))


if __name__ == "__main__":

	path_to = "."
	name = 'amorphous_450x450_run_1'
	t0 = time.time()
	test_NelderMead(path_to, name)
	print ("Time used: ", time.time()-t0)
