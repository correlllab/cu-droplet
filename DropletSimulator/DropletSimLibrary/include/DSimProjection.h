#pragma once

#ifndef _DROPLET_PROJECTOR
#define _DROPLET_PROJECTOR

#include "DSimDataStructs.h"
#include "DSimGlobals.h"


#ifdef _WIN32
#include "inttypes.h"   // inttypes.h is included in UNIX systems
#else 
#include <inttypes.h>
#endif

#include <EasyBMP.h>

#include <vector>
#include <string>
#include <math.h>
#include <iostream>

/**
 * DSimProjection controls the projector inside the arena.
 */

class DSimProjection
{
protected :
	
	int		imgLength,			imgWidth;
	int		projLength,			projWidth;
	float	floorLength,		floorWidth;
	float	projPixelLength,	projPixelWidth;

	std::string fileDir, fileName, fileFormat;	

	/**
	 * True if data is set.
	 */

	bool dataSet;

	/**
	 * Information describing the image.
	 */

	BMP *imgData;

public :

	/**
	 * Constructor.
	 *
	 * \param	projLength	Length of the project.
	 * \param	projWidth 	Width of the project.
	 */

	DSimProjection(int projLength, int projWidth);

	/**
	 * Destructor.
	 */

	~DSimProjection();

	/**
	 * Sets the working directory for bitmap files.
	 *
	 * \param	dirLocation	The dir location.
	 *
	 * \return	.
	 */

	virtual DS_RESULT SetDirectory(std::string dirLocation);

	/**
	 * Loads a bitmap file into the projector.
	 *
	 * \param	fileName	Filename of the file.
	 *
	 * \return	The file.
	 */

	virtual DS_RESULT LoadFile(std::string fileName);

	/**
	 * Gets the RGBA value of a pixel at an xy location.
	 *
	 * \param [in,out]	xyLoc  	If non-null, the xy location.
	 * \param [in,out]	rgbaVal	If non-null, the RGBA value.
	 *
	 * \return	The pixel.
	 */

	virtual DS_RESULT GetPixel(float *xyLoc, uint8_t *rgbaVal);

	/**
	 * Gets the RGBA values of pixels at a specified set of xy locations.
	 *
	 * \param [in,out]	xyLocs  	If non-null, the xy locs.
	 * \param [in,out]	rgbaVals	If non-null, the RGBA vals.
	 *
	 * \return	The pixels.
	 */

	virtual DS_RESULT GetPixels(std::vector<float *> *xyLocs, std::vector<uint8_t *> *rgbaVals);

	/**
	 * Gets the RGBA values of pixels in a specified area.
	 *
	 * \param [in,out]	xyTopLeft	 	If non-null, the xy top left.
	 * \param [in,out]	xyBottomRight	If non-null, the xy bottom right.
	 * \param [in,out]	rgbaVals	 	If non-null, the RGBA vals.
	 *
	 * \return	The pixels.
	 */

	//virtual DS_RESULT GetPixels(float *xyTopLeft, float *xyBottomRight, uint8_t **rgbaVals);
	
	
};


#endif
