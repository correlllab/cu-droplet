#include "matrix_utils.h"

void vectorAdd(Vector* dst, Vector* a, Vector* b){		//dst = a + b
	(*dst)[0] = (*a)[0] + (*b)[0];
	(*dst)[1] = (*a)[1] + (*b)[1];
	(*dst)[2] = (*a)[2] + (*b)[2];
}

void vectorSubtract(Vector* dst, Vector* a, Vector* b){	//dst = a - b
	(*dst)[0] = (*a)[0] - (*b)[0];
	(*dst)[1] = (*a)[1] - (*b)[1];
	(*dst)[2] = (*a)[2] - (*b)[2];
}

void vectorSquare(Matrix* DST, Vector* a){
	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			(*DST)[i][j] = (*a)[i] * (*a)[j];
		}
	}
}

void matrixScale(Matrix* A, float s){
	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			(*A)[i][j] = (*A)[i][j]*s;
		}
	}
}

void matrixTimesVector(Vector* dst, Matrix* A, Vector* b){	//dst = A x b
	for(uint8_t i=0;i<3;i++){
		(*dst)[i] = 0;
		for(uint8_t j=0;j<3;j++){
			(*dst)[i] += (*A)[i][j] * (*b)[j];
		}
	}
}

void matrixAdd(Matrix* DST, Matrix* A, Matrix* B){			//DST = A + B
	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			(*DST)[i][j] = (*A)[i][j] + (*B)[i][j];
		}
	}
}

void matrixSubtract(Matrix* DST, Matrix* A, Matrix* B){		//DST = A - B
	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			(*DST)[i][j] = (*A)[i][j] - (*B)[i][j];
		}
	}
}

void matrixMultiply(Matrix* DST, Matrix* A, Matrix* B){		//DST = A x B
	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			(*DST)[i][j] = 0;
			for(uint8_t k=0;k<3;k++){
				(*DST)[i][j] += (*A)[i][k] * (*B)[k][j];
			}
		}
	}
}

void matrixInplaceMultiply(Matrix* DST, Matrix* A, Matrix* B){		//DST = A x B
	Matrix tmp;
	Matrix* tmpA;
	Matrix* tmpB;
	if(A==DST){
		matrixCopy(&tmp, A);
		tmpA = &tmp;
		tmpB = B;
	}else if(B==DST){
		matrixCopy(&tmp, B);
		tmpB = &tmp;
		tmpA = A;
	}else{
		tmpA = A;
		tmpB = B;
	}

	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			(*DST)[i][j] = 0;
			for(uint8_t k=0;k<3;k++){
				(*DST)[i][j] += (*tmpA)[i][k] * (*tmpB)[k][j];
			}
		}
	}
}

void matrixTranspose(Matrix* DST, Matrix* A){
	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			(*DST)[j][i] = (*A)[i][j];
		}
	}
}

void matrixInplaceTranspose(Matrix* A){
	float tmp;
	for(uint8_t i=0;i<2;i++){
		for(uint8_t j=i+1;j<3;j++){
			tmp = (*A)[i][j];
			(*A)[i][j] = (*A)[j][i];
			(*A)[j][i] = tmp;
		}
	}
}

uint8_t positiveDefiniteQ(Matrix* A){
	Matrix L;
	uint8_t flag=0;
	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			L[i][j] = 0;
		}
	}
	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<(i+1);j++){
			float s = 0;
			for(uint8_t k=0; k<j; k++){
				s += L[i][k] * L[j][k];
			}
			if(i==j){
				float tmp = (*A)[i][j]-s;
				if(tmp<=0 || isnan(tmp)) flag=1;
				L[i][j] = sqrtf(tmp);
			}else{
				L[i][j] = (1.0/ L[j][j] * ( (*A)[i][j] - s ));
			}
		}
	}
	if(flag){
		printf("Non-Positive Definite Matrix Detected!\r\n");
		printMatrixMathematica(A);
		printf("Cholesky Decomposition:\r\n");
		printMatrixMathematica(&L);
		return 0;
	}
	return 1;
}

//returns upper triangular.
void choleskyDecomposition(Matrix* L, Matrix* A){
	(*L)[0][0] = sqrtf((*A)[0][0]);
	(*L)[0][1] = (*A)[0][1] / (*L)[0][0];
	(*L)[0][2] = (*A)[0][2] / (*L)[0][0];
	(*L)[1][0] = 0;
	(*L)[1][1] = sqrtf( (*A)[1][1] - powf((*L)[0][1],2));
	(*L)[1][2] = ( (*A)[1][2] - (*L)[0][2] * (*L)[0][1] ) / (*L)[1][1];
	(*L)[2][0] = 0;
	(*L)[2][1] = 0;
	(*L)[2][2] = sqrtf( (*A)[2][2] - powf((*L)[0][2],2) - powf((*L)[1][2],2) );
}

//returns upper triangular.
void ldlDecomposition(Matrix* L, Matrix* D, Matrix* A){
	(*D)[0][0] = (*A)[0][0];
	(*L)[0][0] = 1;
	(*D)[0][1] = 0;
	(*L)[0][1] = (*A)[0][1]/(*D)[0][0];
	(*D)[0][2] = 0;
	(*L)[0][2] = (*A)[0][2]/(*D)[0][0];
	(*D)[1][0] = 0;
	(*L)[1][0] = 0;
	(*D)[1][1] = (*A)[1][1] - ( (*L)[0][1] * (*L)[0][1] * (*D)[0][0] );
	(*L)[1][1] = 1;
	(*D)[1][2] = 0;
	(*L)[1][2] = ( (*A)[1][2] - ( (*L)[0][1] * (*L)[0][2] * (*D)[0][0] ) ) / (*D)[1][1];
	(*D)[2][0] = 0;
	(*L)[2][0] = 0;
	(*D)[2][0] = 0;
	(*L)[2][1] = 0;
	(*D)[2][2] = (*A)[2][2] - ( (*L)[0][2] * (*L)[0][2] * (*D)[0][0] ) - ( (*L)[1][2] * (*L)[1][2] * (*D)[1][1] );
	(*L)[2][2] = 1; 
}

