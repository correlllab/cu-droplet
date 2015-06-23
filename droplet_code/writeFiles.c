/*The purpose of this code is to write the files that will each be uploaded to a droplet. Hopefully 
 it will create a folder with a file for each Droplet.
*/

#include "droplet_programs/test_code.h"
#include "Atom.h"
#include "stdbool.h"
#include <stdio.h>
#include "vector.h"

//This is a list of Atoms. Should they be static or constant or something?
//bond type, diatomic, valence shell, name, atomic number
//Bond type 0 = ionic
//Diatomic 0 = false
Atom H = {1, 1, {1, 0, -1, -1, -1, -1, -1, -1}, "H", 1, {1, 1, 1, 0, -1,-1, -1, -1, -1, -1, 'H', ' ', 1}};
Atom He = {1, 0, {1, 1, -1, -1, -1, -1, -1, -1}, "He", 2, {1, 0, 1, 1, -1, -1, -1, -1, -1, -1, 'H', 'e', 2}};
Atom Li = {0, 0, {1, 0, 0, 0, 0, 0, 0, 0}, "Li", 3, {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'L', 'i', 3,}};
Atom Be = {0, 0, {1, 1, 0, 0, 0, 0, 0, 0}, "Be", 4, {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 'B', 'e', 4,}};
	
//arSize is the size of both arrays (they're the same length). atomArray is to be returned. numArray is the array of atomic numbers that must be converted to an array of Atom structs.
vector initAtoms(uint8_t vectorSize, vector atomicNums)
{
	vector atomVector;
	vector_init(&atomVector);
	for(uint8_t i = 0; i < vectorSize; i++)
	{
		uint8_t atomicNum = *(int*)(vector_get(&atomicNums, i));
		switch(atomicNum){
			case 1:
			{
				//Atom* H = {1, 1, {1, 0, -1, -1, -1, -1, -1, -1}, "H", 1};
				printf("case 1");
				vector_add(&atomVector, &H);
				break;
			}
			case 2:
			{
				//Atom* He = {1, 0, {1, 1, -1, -1, -1, -1, -1, -1}, {'H', 'e'}, 2};
				printf("case 2");
				vector_add(&atomVector, &He);
				break;
			}
			case 3:
			{
				//Atom* Li = {0, 0, {1, 0, 0, 0, 0, 0, 0, 0}, {'L','i'}, 3};
				printf("case 3");
				vector_add(&atomVector, &Li);
				break;
			}
			case 4:
			{
				//Atom* Be = {0, 0, {1, 1, 0, 0, 0, 0, 0, 0}, {'B','e'}, 4};
				printf("case 4");
				vector_add(&atomVector, &Be);
				break;
			}
			default:
			printf("No such element");
		}
		

		//printf("Not enough elements exist or I didn't initialize enough. Here's what we've got. \n");
		
	}
	return atomVector;
}

//Sets the global uint8_t MY_CHEM_ID in each file that will be uploaded to a droplet
Atom setChemID(string filename, vector allChemIDs)
{
	/*
	for(uint8_t i = 0; i < allChemIDs.size(); i++)
	{
		write("uint8_t MY_CHEM_ID = ", allChemIDs[i][12], ";") to filename on line 7 (or first free line?)
	}
	*/
}

void main()
{
	set_rgb(10, 10, 255); //this is a test line
	//Eventually, this vector should be created by user input. It has the atomic numbers of all the atoms we want to make.
	vector atomicNums;
	uint8_t one = 1;
	uint8_t two = 2;
	uint8_t three = 3;
	uint8_t four = 4;
	vector_init(&atomicNums);
	vector_add(&atomicNums, &one);
	vector_add(&atomicNums, &two);
	vector_add(&atomicNums, &three);
	vector_add(&atomicNums, &four);
	uint8_t vectorSize = vector_count(&atomicNums);

	//Convert vector of atomic numbers to vector of Atom structs
	vector atomVector;
	atomVector = initAtoms(vectorSize, atomicNums);
	//Test whether atomVector is correctly initialized
	for(uint8_t i = 0; i < vectorSize; i++)
	{
		//vector_get returns *void. To typecast to *Atom, use (Atom*)vector_get(stuff)
		printf("atomVector[i].name = %s \n", ((Atom*)vector_get(&atomVector, i))->name);
	}
	
	/*
	for(uint8_t i = 0; i < atomVector.size(); i++)
	{
		copy main.c to a file named d(i).c, ex d1.c, d2.c, etc
		setChemID(di.c, atomVector);
	}
	*/
	
}