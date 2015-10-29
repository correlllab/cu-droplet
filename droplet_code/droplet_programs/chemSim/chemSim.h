#ifndef CHEM_SIM
#define CHEM_SIM

#include "droplet_init.h"

#define RNB_BROADCAST_PERIOD 15000
#define BLINK_PERIOD 2350
#define CHEM_ID_BROADCAST_PERIOD 3900
#define DETECT_OTHER_DROPLETS_PERIOD 1000
#define UPDATE_ATOMS_PERIOD 100
#define LOOP_PERIOD 50
#define MOLECULE_BROADCAST_PERIOD 4150
#define NUM_FIXED_DROPLETS 8

typedef struct  
{
	uint16_t bonded_atoms[4];
	uint16_t blink_timer; //only if in molecule	
	uint8_t molecule_nums[15];	
	uint8_t atomicNum;	
	uint8_t valence[3];	
	uint8_t bondType;
	char msgFlag;	
}State_Msg;

typedef struct
{
	int8_t valence[8]; //cut down data to 3 bits each? -1 for no slot (H and He), 0 for empty, 1 for free electron, 2 for single bond, 3 for double bond, 4 for triple bond
	uint16_t bonded_atoms[6];
	float chi; //this number represents Mulliken-Jaffe electronegativity on a Pauling scale: X = 3.48[(IEv + EAv)/2 - 0.602], where EAv = electron affinity and IEv = first ionization energy
	char name[2]; 
	uint8_t bondType; //0 = no bonds, 1 = ionic bonds, 2 = covalent bonds. First two bits are bond type. Six most significant bits are stability.
	uint8_t diatomic;	
	uint8_t atomicNum;
}Atom;

typedef struct
{
	Atom atom;
	uint16_t id;
	uint16_t range; //in theory you should reorder this at some point.
	float bearing;
	float heading;
	uint8_t last_msg_t; //time that this atom last sent a message to me
	uint8_t bonded;
	uint8_t stability;
}Near_Atom;

typedef struct 
{
	uint16_t bonded_atoms[6];
	uint16_t blink_timer;
}Bonded_Atoms_Msg;

typedef struct  
{
	char flag_array[9];
	int8_t orbitals[6]; //-1 for empty orbital, 0 for orbital that the recipient doesn't occupy, 1 for an orbital it does occupy.
}Bond_Made_Msg;

typedef struct  
{
	uint16_t ID;
	uint8_t stability;
}Stability_Tool;

typedef struct{
	int16_t x;
	int16_t y;
}FixedRNBPos;

FixedRNBPos fixedRNBPositions[NUM_FIXED_DROPLETS] = {{0,0},{-80,90},{-80,230},{260,260},{260,160},{260,60},{200,0},{100,0},{100,150}};

typedef struct
{
	float range;
	float bearing;
	float heading;
	uint32_t time;
}FixedRNBMeas;

FixedRNBMeas fixedRNBMeasurements[NUM_FIXED_DROPLETS];

Near_Atom near_atoms[12]; //this number is pretty arbitrary.
Atom NULL_ATOM = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0},{'0','0'},0,0,0};
Near_Atom NULL_NEAR_ATOM = {{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0},{'0','0'},0,0,0}, 0, 0, 0, 0, 0, 0};
volatile uint32_t bondDelay;
volatile uint16_t potentialPartner;
volatile uint32_t tap_delay;
volatile uint32_t bonded_atoms_delay;
volatile uint32_t sent_atom_delay;
volatile uint32_t last_rnb;
uint32_t last_chem_ID_broadcast;
volatile uint16_t global_blink_timer;
uint16_t my_molecule[15];  //this differs from bonded_atoms in that it includes all atoms in the molecule, not just ones directly bonded to self. 
Stability_Tool my_molecule_stability[15];
uint16_t used_in_stability[15];
uint8_t my_molecule_length;
uint8_t collided;
float target_spot;
uint16_t target_id;
uint8_t stability;
uint32_t timeLastMoved;
int16_t deltaGself;		//my molecule's deltaG
int16_t deltaGself_p;	//my molecule's deltaG if the atom I'm talking to were to bond with me
int16_t deltaGself_m;	//my molecule's deltaG without me
Atom myID;
uint32_t lastPositionUpdateCall;