void matrixInverse(Matrix* DST, Matrix* A){					//DST = A^(-1)
	(*DST)[0][0] = (*A)[1][1]*(*A)[2][2] - (*A)[1][2]*(*A)[2][1];
	(*DST)[0][1] = (*A)[0][2]*(*A)[2][1] - (*A)[0][1]*(*A)[2][2];
	(*DST)[0][2] = (*A)[0][1]*(*A)[1][2] - (*A)[0][2]*(*A)[1][1];
	(*DST)[1][0] = (*A)[1][2]*(*A)[2][0] - (*A)[1][0]*(*A)[2][2];
	(*DST)[1][1] = (*A)[0][0]*(*A)[2][2] - (*A)[0][2]*(*A)[2][0];
	(*DST)[1][2] = (*A)[0][2]*(*A)[1][0] - (*A)[0][0]*(*A)[1][2];
	(*DST)[2][0] = (*A)[1][0]*(*A)[2][1] - (*A)[1][1]*(*A)[2][0];
	(*DST)[2][1] = (*A)[0][1]*(*A)[2][0] - (*A)[0][0]*(*A)[2][1];
	(*DST)[2][2] = (*A)[0][0]*(*A)[1][1] - (*A)[0][1]*(*A)[1][0];

	float determinant = (*A)[0][0]*(*DST)[0][0] + (*A)[0][1]*(*DST)[1][0] + (*A)[0][2]*(*DST)[2][0];

	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			(*DST)[i][j] = (*DST)[i][j]/determinant;
		}
	}
}

void matrixInplaceInverse(Matrix* A){					//DST = A^(-1)
	Matrix tmp;
	matrixCopy(&tmp, A);

	(*A)[0][0] = tmp[1][1]*tmp[2][2] - tmp[1][2]*tmp[2][1];
	(*A)[0][1] = tmp[0][2]*tmp[2][1] - tmp[0][1]*tmp[2][2];
	(*A)[0][2] = tmp[0][1]*tmp[1][2] - tmp[0][2]*tmp[1][1];
	(*A)[1][0] = tmp[1][2]*tmp[2][0] - tmp[1][0]*tmp[2][2];
	(*A)[1][1] = tmp[0][0]*tmp[2][2] - tmp[0][2]*tmp[2][0];
	(*A)[1][2] = tmp[0][2]*tmp[1][0] - tmp[0][0]*tmp[1][2];
	(*A)[2][0] = tmp[1][0]*tmp[2][1] - tmp[1][1]*tmp[2][0];
	(*A)[2][1] = tmp[0][1]*tmp[2][0] - tmp[0][0]*tmp[2][1];
	(*A)[2][2] = tmp[0][0]*tmp[1][1] - tmp[0][1]*tmp[1][0];

	float determinant = tmp[0][0]*(*A)[0][0] + tmp[0][1]*(*A)[1][0] + tmp[0][2]*(*A)[2][0];

	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			(*A)[i][j] = (*A)[i][j]/determinant;
		}
	}
}


float matrixDet(Matrix* A){
	return ( (*A)[0][0]*((*A)[1][1]*(*A)[2][2] - (*A)[1][2]*(*A)[2][1]) +
			 (*A)[0][1]*((*A)[1][2]*(*A)[2][0] - (*A)[1][0]*(*A)[2][2]) +
			 (*A)[0][2]*((*A)[1][0]*(*A)[2][1] - (*A)[1][1]*(*A)[2][0]));
}


float matrixTrace(Matrix* A){
	return ( (*A)[0][0] + (*A)[1][1] + (*A)[2][2] );
}

void matrixCopy(Matrix* DST, Matrix* A){					//DST = A
	for(uint8_t i=0;i<3;i++){
		for(uint8_t j=0;j<3;j++){
			(*DST)[i][j] = (*A)[i][j];
		}
	}
}

const char matrix3str[] PROGMEM = "| %7.1f %7.1f %7.1f |\r\n";
void printMatrix(Matrix* A){
	printf_P(matrix3str, (*A)[0][0], (*A)[0][1], (*A)[0][2]);
	printf_P(matrix3str, (*A)[1][0], (*A)[1][1], (*A)[1][2]);
	printf_P(matrix3str, (*A)[2][0], (*A)[2][1], (*A)[2][2]);
}

const char matrix3mathStartStr[] PROGMEM = "{\r\n  {%15.10f, %15.10f, %15.10f},\r\n";
const char matrix3mathStr[] PROGMEM = "  {%15.10f, %15.10f, %15.10f},\r\n";
const char matrix3mathEndStr[] PROGMEM = "  {%15.10f, %15.10f, %15.10f}\r\n};\r\n";
void printMatrixMathematica(Matrix* A){
	printf_P(matrix3mathStartStr, (*A)[0][0], (*A)[0][1], (*A)[0][2]);
	printf_P(matrix3mathStr,	  (*A)[1][0], (*A)[1][1], (*A)[1][2]);
	printf_P(matrix3mathEndStr,   (*A)[2][0], (*A)[2][1], (*A)[2][2]);
}