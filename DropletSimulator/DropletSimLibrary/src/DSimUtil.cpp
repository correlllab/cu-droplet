#include "DSimUtil.h"

TrigArray *dropletRelPos;

TrigArray::TrigArray(unsigned int numDroplets)
{
	n = numDroplets;
	l = n * (n - 1) / 2;
	data = (RelPosData **)malloc(sizeof(RelPosData *) * l);

	for(unsigned int i = 0; i < l; i++)
	{
		data[i] = (RelPosData *)malloc(sizeof(RelPosData));
		data[i]->xDist = 0.f;
		data[i]->yDist = 0.f;
		data[i]->zDist = 0.f;
	}
}

TrigArray::~TrigArray(void)
{
	for(int i = l - 1; i >= 0; i--)
	{	free(data[i]); data[i] = NULL; }
}

DS_RESULT TrigArray::AddData(unsigned int d1, unsigned int d2, float xDist, float yDist, float zDist)
{
	if(d1 >= n || d2 >= n) return DS_ERROR;

	unsigned int i = computeIndex(d1, d2);
	if(i == -1) return DS_ERROR;

	if(d1 > d2)
	{
		xDist *= -1;
		yDist *= -1;
		zDist *= -1;
	}

	data[i]->xDist = xDist;
	data[i]->yDist = yDist;
	data[i]->zDist = zDist;

	return DS_SUCCESS;
}

DS_RESULT TrigArray::RemoveData(unsigned int d1)
{
	if(d1 >= n) return DS_ERROR;

	// TODO : Implement TrigArray::RemoveData(unsigned int d1). This one's a doozy!

	return DS_SUCCESS;
}

DS_RESULT TrigArray::GetData(unsigned int d1, unsigned int d2, float *dist, float *angle)
{
	if(d1 >= n || d2 >= n) return DS_ERROR;

	*dist = this->GetDistance(d1, d2);
	*angle = this->GetAngle(d1, d2);

	return DS_SUCCESS;
}

float TrigArray::GetDistance(unsigned int d1, unsigned int d2)
{
	if(d1 >= n || d2 >= n) return 0.f;

	unsigned int i = computeIndex(d1, d2);
	if(i == -1) return 0.f;

	float dist=sqrtf(
		powf(data[i]->xDist, 2) +
		powf(data[i]->yDist, 2) +
		powf(data[i]->zDist, 2));

	return dist;
}

float TrigArray::GetAngle(unsigned int d1, unsigned int d2)
{
	if(d1 >= n || d2 >= n) return 0.f;

	unsigned int i = computeIndex(d1, d2);
	if(i == -1) return 0.f;

	float angle = 0.f;

	// Subtracting pi/2 from atan2f because bullet thinks due north is 0 degree.  ^_^;
	if(d1 > d2)
		angle = atan2f(-1 * data[i]->yDist, -1 * data[i]->xDist) - SIMD_HALF_PI;
	else
		angle = atan2f(data[i]->yDist, data[i]->xDist) - SIMD_HALF_PI;

	return angle;
}

unsigned int TrigArray::computeIndex(unsigned int d1, unsigned int d2)
{
	if(d1 < d2) return ((d1 * n) - (d1 * (d1 + 1) / 2)) + (d2 - d1) - 1;
	if(d1 > d2) return ((d2 * n) - (d2 * (d2 + 1) / 2)) + (d1 - d2) - 1;
	else return -1;	// Should never happen
}
