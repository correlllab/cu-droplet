#ifndef CHEM_SIM
#define CHEM_SIM

#include "droplet_init.h"

#define RNB_BROADCAST_PERIOD 11000
#define BLINK_PERIOD 2500
#define MIN_INTER_CHEM_ID_BROADCAST_DELAY 3000
#define DETECT_OTHER_DROPLETS_PERIOD 1000
#define UPDATE_ATOMS_PERIOD 100
#define LOOP_PERIOD 50

typedef struct
{
	int8_t valence[8]; //cut down data to 3 bits each? -1 for no slot (H and He), 0 for empty, 1 for free electron, 2 for single bond, 3 for double bond, 4 for triple bond
	uint16_t bonded_atoms[6];
	float chi; //this number represents Mulliken-Jaffe electronegativity on a Pauling scale: X = 3.48[(IEv + EAv)/2 - 0.602], where EAv = electron affinity and IEv = first ionization energy
	char name[2]; 
	uint8_t bondType; //0 = no bonds, 1 = ionic bonds, 2 = covalent bonds. Necessary because an alkali can't (usually doesn't?) bond to a covalent molecule.
	uint8_t diatomic;	
	uint8_t atomicNum;
}Atom;

typedef struct
{
	Atom atom;
	uint16_t id;
	uint8_t range; //in theory you should reorder this at some point.
	int16_t bearing;
	int16_t heading;
	uint8_t last_msg_t; //time that this atom last sent a message to me
	uint8_t bonded;
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
	int8_t type; 
	/*
	Type refers to the hybridization. The code is as follows:
		-1 = nonexistent orbital
		0 = s
		1 = p
		2 = sp
		3 = sp2
		4 = sp3
		5 = dsp3
		6 = d2sp3
		7 = d
		8 = f
	*/
}Orbital;

Near_Atom near_atoms[12]; //this number is pretty arbitrary.
Atom NULL_ATOM = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0},{'0','0'},0,0,0};
Near_Atom NULL_NEAR_ATOM = {{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0},{'0','0'},0,0,0}, 0, 0, 0, 0, 0};
volatile uint32_t bondDelay;
volatile uint16_t potentialPartner;
volatile uint32_t tap_delay;
volatile uint32_t bonded_atoms_delay;
uint32_t last_chem_ID_broadcast;
uint16_t global_blink_timer;
uint8_t my_molecule[32];  //this differs from bonded_atoms in that it includes all atoms in the molecule, not just ones directly bonded to self. Also, it's atomic nums not IDs
Orbital my_orbitals[6];
Atom myID;

void init();
void loop();
void handle_msg(ir_msg* msg_struct);
void user_leg_status_interrupt();

void add_to_bonded_atoms(uint16_t ID);
void add_to_my_orbitals(uint16_t ID, uint8_t num_bonds);
void add_to_near_atoms();
void broadcastChemID(Atom ID);
void calculate_path(char* orbitals, uint16_t ID);
void detectOtherDroplets();
void formBond(uint16_t senderID, Atom near_atom, char flag);
void found_bond_routine(char flag);
uint8_t getAtomicNumFromID(uint16_t ID);
Atom getAtomFromAtomicNum(uint8_t atomicNum);
Atom getAtomFromID(uint16_t ID);
float getChiFromID(uint16_t ID);
void getOrbitals(Atom atom);
void makePossibleBonds(Atom near_atom, char flag, uint16_t senderID);
void modify_valences_ionic(char* newValence, Atom near_atom, uint16_t senderID);
void modify_valences_covalent(char* newValence, Atom near_atom, uint16_t senderID);
void msgAtom(ir_msg* msg_struct);
void msgBondedAtoms(ir_msg* msg_struct);
void msgBondMade(ir_msg* msg_struct, char flag);
void msgPossibleBond(ir_msg* msg_struct);
uint8_t* orbital_order(uint8_t *valence);
void print_near_atoms();
void printValence(int8_t valence[]);
void repairBondedAtoms();
void repairValence();
void setAtomColor(Atom ID);
void update_near_atoms();
void update_molecule(char* atNums, uint8_t length, uint16_t sender);
uint8_t valenceState();


#endif
