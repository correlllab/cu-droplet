#include "DSimProjection.h"

DSimProjection::DSimProjection(int projLength, int projWidth)
{
	this->projLength = projLength;
	this->projWidth  = projWidth;

	// TODO : For now floor projLength and projwidth is the same as floorLength and floorWidth
	this->floorLength = (float)projLength;
	this->floorWidth  = (float)projWidth;

	dataSet = false;
	imgData = NULL;
	fileDir.clear();
	fileName.clear();
	fileFormat.clear();
}

DSimProjection::~DSimProjection()
{
	SAFE_DELETE(imgData);
}

DS_RESULT DSimProjection::SetDirectory(std::string dirLocation)
{
	// TODO : Check if directory exists
	fileDir = dirLocation;

	return DS_SUCCESS;
}

DS_RESULT DSimProjection::LoadFile(std::string fileName)
{
	this->fileName = fileName;
	std::string filePath = fileDir;
	filePath.append(fileName);

	SAFE_DELETE(imgData);

	imgData = new BMP();
	if(!imgData->ReadFromFile(filePath.c_str()))
	{
		std::cerr << "[WARNING] Failed to open/read image file : " << filePath << std::endl;
		SAFE_DELETE(imgData);
		dataSet = false;
		return DS_WARNING;
	}

	imgLength = imgData->TellHeight();
	imgWidth  = imgData->TellWidth();

	projPixelLength = (float)projLength / imgLength;
	projPixelWidth  = (float)projWidth  / imgWidth;

	dataSet = true;

	std::cerr << 
		"[DEBUG] Droplet Projector set up. Projecting image : " << filePath << 
		"\n        Image Dimensions          : " << imgLength << " x " << imgWidth << 
		"\n        Projection Dimensions     : " << projLength << " x " << projWidth <<
		"\n        Projected Pixel Dimension : " << 
		projPixelLength << " x " << projPixelWidth << std::endl;

	return DS_SUCCESS;
}

DS_RESULT DSimProjection::GetPixel(float *xyLoc, uint8_t *rgbaVal)
{
	if(!dataSet)
	{
		std::cerr << "[WARNING] No image file loaded. Use DSimProjection::LoadFile() first." 
			<< std::endl;
		return DS_WARNING;
	}

	int cLoc = static_cast<int>((xyLoc[0] + (floorWidth / 2)) / projPixelWidth);
	int rLoc = static_cast<int>((floorLength - (xyLoc[1] + (floorLength / 2))) / projPixelLength); 
	if (cLoc < (*imgData).TellWidth() && rLoc < (*imgData).TellHeight() && cLoc >= 0 && rLoc >= 0)
	{
		RGBApixel *pixelVal = (*imgData)(cLoc, rLoc);

		rgbaVal[0] = pixelVal->Red;
		rgbaVal[1] = pixelVal->Green;
		rgbaVal[2] = pixelVal->Blue;
		rgbaVal[3] = pixelVal->Alpha;
		return DS_SUCCESS;
	} else
	{
		rgbaVal[0] = 0;
		rgbaVal[1] = 0;
		rgbaVal[2] = 0;
		rgbaVal[3] = 0;
		return DS_WARNING;
	}
}

DS_RESULT DSimProjection::GetPixels(
	std::vector<float *> *xyLocs, 
	std::vector<uint8_t *> *rgbaVals)
{
	float halfFW = floorWidth / 2;
	float halfFL = floorLength / 2;

	std::vector<float *>::iterator xy_it;
	std::vector<uint8_t *>::iterator rgb_it;

	xy_it = xyLocs->begin();
	rgb_it = rgbaVals->begin();

	for(unsigned int i = 0; i < xyLocs->size(); i++){

		float *xyLoc =  *xy_it;
		int cLoc = static_cast<int>((xyLoc[0] + halfFW) / projPixelWidth);
		int rLoc = static_cast<int>((floorLength - (xyLoc[1] + (floorLength / 2))) / projPixelLength); 
		if (cLoc < (*imgData).TellWidth() && rLoc < (*imgData).TellHeight() && cLoc >= 0 && rLoc >= 0)
		{
			RGBApixel *pixVal = (*imgData)(cLoc, rLoc);
			uint8_t *rgbaVal = *rgb_it;
			rgbaVal[0] = pixVal->Red;
			rgbaVal[1] = pixVal->Green;
			rgbaVal[2] = pixVal->Blue;
			rgbaVal[3] = pixVal->Alpha;
		} else
		{
			uint8_t *rgbaVal = *rgb_it;
			rgbaVal[0] = 0;
			rgbaVal[1] = 0;
			rgbaVal[2] = 0;
			rgbaVal[3] = 0;
		}
		xy_it++;
		rgb_it++;
	}


	return DS_SUCCESS;
}

//DS_RESULT DSimProjection::GetPixels(float *xyTopLeft, float *xyBottomRight, uint8_t **rgbaVals)
//{
//	// TODO : Frank Erdesz's BMP Code goes here
//
//	return DS_SUCCESS;
//}