uint8_t turning_on;
uint16_t main_cca;

void init();
void loop();
void handle_msg(ir_msg* msg_struct);
void user_leg_status_interrupt();

uint8_t addAtomToMolecule(uint16_t atom_id);
int16_t addAtomToStability(uint16_t ID, int16_t cur_s, uint8_t cur_s_size);
void addToBondedAtoms(uint16_t ID, uint8_t index, uint8_t num_bonds);
//void add_to_my_orbitals(uint16_t ID, uint8_t num_bonds);
uint8_t addToNearAtoms();
//void broadcastChemID(Atom ID);
void breakBond(Atom* near_atom, uint16_t sender_ID);
uint8_t calculateMyStability();
void calculatePath(float target, uint16_t range, float bearing);
void calculateTarget(Atom* nearAtom, uint16_t range, float bearing, float heading);
uint8_t cleanOtherMolecule(Atom* near_atom, uint8_t* dirtyMolecule, uint8_t* cleanMolecule, uint8_t count);
int comparison(uint8_t* aPtr, uint8_t* bPtr);
uint8_t convertBearingToEmitorDir(float bearing);
void convertEmitorDirToArray(uint8_t dirs, uint8_t* bits);
void createStateMessage(State_Msg* msg, char flag);
void detectOtherDroplets();
void formBond(uint16_t senderID, Atom* near_atom, char flag);
void foundBondRoutine(char flag);
uint8_t getAtomicNumFromID(uint16_t ID);
void getAtomColor(Atom* ID, uint8_t* r, uint8_t* g, uint8_t* b);
Atom* getAtomFromAtomicNum(uint8_t atomicNum);
Atom* getAtomFromID(uint16_t ID);
float getChiFromID(uint16_t ID);
uint8_t getFilledOrbs();
//uint8_t getFixedIndex(uint16_t id);
//void getOrbitals(Atom* atom);
void initAtomState();
void initRandomMove(uint16_t direc);
uint8_t isGoodRNB(float rng, float bearing, uint16_t ID);
void makePossibleBonds(Atom* near_atom_ptr, char flag, int16_t deltaGother, int16_t deltaGother_p, int16_t deltaGother_m, uint16_t senderID);
void matchMolecule(uint16_t* other_molecule, uint8_t length, uint16_t exclude_id);
void modifyValencesIonic(char* newValence, Atom* near_atom_ptr, uint16_t senderID);
void modifyValencesCovalent(char* newValence, Atom* near_atom_ptr, uint16_t senderID);
void moveToTarget(uint16_t rng, float bearing);
void msgBondedAtoms(Atom* near_atom, uint16_t new_blink, uint16_t sender_ID);
void msgBondMade(ir_msg* msg_struct, char flag);
//void msgContactFirst(uint16_t senderID);
//void msgContactSecond(char* msg, uint16_t senderID);
//void msgOrbital(uint16_t* other_bonded_atoms, uint16_t senderID);
void msgPossibleBond(ir_msg* msg_struct);
void msgState(ir_msg* msg_struct);
char IMRTest(Atom* near_atom, int16_t deltaGother, int16_t deltaGother_p, int16_t deltaGother_m, uint16_t senderID);
uint8_t* orbitalOrder(uint8_t *valence);
void packValences(uint8_t* packed_shells, int8_t* shells);
void printBondedAtoms();
void printValence(int8_t valence[]);
void removeAtomFromMolecule(uint16_t atom_id);
void repairBondedAtoms();
void repairValence();
void setAtomColor(Atom* ID);
void transmitMoleculeStruct(uint16_t exclude_id, char flag);
void unpackValences(uint8_t* packed_shells, int8_t* shells);
void updateDeltaGs();
void updateMolecule(uint16_t* atNums, uint8_t length, uint16_t sender);
void updateMoleculeTwo(Atom* near_atom, uint16_t senderID);
uint8_t updateNearAtoms(Atom* near_atom, ir_msg* msg_struct);
//void updatePositionEstimate();
void updateStability();
uint8_t valenceState();


/*
 int8_t bear = (int8_t)ceilf((3.0*last_good_rnb.bearing)/M_PI);
 uint8_t newDir = ((6-bear)%6);

*/

#endif
