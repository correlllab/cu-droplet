/*
 * DSimUtil.h - 
 *
 * Random number generator from Numerical Recipes, 3rd Edition 
 *
 */
#pragma once

#ifndef _DROPLET_UTIL
#define _DROPLET_UTIL

#ifdef _WIN32
#include "inttypes.h"   // inttypes.h is included in UNIX systems
#else 
#include <inttypes.h>
#endif

#include <cmath>

#include "DSimDataStructs.h"

struct Ran {

	uint64_t u, v, w;

	// Constructor. Call with any integer seed (except value of v below)
	Ran(uint64_t j) : v(4101842887655102017LL), w(1) {
		u = j ^ v; int64();
		v = u; int64();
		w = v; int64();
	}
	
	// Return 64-bit random integer
	inline uint64_t int64() {
		u = u * 2862933555777941757LL + 7046029254386353087LL;
		v ^= v >> 17; v ^= v << 31; v ^= v >> 8;
		w = 4294957665U * (w & 0xffffffff) + (w >> 32);
		uint64_t x = u ^ (u << 21); x ^= x >> 35; x ^= x << 4;
		return (x + v) ^ w;
	}
	
	// Return random double-precision floating value in the range 0. to 1.
	inline double doub() { return 5.42101086242752217E-20 * int64(); }
};	

class TrigArray
{
private :
	typedef struct _Relative_Position_Data
	{
		float xDist, yDist, zDist;
	} RelPosData;

	unsigned int n, l;
	RelPosData **data;

	unsigned int computeIndex(unsigned int d1, unsigned int d2);

public :
	
	TrigArray(unsigned int numDroplets);
	~TrigArray(void);
	
	// Data is always signed relative to the lower droplet id (by convention)
	DS_RESULT AddData(unsigned int d1, unsigned int d2, float xDist, float yDist, float zDist);
	
	DS_RESULT RemoveData(unsigned int d1);

	DS_RESULT GetData(unsigned int d1, unsigned int d2, float *dist, float *angle);
	
	float GetDistance(unsigned int d1, unsigned int d2);
	
	float GetAngle(unsigned int d1, unsigned int d2);
};

#endif
