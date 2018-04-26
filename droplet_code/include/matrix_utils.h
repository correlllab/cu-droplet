#pragma once
#include "droplet_base.h"

typedef float Vector[3];
typedef float Matrix[3][3];

void vectorAdd(Vector* dst, Vector* a, Vector* b);		//dst = a + b
void vectorSubtract(Vector* dst, Vector* a, Vector* b);	//dst = a - b
void vectorSquare(Matrix* dst, Vector* a); // = a x b^{tr}

void matrixScale(Matrix* A, float s);
void matrixTimesVector(Vector* dst, Matrix* A, Vector* b);	//dst = A x b

void matrixAdd(Matrix* DST, Matrix* A, Matrix* B);			//DST = A + B
void matrixSubtract(Matrix* DST, Matrix* A, Matrix* B);		//DST = A - B
void matrixMultiply(Matrix* DST, Matrix* A, Matrix* B);		//DST = A x B
void matrixInplaceMultiply(Matrix* DST, Matrix* A, Matrix* B);

void matrixTranspose(Matrix* DST, Matrix* A);
void matrixInplaceTranspose(Matrix* A);

uint8_t positiveDefiniteQ(Matrix* A);
void choleskyDecomposition(Matrix* L, Matrix* A);


//void ldlDecomposition(Matrix* L, Matrix* D, Matrix* A);
void eigenvalues(Vector* eigVals, Matrix* A);
void eigensystem(Vector* eigVals, Matrix* eigVecs, Matrix* A);

void matrixInverse(Matrix* DST, Matrix* A);					//DST = A^(-1)
void matrixInplaceInverse(Matrix* A);
float matrixDet(Matrix* A);
float matrixTrace(Matrix* A);

void matrixCopy(Matrix* DST, Matrix* A);						//DST = A
void printMatrix(Matrix* A);
void printMatrixMathematica(Matrix* A);
