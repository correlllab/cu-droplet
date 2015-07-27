#ifndef CHEM_SIM
#define CHEM_SIM

#include "droplet_init.h"
#include "Atom.h"

#define RNB_BROADCAST_PERIOD 11000
#define BLINK_PERIOD 2500
#define MIN_INTER_CHEM_ID_BROADCAST_DELAY 3000
#define DETECT_OTHER_DROPLETS_PERIOD 1000
#define UPDATE_ATOMS_PERIOD 100
#define LOOP_PERIOD 50

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

Near_Atom near_atoms[12]; //this number is pretty arbitrary.
Atom NULL_ATOM = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0},{'0','0'},0,0,0};
Near_Atom NULL_NEAR_ATOM = {{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0},{'0','0'},0,0,0}, 0, 0, 0, 0, 0};
volatile uint32_t bondDelay;
volatile uint16_t potentialPartner;
volatile uint32_t tap_delay;
volatile uint32_t bonded_atoms_delay;
uint32_t last_chem_ID_broadcast;
uint16_t global_blink_timer;

void init();
void loop();
void handle_msg(ir_msg* msg_struct);
void user_leg_status_interrupt();

void add_to_bonded_atoms(uint16_t ID);
void add_to_near_atoms();
void broadcastChemID(Atom ID);
void checkPossibleBonds(Atom* near_atom, uint16_t senderID);
void detectOtherDroplets();
void formCovalentBond(uint16_t senderID, Atom near_atom);
void formDiatomicBond(uint16_t senderID, Atom near_atom, uint8_t my_empty, uint8_t other_empty);
void formIonicBond(uint16_t senderID, Atom near_atom);
void found_bond_routine(char flag);
Atom getAtomFromAtomicNum(uint8_t atomicNum);
Atom getAtomFromID(uint16_t ID);
float getChiFromID(uint16_t ID);
void makePossibleBonds(Atom near_atom, char flag, uint16_t senderID);
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
uint8_t valenceState();


Atom myID;

#endif
