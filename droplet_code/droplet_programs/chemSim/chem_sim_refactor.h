/*
 * chem_sim.h
 *
 * Created: 5/10/2016 
 *  Author: Audrey Randall
 */ 


#ifndef CHEM_SIM_H_
#define CHEM_SIM_H_

#define MAX_BONDS 4
#define STATE_1 1
#define STATE_2 2
#define STATE_3 3
#define MAX_NEAR_ATOMS 12
#define SLOT_LENGTH_MS			419
#define SLOTS_PER_FRAME			29 //116
#define FRAME_LENGTH_MS			(SLOT_LENGTH_MS*SLOTS_PER_FRAME)
#define LOOP_PERIOD 17
#define BOND_TYPE_COV 2
#define BOND_TYPE_ION 1
#define NUM_ELEMENTS 13
#define MAX_ATOMS_IN_MC 15

#include "droplet_init.h"

typedef struct
{
	int8_t valence[8]; //cut down data to 3 bits each? -1 for no slot (H and He), 0 for empty, 1 for free electron, 2 for single bond, 3 for double bond, 4 for triple bond
	uint16_t bonded_atoms[MAX_BONDS];
	float chi; //this number represents Mulliken-Jaffe electronegativity on a Pauling scale: X = 3.48[(IEv + EAv)/2 - 0.602], where EAv = electron affinity and IEv = first ionization energy
	char name[2];
	uint8_t atomicNum;
}Atom;

typedef struct
{
	Atom atom;
	float bearing;
	float heading;
	uint16_t id;
	uint16_t range; 
}Near_Atom;

typedef struct  
{
	uint16_t ID;
	uint8_t atomicNum;
}MC_Component;

typedef struct
{
	uint16_t bonded_atoms[4];
	uint16_t blink_timer; //only if in molecule
	MC_Component molecule[15];
	uint8_t atomicNum;
	uint8_t valence[3];
	char msgFlag;
}State;

typedef struct  
{
	uint8_t atomicNum;
	uint16_t bonded_atoms[4];
	uint8_t molecule[23];
	uint8_t valence[3];
	uint16_t blink_timer; //can probably also be replaced with an ordinal if necessary
	char msgFlag;
}State_Msg;

/*My atomic number */
uint8_t MY_CHEM_ID;

/*Status of the electrons in my valence shell: electrons are paired to simulate orbitals. Numbers represent the status of each individual electron.
	-1: no orbital exists here. Should always be paired with another -1.
	0: no electron here, but orbital exists
	1: free electron
	2: electron in single bond
	3: electron in double bond
	4: electron in triple bond
*/
uint8_t valence[8];

/*Information about nearby robots*/
Near_Atom NULL_NEAR_ATOM = {{{0,0,0,0,0,0,0,0},{0,0,0,0},0,{' ',' '},0}, 0, 0, 0};
Atom NULL_ATOM = {{0,0,0,0,0,0,0,0},{0,0,0,0},0,{'0','0'},0};
volatile uint16_t globalBlinkTimer;
Near_Atom near_atoms[MAX_NEAR_ATOMS];
	
/*My atom struct */
Atom myID;

/* My molecule */
MC_Component my_molecule[MAX_ATOMS_IN_MC]; 

/*Info about the atom I'm moving toward, if I'm in a molecule */
uint16_t target_id;

uint32_t	frameCount;
uint32_t	frameStart;
uint16_t	lastLoop;
uint16_t	mySlot;

uint8_t addToNearAtoms(Near_Atom* near_atom);
uint8_t attemptToBond(Atom* other, int bondType, uint16_t other_ID);
uint8_t breakBond(Atom* other, uint16_t senderID, uint8_t bondType);
uint8_t chiCheck(Atom* other);
void createStateMessage(State_Msg* msg, char flag);
uint8_t energyCheck(MC_Component sender_mc[MAX_ATOMS_IN_MC], uint8_t otherHalfBond);
void getAtomColor(Atom* ID, uint8_t* r, uint8_t* g, uint8_t* b);
Atom* getAtomFromAtomicNum(uint8_t atomicNum);
//getAtomFromID? All the rest of the get x from ID functions boil down to this
float getChiFromAtomicNum(uint8_t atomicNum);
void getNameFromAtomicNum(char* name, uint8_t atomicNum);
uint8_t isInMyMolecule(uint16_t ID);
void initAtomState();
void initBondedAtoms(Atom atom);
uint8_t moleculesOverlap(MC_Component sender_mc[MAX_ATOMS_IN_MC]);
void msgState(ir_msg* msg_struct);
uint8_t otherBondedToSelf(Atom* other);
void packValences(uint8_t* packed_shells, int8_t* shells);
void packMolecule(uint8_t packed_mc[21], MC_Component mc[MAX_ATOMS_IN_MC]);
void printMolecularReaction(uint8_t* reactant1, uint8_t* reactant2, uint8_t* product1, uint8_t* product2, uint8_t lenR1, uint8_t lenR2, uint8_t lenP1, uint8_t lenP2);
void printMyValence();
void printMyBondedAtoms();
uint8_t selfBondedToOther(uint16_t other_ID);
void setAtomColor(Atom* ID);
void unpackMolecule(uint8_t packed_mc[21], MC_Component mc[MAX_ATOMS_IN_MC]);
void unpackValences(uint8_t* packed_shells, int8_t* shells);
uint8_t updateNearAtoms(Atom* near_atom, ir_msg* msg_struct);

static inline uint8_t molecule_length(MC_Component mc[MAX_ATOMS_IN_MC]) {
	uint8_t i;
	for(i = 0; i < MAX_ATOMS_IN_MC; i++) {
		if(mc[i].ID==0) break;
	}
	return i;
}

void init();
void loop();
void handle_msg(ir_msg* msg_struct);

#endif /* CHEM_SIM_H_ */