#ifndef HEADER_FILE
#define HEADER_FILE

#include <avr/io.h>

typedef uint8_t bool;

typedef struct
{
	int8_t valence[8]; //cut down data to 3 bits each? -1 for no slot (H and He), 0 for empty, 1 for free electron, 2 for ionically bonded, 3 for covalently bonded
	uint16_t bonded_atoms[6];
	float chi; //this number represents Mulliken-Jaffe electronegativity on a Pauling scale: X = 3.48[(IEv + EAv)/2 - 0.602], where EAv = electron affinity and IEv = first ionization energy
	char name[2]; 
	uint8_t bondType; //0 = no bonds, 1 = ionic bonds, 2 = covalent bonds. Necessary because an alkali can't (usually doesn't?) bond to a covalent molecule.
	bool diatomic;	
	uint8_t atomicNum;
}Atom;

#endif
